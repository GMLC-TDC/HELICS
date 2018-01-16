/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "player.h"
#include "recorder.h"
#include "../core/BrokerFactory.h"
#include "../core/core-exceptions.h"
#include <iostream>

void showHelp()
{
    std::cout << "helics_app <appName> <appArguments>...\n";
    std::cout << "available apps: echo, source, player, recorder, broker\n";
    std::cout << " helics_app -? or --help shows this help\n";
        std::cout << "helics_app <appName> --help for application specific help\n";
        std::cout << "helics_app --version will show the helics version string\n";
}
int main (int argc, char *argv[])
{ 
    if (argc == 1)
    {
        showHelp();
    }
    std::string arg1(argv[1]);
    //now redo the arguments remove the second argument which is the app name
    argc -= 1;
    for (int ii = 2; ii < argc; ++ii)
    {
        argv[ii - 1] = argv[ii];
    }
    try
    {
        if (arg1 == "player")
        {
            helics::Player Player(argc, argv);
            Player.run();
        }
        else if (arg1 == "recorder")
        {
            helics::Recorder Recorder(argc, argv);
            Recorder.run();
        }
        else if ((arg1 == "--version")||(arg1=="-v"))
        {
            std::cout << "helics_app\n" << helics::getHelicsVersionString() << '\n';
        }
        else if ((arg1 == "--help") || (arg1 == "-?"))
        {
            showHelp();
        }
        else if (arg1 == "echo")
        {

        }
        else if (arg1 == "source")
        {

        }
        else if (arg1 == "broker")
        {

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