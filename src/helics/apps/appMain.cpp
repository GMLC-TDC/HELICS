/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Player.hpp"
#include "Recorder.hpp"
#include "Echo.hpp"
#include "Tracer.hpp"
#include "Source.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>

void showHelp()
{
    std::cout << "helics_app <appName> <appArguments>...\n";
    std::cout << "available apps: echo, source, player, recorder, broker, tracer\n";
    std::cout << " helics_app -? or --help shows this help\n";
        std::cout << "helics_app <appName> --help for application specific help\n";
        std::cout << "helics_app --version or -v will show the helics version string\n";
}
int main (int argc, char *argv[])
{
    if (argc == 1)
    {
        showHelp();
        return 0;
    }
    std::string arg1(argv[1]);
    //now redo the arguments remove the second argument which is the app name
    argc -= 1;
    for (int ii = 2; ii <= argc; ++ii)
    {
        argv[ii - 1] = argv[ii];
    }
    try
    {
        if (boost::iequals(arg1,"player"))
        {
            helics::apps::Player Player(argc, argv);
            if (Player.isActive())
            {
                Player.run();
            }

        }
        else if (boost::iequals(arg1, "recorder"))
        {
            helics::apps::Recorder Recorder(argc, argv);
            if (Recorder.isActive())
            {
                Recorder.run();
            }

        }
        else if ((arg1 == "--version")||(arg1=="-v"))
        {
            std::cout << "helics_app\n" << helics::versionString << '\n';
        }
        else if ((arg1 == "--help") || (arg1 == "-?"))
        {
            showHelp();
        }
        else if (boost::iequals(arg1, "echo"))
        {
            helics::apps::Echo Echo(argc, argv);
            if (Echo.isActive())
            {
                Echo.run();
            }
        }
        else if (boost::iequals(arg1, "source"))
        {
            helics::apps::Source Source(argc, argv);
            if (Source.isActive())
            {
                Source.run();
            }
        }
        else if (boost::iequals(arg1, "broker"))
        {

        }
        else if (boost::iequals(arg1, "tracer"))
        {
            helics::apps::Tracer Tracer(argc, argv);
            if (Tracer.isActive())
            {
                Tracer.enableTextOutput();
                Tracer.run();
            }
        }
        else
        {
            std::cout << "ERROR:  unrecognized app name\n";
            showHelp();
        }
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << ia.what() << std::endl;
        helics::cleanupHelicsLibrary();
        return (-2);
    }
    catch (const helics::HelicsException &he)
    {
        std::cerr << he.what() << std::endl;
        helics::cleanupHelicsLibrary();
        return (-4);
    }


    helics::cleanupHelicsLibrary();
    return 0;

}

