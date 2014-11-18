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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <fstream>
#include <ctime>
#include <signal.h>
#include <thread>

#ifndef _WIN32
#include <pthread.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>

#include <hexanoise/generator_context.hpp>
#include <hexanoise/simple_global_variables.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/base58.hpp>
#include <hexa/config.hpp>
#include <hexa/json.hpp>
#include <hexa/os.hpp>
#include <hexa/protocol.hpp>
#include <hexa/drop_privileges.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/persistence_leveldb.hpp>
#include <hexa/trace.hpp>
#include <hexa/entity_system_physics.hpp>
#include <hexa/win32_minidump.hpp>
#include <hexa/log.hpp>

#include "clock.hpp"
#include "extract_surface.hpp"
#include "init_terrain_generators.hpp"
#include "lua.hpp"
#include "network.hpp"
#include "opencl.hpp"
#include "server_entity_system.hpp"
#include "server_registration.hpp"
#include "udp_server.hpp"
#include "world.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using boost::bind;
using boost::format;
using namespace hexa;

namespace hexa
{
po::variables_map global_settings;
fs::path gamedir;
}

static std::string default_db_path()
{
    return (app_user_dir() / fs::path(SERVER_DB_PATH)).string();
}

std::atomic<bool> lolquit(false);

void physics(server_entity_system& s, world& w)
{
    using namespace std::chrono;

    milliseconds tick(50);
    auto last_tick(steady_clock::now());
    while (!lolquit.load()) {
        std::this_thread::sleep_for(tick);
        auto current_time(steady_clock::now());
        auto delta_tick(current_time - last_tick);
        last_tick = current_time;
        double delta(duration_cast<microseconds>(delta_tick).count() * 1.0e-6);

        {
            auto read_world(w.acquire_read_access());
            auto write_lock(s.acquire_write_lock());
            while (delta > 0) {
                constexpr double max_step = 0.05;
                double step;
                if (delta > max_step) {
                    step = max_step;
                    delta -= max_step;
                } else if (delta < 0.001) {
                    break;
                } else {
                    step = delta;
                    delta = 0;
                }
                system_gravity(s, step);
                system_walk(s, step);
                system_motion(s, step);
                system_terrain_collision(
                    s, [&](chunk_coordinates c)
                           -> boost::optional<const surface_data&> {
                           return read_world.get_surface(c);
                       },
                    [&](chunk_coordinates c) {
                        return read_world.is_air_chunk(c);
                    });
                system_terrain_friction(s, step);
            }
        }
    }
}

void ping_auth_server(server_registration info)
{
    using namespace std::chrono;
    size_t tick = 87; // Wait a few seconds before announcing we're online

    while (!lolquit.load()) {
        if (++tick >= 90) {
            tick = 0;
            try {
                ping_server(info);
            } catch (std::exception& e) {
                std::cerr << "ping_auth_server: " << e.what() << std::endl;
            }
        }
        std::this_thread::sleep_for(seconds(1));
    }
    go_offline(info);
}

#ifdef _WIN32

HANDLE stopEvent;

BOOL WINAPI win32_signal_handler(DWORD)
{
    std::cout << "Signal caught" << std::endl;
    ::SetEvent(stopEvent);
    return TRUE;
}

#endif

