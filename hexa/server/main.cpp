//---------------------------------------------------------------------------
// server/main.cpp
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include <cassert>
#include <iostream>
#include <fstream>
#include <ctime>
#include <signal.h>

#ifndef _WIN32
# include <pthread.h>
#endif

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/config.hpp>
#include <hexa/os.hpp>
#include <hexa/protocol.hpp>
#include <hexa/drop_privileges.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/persistence_sqlite.hpp>
#include <hexa/memory_cache.hpp>
#include <hexa/trace.hpp>
#include <hexa/entity_system_physics.hpp>
#include <hexa/win32_minidump.hpp>

#include "clock.hpp"
#include "lua.hpp"
#include "udp_server.hpp"
#include "network.hpp"
#include "server_entity_system.hpp"
#include "init_terrain_generators.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using boost::bind;
using boost::format;
using namespace hexa;

po::variables_map global_settings;

static std::string default_db_path()
{
    return (app_user_dir() / fs::path(SERVER_DB_PATH)).string();
}

bool lolquit = false;
void physics (server_entity_system& s, storage_i& terrain)
{
    using namespace boost::chrono;

    milliseconds tick (50);
    auto last_tick (steady_clock::now());
    while (!lolquit)
    {
        boost::this_thread::sleep_for(tick);
        auto current_time (steady_clock::now());
        auto delta (current_time - last_tick);
        last_tick = current_time;
        double sec (duration_cast<microseconds>(delta).count() * 1.0e-6);

        {
        auto write_lock (s.acquire_write_lock());
        //trace("physics timestep %1%", sec);
        system_gravity(s, sec);
        system_walk(s, sec);
        system_motion(s, sec);
        system_terrain_collision(s, terrain);
        system_terrain_friction(s, sec);
        }
    }
}

#ifdef _WIN32

HANDLE stopEvent;

BOOL WINAPI win32_signal_handler (DWORD)
{
    std::cout << "Signal caught" << std::endl;
    ::SetEvent(stopEvent);
    return TRUE;
}

#endif

