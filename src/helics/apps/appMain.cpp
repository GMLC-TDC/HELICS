/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../core/BrokerFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include "BrokerApp.hpp"
#include "Echo.hpp"
#include "Player.hpp"
#include "Recorder.hpp"
#include "Source.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>

void showHelp ()
{
    std::cout << "helics_app <appName> <appArguments>...\n";
    std::cout << "available apps: echo, source, player, recorder, broker, tracer\n";
    std::cout << " helics_app -?, -h, or --help shows this help\n";
    std::cout << "helics_app <appName> --help for application specific help\n";
    std::cout << "helics_app --version or -v will show the helics version string\n";
}
int main (int argc, char *argv[])
{
    if (argc == 1)
    {
        showHelp ();
        return 0;
    }
    std::string arg1 (argv[1]);
    int ret = 0;
    // now redo the arguments remove the second argument which is the app name
    argc -= 1;
    for (int ii = 2; ii <= argc; ++ii)
    {
        argv[ii - 1] = argv[ii];
    }
    try
    {
        if (boost::iequals (arg1, "player"))
        {
            helics::apps::Player player (argc, argv);
            if (player.isActive ())
            {
                player.run ();
            }
        }
        else if (boost::iequals (arg1, "recorder"))
        {
            helics::apps::Recorder recorder (argc, argv);
            if (recorder.isActive ())
            {
                recorder.run ();
            }
        }
        else if ((arg1 == "--version") || (arg1 == "-v"))
        {
            std::cout << "helics_app\n" << helics::versionString << '\n';
        }
        else if ((arg1 == "--help") || (arg1 == "-?") || (arg1 == "-h"))
        {
            showHelp ();
        }
        else if (boost::iequals (arg1, "echo"))
        {
            helics::apps::Echo echo (argc, argv);
            if (echo.isActive ())
            {
                echo.run ();
            }
        }
        else if (boost::iequals (arg1, "source"))
        {
            helics::apps::Source source (argc, argv);
            if (source.isActive ())
            {
                source.run ();
            }
        }
        else if (boost::iequals (arg1, "broker"))
        {
            helics::apps::BrokerApp broker (argc, argv);
            // broker just waits on the destructor if it was active so this is all we do
        }
        else if (boost::iequals (arg1, "tracer"))
        {
            helics::apps::Tracer Tracer (argc, argv);
            if (Tracer.isActive ())
            {
                Tracer.enableTextOutput ();
                Tracer.run ();
            }
        }
        else
        {
            std::cout << "ERROR:  unrecognized app name\n";
            showHelp ();
        }
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << ia.what () << std::endl;
        ret = -2;
    }
    catch (const helics::HelicsException &he)
    {
        std::cerr << he.what () << std::endl;
        ret = -4;
    }

    helics::cleanupHelicsLibrary ();
    return ret;
}
