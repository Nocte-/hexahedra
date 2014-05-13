//---------------------------------------------------------------------------
// client/main.cpp
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

#include <cassert>
#include <iostream>
#include <signal.h>
#include <ctime>
#include <functional>
#include <GL/glew.h>
#include <GL/gl.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>

#include <enet/enet.h>

#include <hexa/config.hpp>
#include <hexa/drop_privileges.hpp>
#include <hexa/log.hpp>
#include <hexa/os.hpp>
#include <hexa/packet.hpp>
#include <hexa/protocol.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/win32_minidump.hpp>

#include "sfml_ogl2.hpp"
#include "sfml_ogl3.hpp"
#include "game.hpp"
#include "main_game.hpp"
#include "main_menu.hpp"
#include "player_info.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using boost::bind;
using boost::ref;
using namespace hexa;

namespace hexa {

// Accessible by other modules
po::variables_map global_settings;

}

int main (int argc, char* argv[])
{
    setup_minidump("hexahedra-client");
    trace("Trace on");

    auto& vm (global_settings);
    po::options_description generic("Command line options");
    generic.add_options()
        ("version,v",   "print version string")
        ("help",        "show help message")
        //("newsingle",   "Immediately start a new single-player game")
        //("continue",    "Continue the last game")
        //("quickjoin",   "Join your favorite server")
        ;

    po::options_description config("Configuration");
    config.add_options()
        ("hostname", po::value<std::string>()->default_value("localhost"),
            "server name")
        ("port", po::value<uint16_t>()->default_value(15556),
            "default port")
        ("viewdist", po::value<unsigned int>()->default_value(20),
            "view distance in chunks")
        ("vsync", po::value<bool>()->default_value(true),
            "use v-sync")
        ("ogl2",
            "Force the use of the OpenGL 2.0 backend")
        ("db", po::value<std::string>()->default_value("world.db"),
            "local database file name")
        ("uid", po::value<std::string>()->default_value("nobody"),
            "drop to this user id after initialising the client")
        ("datadir", po::value<std::string>()->default_value(GAME_DATA_PATH),
            "where to find the game's assets")
        ("userdir", po::value<std::string>()->default_value(app_user_dir().string()),
            "user's game directory")
        ;


    po::options_description cmdline;
    cmdline.add(generic).add(config);

    try
    {
        po::store(po::parse_command_line(argc, argv, cmdline), vm);

        fs::path inifile (app_user_dir() / "hexahedra.ini");
        if (fs::exists(inifile))
            po::store(po::parse_config_file<char>(inifile.string().c_str(), config), vm);

        po::notify(vm);
    }
    catch (po::unknown_option& e)
    {
        std::cerr << "Unrecognized option: " << e.get_option_name() << std::endl;
        std::cerr << cmdline << std::endl;
        return EXIT_FAILURE;
    }
    catch (std::exception& e)
    {
        std::cerr << "Could not parse options: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help"))
    {
        std::cout << cmdline << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version"))
    {
        std::cout << "hexahedra " << GIT_VERSION << std::endl;
        return EXIT_SUCCESS;
    }

    std::ofstream logfile((app_user_dir() / "hexahedra_log.txt").string());
    if (logfile)
    {
        set_log_output(logfile);
    }
    else
    {
        std::cerr << "Warning: could not open logfile in " << temp_dir().string() << std::endl;
        set_log_output(std::cout);
    }

    auto plinf (get_player_info());
    trace("Player name : %1%", plinf.name);
    trace("Player uid  : %1%", plinf.uid);
    trace("Player key  : %1%", plinf.public_key);

    log_msg("Initializing Enet");
    auto enet_rc (enet_initialize());
    if (enet_rc != 0)
    {
        log_msg("Enet init failed");
        std::cerr << "Could not initialize ENet, exiting" << std::endl;
        return EXIT_FAILURE;
    }
    log_msg("Enet running");

    try
    {
        hexa::game game_states ("Hexahedra", 1200, 800);
        game_states.run(game_states.make_state<hexa::main_menu>());
        log_msg("Shut down...");
    }
    catch (std::exception& e)
    {
        log_msg(e.what());
        return -1;
    }
    catch (...)
    {
        log_msg("unknown exception caught");
        return -1;
    }

    enet_deinitialize();
    log_msg("end");

    return EXIT_SUCCESS;
}