int main(int argc, char* argv[])
{
    setup_minidump("hexahedra-server");
    auto& vm(global_settings);

    po::options_description generic("Command line options");
    generic.add_options()("version,v",
                          "print version string")("help", "show help message");

    po::options_description config("Configuration");
    config.add_options()(
        "mode", po::value<std::string>()->default_value("multiplayer"),
        "server game mode")("max-players",
                            po::value<unsigned int>()->default_value(10),
                            "maximum number of players")(
        "port", po::value<unsigned short>()->default_value(15556),
        "server port number")("server-name",
                              po::value<std::string>()->default_value("Foo"),
                              "server name")(
        "hostname", po::value<std::string>()->default_value(""),
        "publish server info with this domain name instead of my IP address")(
        "register",
        po::value<std::string>()->implicit_value("auth.hexahedra.net"),
        "advertise this server on a global list")(
        "passphrase", "generate the private key from a passphrase")(
        "uid", po::value<std::string>()->default_value("nobody"),
        "drop to this user id after initialising the server")(
        "chroot", po::value<std::string>()->default_value(""),
        "chroot to this path after initialising the server")(
        "datadir", po::value<std::string>()->default_value(GAME_DATA_PATH),
        "the data directory")(
        "dbdir", po::value<std::string>()->default_value(default_db_path()),
        "the server database directory")(
        "game", po::value<std::string>()->default_value("defaultgame"),
        "which game to start")("log", po::value<bool>()->default_value(true),
                               "log debug info to file");

    po::options_description cmdline;
    cmdline.add(generic).add(config);

    po::store(po::parse_command_line(argc, argv, cmdline), vm);

    po::notify(vm);

    if (vm.count("help")) {
        std::cout << cmdline << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version")) {
        std::cout << "hexahedra " << GIT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    std::ofstream logfile;
    if (vm["log"].as<bool>()) {
        logfile.open((app_user_dir() / "hexahedra-server_log.txt").string());
        if (logfile) {
            set_log_output(logfile);
            log_msg("Server started");
        } else {
            std::cerr << "Warning: could not open logfile in "
                      << temp_dir().string() << std::endl;
            set_log_output(std::cout);
        }
    }

    if (vm.count("passphrase")) {
        try {
            std::cout << "Passphrase:" << std::endl;
            std::string pphr;
            std::getline(std::cin, pphr);
            boost::algorithm::trim(pphr);
            use_private_key_from_password(pphr);
        } catch (std::runtime_error& e) {
            std::cerr << "\nCould not generate private key: " << e.what()
                      << std::endl;
            return -1;
        }
    }

    log_msg("Initializing OpenCL...");
    init_opencl();
    if (have_opencl())
        log_msg("OpenCL activated");
    else
        log_msg("No OpenCL support, fallback to native implementation");

    log_msg("Initializing Enet...");
    if (enet_initialize() != 0) {
        log_msg("Could not initialize ENet, exiting");
        return EXIT_FAILURE;
    }
    log_msg("ENet initialized");

    try {
        std::string game_name(vm["game"].as<std::string>());
        fs::path datadir(vm["datadir"].as<std::string>());
        fs::path db_root(vm["dbdir"].as<std::string>());
        fs::path dbdir(db_root / game_name);

        gamedir = fs::path(datadir / std::string("games") / game_name);

        if (!fs::is_directory(datadir)) {
            log_msg("Datadir '%1%' is not a directory", datadir.string());
            return -1;
        }

        if (!fs::is_directory(gamedir)) {
            log_msg("Gamedir '%1%' is not a directory", datadir.string());
            return -1;
        }

        if (!fs::is_directory(dbdir)) {
            if (!fs::create_directories(dbdir)) {
                log_msg("Cannot create dir %1%", dbdir.string());
                return -2;
            }
        }

        crypto::buffer server_id;
        std::thread ping_server_thread;
        if (vm["mode"].as<std::string>() == "multiplayer"
            && vm.count("register")) {
            try {
                auto info = register_server(vm["register"].as<std::string>());
                server_id = base58_decode(info.uid);
                log_msg(
                    "Registered server. API token is %1%, server ID is %2%",
                    info.api_token, info.uid);
                std::thread tmp{ping_auth_server, info};
                ping_server_thread.swap(tmp);
            } catch (std::runtime_error& e) {
                log_msg("Failed to register server: %1%", e.what());
            }
        }
        if (server_id.empty()) {
            try {
                server_id = base58_decode(
                    server_info().get<std::string>("server.id"));
                log_msg("Server ID read from settings: %1%",
                        base58_encode(server_id));
            } catch (...) {
                server_id = crypto::make_random(8);
                log_msg("Generated a totally random server ID: %1%",
                        base58_encode(server_id));
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

        init_surface_extraction();

        // Set up the game world
        // hexa::network::connections_t players;
        fs::path db_file(dbdir / "world.leveldb");

        trace("Game DB %1%", db_file.string());
        log_msg("Server game DB: %1%", db_file.string());

        persistence_leveldb db_per(db_file);
        hexa::server_entity_system entities;
        hexa::world world(db_per);
        hexa::lua scripting(entities, world);
        hexa::network server(vm["port"].as<unsigned short>(), world, entities,
                             scripting);

        scripting.uglyhack(&server);

        // std::cout << "Drop privileges" << std::endl;
        // We have all the file handles we need.  Now would be a good
        // time to drop our privileges.
        // drop_privileges(vm["uid"].as<std::string>(),
        //                vm["chroot"].as<std::string>());

        for (fs::recursive_directory_iterator i(gamedir);
             i != fs::recursive_directory_iterator(); ++i) {
            if (fs::is_regular_file(*i) && i->path().extension() == ".lua") {
                log_msg("Read Lua script %1%", i->path().string());
                if (!scripting.load(i->path()))
                    throw std::runtime_error(scripting.get_error());
            }
        }

        fs::path conf_file(gamedir / "setup.json");
        auto config = read_json(conf_file);
        log_msg("Set up game world from %1%", conf_file.string());
        hexa::init_terrain_gen(world, config);

        log_msg("Read entity database");
        db_per.retrieve(entities);

        std::thread gameloop(
            [&] { server.run(get_server_private_key(), server_id); });
        std::thread physics_thread([&] { physics(entities, world); });
        log_msg("All systems go");

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
        int sig(0);
        sigwait(&wait_mask, &sig);
#else
        std::cout << "Running..." << std::endl;
        stopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        ::SetConsoleCtrlHandler(win32_signal_handler, TRUE);
        ::WaitForSingleObject(stopEvent, INFINITE);
        ::CloseHandle(stopEvent);
#endif

        log_msg("Stopping network...");
        server.stop();

        log_msg("Stopping server...");
        server.jobs.push({network::job::quit, chunk_coordinates(), 0});

        log_msg("Stopping threads...");
        lolquit.store(true);
        physics_thread.join();
        gameloop.join();
        ping_server_thread.join();

        log_msg("Saving state...");
        db_per.store(entities);

        log_msg("Shutting down...");
    } catch (luabind::error& e) {
        log_msg("Uncaught Lua error: %1%", lua_tostring(e.state(), -1));
        return -1;
    } catch (boost::property_tree::ptree_error& e) {
        log_msg("Error in JSON: %1%", e.what());
        return -1;
    } catch (std::runtime_error& e) {
        log_msg("Runtime error: %1%", e.what());
        return -1;
    } catch (std::exception& e) {
        log_msg("Uncaught exception: %1%", e.what());
        return -1;
    } catch (...) {
        log_msg("Unknown exception caught");
        return -1;
    }

    return EXIT_SUCCESS;
}