int main (int argc, char* argv[])
{
    setup_minidump();
    auto& vm (global_settings);

    po::options_description generic("Command line options");
    generic.add_options()
        ("version,v", "print version string")
        ("help", "show help message");

    po::options_description config("Configuration");
    config.add_options()
        ("mode", po::value<std::string>()->default_value("singleplayer"),
            "server game mode")
        ("max-players", po::value<unsigned int>()->default_value(1),
            "maximum number of players")
        ("port", po::value<unsigned int>()->default_value(15556),
            "default port")
        ("server-name", po::value<std::string>()->default_value("Foo"),
            "server name")
        ("uid", po::value<std::string>()->default_value("nobody"),
            "drop to this user id after initialising the server")
        ("chroot", po::value<std::string>()->default_value("/tmp"),
            "chroot to this path after initialising the server")
        ("datadir", po::value<std::string>()->default_value(GAME_DATA_PATH),
            "the data directory")
        ("dbdir", po::value<std::string>()->default_value(default_db_path()),
            "the server database directory")
        ("game", po::value<std::string>()->default_value("defaultgame"),
            "which game to start")
        ;

    po::options_description cmdline;
    cmdline.add(generic).add(config);

    po::store(po::parse_command_line(argc, argv, cmdline), vm);

    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << cmdline << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version"))
    {
        std::cout << "hexahedra " << PROJECT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    if (enet_initialize() != 0)
    {
        std::cerr << "Could not initialize ENet, exiting" << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        std::string game_name (vm["game"].as<std::string>());
        fs::path datadir (vm["datadir"].as<std::string>());
        fs::path db_root (vm["dbdir"].as<std::string>());
        fs::path dbdir   (db_root / game_name);
        fs::path gamedir (datadir / std::string("games") / game_name );

        if (!fs::is_directory(datadir))
        {
            std::cerr << "Cannot open " << datadir.string() << std::endl;
            return -1;
        }

        if (!fs::is_directory(gamedir))
        {
            std::cerr << "Cannot open game '" << game_name << "'"
                      << std::endl;
            return -1;
        }

        if (!fs::is_directory(dbdir))
        {
            if (!fs::create_directories(dbdir))
            {
                std::cerr << "Cannot create directory '" << dbdir.string()
                          << "'" << std::endl;
                return -2;
            }
        }

#ifndef _WIN32
        // Block all signals for background thread.
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif
        // Start the game clock
        clock::init();

        boost::asio::io_service io_srv;

        // Set up the game world
        //hexa::network::connections_t players;
        fs::path db_file (dbdir / "world.db");

        trace("Game DB %1%", db_file.string());

        persistence_sqlite          db_per (io_srv, db_file, datadir / "dbsetup.sql");
        memory_cache                storage (db_per);
        hexa::server_entity_system  entities;
        hexa::world                 world (storage);
        hexa::lua                   scripting (entities, world);
        hexa::network               server (vm["port"].as<unsigned int>(), world, entities, scripting);

        scripting.uglyhack(&server);

        //std::cout << "Drop privileges" << std::endl;
        // We have all the file handles we need.  Now would be a good
        // time to drop our privileges.
        //drop_privileges(vm["uid"].as<std::string>(),
        //                vm["chroot"].as<std::string>());

        trace("Run Lua scripts");

        for(fs::recursive_directory_iterator i (gamedir);
            i != fs::recursive_directory_iterator(); ++i)
        {
            if (fs::is_regular_file(*i) && i->path().extension() == ".lua")
            {
                trace(i->path().string());
                std::cout << i->path().string() << std::endl;
                if (!scripting.load(i->path()))
                    throw std::runtime_error(scripting.get_error());
            }
        }

        boost::property_tree::ptree config;
        fs::path conf_file (gamedir / "setup.json");
        std::ifstream conf_str (conf_file.string());
        if (!conf_str)
            throw std::runtime_error(std::string("cannot open ") + conf_file.string());

        trace("Set up game world (%1%)", conf_file.string());

        boost::property_tree::read_json(conf_str, config);
        hexa::init_terrain_gen(world, config);
        boost::thread gameloop ([&]{ server.run(); });
        boost::thread physics_thread ([&]{ physics(entities, world); });
        trace("All systems go");

#ifndef _WIN32
        // Restore previous signals.
        pthread_sigmask(SIG_SETMASK, &old_mask, 0);

        // Wait for signal indicating time to shut down.
        sigset_t wait_mask;
        sigemptyset(&wait_mask);
        sigaddset(&wait_mask, SIGHUP);
        sigaddset(&wait_mask, SIGINT);
        sigaddset(&wait_mask, SIGQUIT);
        sigaddset(&wait_mask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
        int sig (0);
        sigwait(&wait_mask, &sig);
#else
        std::cout << "Running..." << std::endl;
        stopEvent = ::CreateEvent(NULL, TRUE, FALSE ,NULL);
        ::SetConsoleCtrlHandler(win32_signal_handler, TRUE);
        ::WaitForSingleObject(stopEvent, INFINITE);
        ::CloseHandle(stopEvent);
#endif

        trace("Stopping server...");
        server.jobs.push({ network::job::quit, chunk_coordinates(), 0 });

        trace("Stopping threads...");
        lolquit=true;
        physics_thread.join();
        gameloop.join();

        trace("Shutting down...");
    }
    catch (luabind::error& e)
    {
        std::cerr << "Uncaught Lua error: " << lua_tostring(e.state(), -1) << std::endl;
        return -1;
    }
    catch (boost::property_tree::ptree_error& e)
    {
        std::cerr << "Error in JSON: " << e.what() << std::endl;
        return -1;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "unknown exception caught"  << std::endl;
        return -1;
    }

    return EXIT_SUCCESS;
}
