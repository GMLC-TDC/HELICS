/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../application_api/Federate.hpp"
#include "../core/core-exceptions.hpp"
#include "BrokerApp.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "../common/stringOps.h"

void terminalFunction (int argc, char *argv[]);

int main (int argc, char *argv[])
{
    int ret = 0;
    bool runterminal = false;
    bool autorestart = false;
    auto firstarg = std::string (argv[1]);
    if (boost::iequals (firstarg, "term"))
    { //if this is true run a user terminal
        // now redo the arguments remove the second argument which is term command
        
        runterminal = true;
    }
    else if (boost::iequals (firstarg, "autorestart"))
	{
        autorestart = true;
	}
    else if ((boost::iequals (firstarg, "help")) || (firstarg == "-?") || (firstarg == "--help"))
    {
        std::cout << "helics_broker term <broker args...> will start a broker and open a terminal control window "
                     "for the broker run help in a terminal for more commands\n";
        std::cout << "helics_broker autorestart <broker args ...> will start a continually regenerating broker "
                     "there is a 3 second countdown on broker completion to halt the program via ctrl-C\n";
        std::cout << "helics_broker <broker args ..> just starts a broker with the given args and waits for it to "
                     "complete\n";
		if (boost::iequals(firstarg, "help"))
		{
			return ret;
		}
    }
	if ((runterminal) || (autorestart))
	{
        argc -= 1;
        for (int ii = 2; ii <= argc; ++ii)
        {
            argv[ii - 1] = argv[ii];
        }
	}
	
    
    try
    {
		if (runterminal)
		{
            terminalFunction (argc, argv);
		}
		else if (autorestart)
		{
			while (true)
			{
				//I am purposely making an object that creates and destroys itself on the same line
                helics::apps::BrokerApp(argc, argv);
                std::cout << "broker restart in 3 seconds" << std::endl;
                std::this_thread::sleep_for (std::chrono::seconds (1));
                std::cout << "broker restart in 2 seconds" << std::endl;
                std::this_thread::sleep_for (std::chrono::seconds (1));
                std::cout << "broker restart in 1 seconds" << std::endl;
                std::this_thread::sleep_for (std::chrono::seconds (1));
                std::cout << "broker restarting" << std::endl;
			}
		}
		else
		{
            helics::apps::BrokerApp broker (argc, argv);
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

/** function to control a user terminal for the broker*/
void terminalFunction (int argc, char *argv[])
{
    std::cout << "starting broker\n";
    auto broker=std::make_unique<helics::apps::BrokerApp>(argc, argv);
    bool cmdcont = true;
	while (cmdcont)
	{
        std::string cmdin;
        std::cout << "helics>>";
        std::cin >> cmdin;
        auto cmdVec = stringOps::splitlineQuotes (cmdin," ",stringOps::default_quote_chars,stringOps::delimiter_compression::on);
        auto cmd1 = convertToLowerCase (cmdVec[0]);
        stringOps::trimString (cmd1);
		if ((cmd1 == "quit")||(cmd1=="q"))
		{
            break;
		}
		else if (cmd1 == "terminate")
		{
            broker->forceTerminate ();
		}
		else if (cmd1 == "help")
		{
            std::cout << "quit >>close the terminal application and wait for broker to finish\n";
            std::cout << "terminate >> force the broker to stop\n";
            std::cout << "help >> this help window\n";
            std::cout << "restart >> restart a completed broker\n";
		}
		else if (cmd1 == "restart")
		{
			if (broker->isActive())
			{
                std::cout << "broker is currently running unable to restart\n";
			}
			else
			{
                broker = nullptr;
                broker = std::make_unique<helics::apps::BrokerApp> (argc, argv);
			}
		}

	}
   
}