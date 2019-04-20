/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../application_api/Federate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/stringOps.h"
#include "../core/Broker.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "BrokerApp.hpp"
#include <iostream>
#include <thread>
#include <boost/algorithm/string.hpp>
#if HELICS_HAVE_ZEROMQ > 0
#include "../common/zmqContextManager.h"
#include "cppzmq/zmq.hpp"
#endif
/** function to run the online terminal program*/
void terminalFunction (const std::vector<std::string> &args);

int main (int argc, char *argv[])
{
    int ret = 0;
    bool runterminal = false;
    bool autorestart = false;

    auto cmdLine = helics::helicsCLI11App ("helics broker command line");
    auto term =
      cmdLine
        .add_subcommand (
          "term", "helics_broker term <broker args...> will start a broker and open a terminal control window "
                  "for the broker run help in a terminal for more commands\n")
        ->prefix_command ();
    term->callback ([&runterminal]() { runterminal = true; });
    cmdLine.add_flag ("--autorestart", autorestart,
                      "helics_broker autorestart <broker args ...> will start a continually regenerating broker "
                      "there is a 3 second countdown on broker completion to halt the program via ctrl-C\n");
    cmdLine.footer ("helics_broker <broker args ..> just starts a broker with the given args and waits for it to "
                    "complete\n");
    cmdLine.allow_extras ();
    auto res = cmdLine.helics_parse (argc, argv);

    try
    {
        if (runterminal)
        {
            terminalFunction (cmdLine.remaining_args);
        }
        else if (autorestart)
        {
            while (true)
            {
                // I am purposely making an object that creates and destroys itself on the same line because this
                // will run until termination so will take a while
                auto rem = cmdLine.remaining_args ();
                helics::apps::BrokerApp (rem);
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
            helics::apps::BrokerApp broker (cmdLine.remaining_args);
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

#if HELICS_HAVE_ZEROMQ > 0
#ifdef __APPLE__
    if (ZmqContextManager::setContextToLeakOnDelete ())
    {
        ZmqContextManager::getContext ().close ();
    }
#endif
#endif
    helics::cleanupHelicsLibrary ();
    return ret;
}

/** function to control a user terminal for the broker*/
void terminalFunction (const std::vector<std::string> &args)
{
    std::cout << "starting broker\n";
    auto broker = std::make_unique<helics::apps::BrokerApp> (args);
    auto closeBroker = [&broker]() {
        if (!broker)
        {
            std::cout << "Broker has terminated\n";
            return;
        }
        broker->forceTerminate ();
        while (broker->isActive ())
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
        if (!broker->isActive ())
        {
            std::cout << "Broker has terminated\n";
        }
    };

    auto restartBroker = [&broker](std::vector<std::string> args) {
        if (!broker)
        {
            broker = std::make_unique<helics::apps::BrokerApp> (std::move (args));
            std::cout << "broker has started\n";
        }
        else if (broker->isActive ())
        {
            std::cout << "broker is currently running unable to restart\n";
        }
        else
        {
            broker = nullptr;
            broker = std::make_unique<helics::apps::BrokerApp> (std::move (args));
            std::cout << "broker has restarted\n";
        }
    };

    bool cmdcont = true;
    auto termProg = helics::helicsCLI11App ("helics broker command line terminal");
    termProg.ignore_case ();
    termProg.add_flag ("-q,--quit,--exit", cmdcont, "close the terminal and wait for the broker to exit");
    termProg.add_subcommand ("quit")->callback ([&cmdcont]() { cmdcont = false; });
    termProg.add_subcommand ("terminate")->callback (closeBroker);

    termProg.add_subcommand ("terminate*")->callback ([closeBroker, &cmdcont]() {
        cmdcont = false;
        closeBroker ();
    });

    auto restart = termProg.add_subcommand ("restart")->allow_extras ();
    restart->callback ([restartBroker, restart]() { restartBroker (restart->remaining_for_passthrough ()); });
    while (cmdcont)
    {
        std::string cmdin;
        std::cout << "helics>>";
        std::getline (std::cin, cmdin);
        auto cmdVec = stringOps::splitlineQuotes (cmdin, " ", stringOps::default_quote_chars,
                                                  stringOps::delimiter_compression::on);
        auto cmd1 = convertToLowerCase (cmdVec[0]);
        stringOps::trimString (cmd1);

        else if ((cmd1 == "help") || (cmd1 == "?"))
        {
            std::cout << "`quit` -> close the terminal application and wait for broker to finish\n";
            std::cout << "`terminate` -> force the broker to stop\n";
            std::cout << "`terminate*` -> force the broker to stop and exit the terminal application\n";
            std::cout << "`help`,`?` -> this help display\n";
            std::cout << "`restart` -> restart a completed broker\n";
            std::cout << "`status` -> will display the current status of the broker\n";
            std::cout << "`info` -> will display info about the broker\n";
            std::cout << "`force restart` -> will force terminate a broker and restart it\n";
            std::cout << "`query` <queryString> -> will query a broker for <queryString>\n";
            std::cout << "`query` <queryTarget> <queryString> -> will query <queryTarget> for <queryString>\n";
        }

        else if (cmd1 == "force")
        {
            if (!broker)
            {
                broker = std::make_unique<helics::apps::BrokerApp> (argc, argv);
            }
            else if ((cmdVec.size () >= 2) && (cmdVec[1] == "restart"))
            {
                if (broker->isActive ())
                {
                    broker->forceTerminate ();
                    broker = nullptr;
                    broker = std::make_unique<helics::apps::BrokerApp> (argc, argv);
                }
                else
                {
                    broker = nullptr;
                    broker = std::make_unique<helics::apps::BrokerApp> (argc, argv);
                }
            }
        }
        else if (cmd1 == "status")
        {
            if (!broker)
            {
                std::cout << "Broker is not available\n";
                continue;
            }
            auto accepting = (*broker)->isOpenToNewFederates ();
            auto connected = (*broker)->isConnected ();
            auto id = (*broker)->getIdentifier ();
            if (connected)
            {
                auto cts = (*broker)->query ("broker", "counts");
                std::cout << "Broker (" << id << ") is connected and " << ((accepting) ? "is" : "is not")
                          << "accepting new federates\n"
                          << cts << '\n';
            }
            else
            {
                std::cout << "Broker (" << id << ") is not connected \n";
            }
        }
        else if (cmd1 == "info")
        {
            if (!broker)
            {
                std::cout << "Broker is not available\n";
                continue;
            }
            auto accepting = (*broker)->isOpenToNewFederates ();
            auto connected = (*broker)->isConnected ();
            auto id = (*broker)->getIdentifier ();
            if (connected)
            {
                auto address = (*broker)->getAddress ();
                std::cout << "Broker (" << id << ") is connected and " << ((accepting) ? "is" : "is not")
                          << " accepting new federates\naddress=" << address << '\n';
            }
            else
            {
                std::cout << "Broker (" << id << ") is not connected \n";
            }
        }
        else if (cmd1 == "query")
        {
            if (!broker)
            {
                std::cout << "Broker is not available\n";
                continue;
            }
            std::string res;
            if (cmdVec.size () == 2)
            {
                res = (*broker)->query ("broker", cmdVec[1]);
            }
            else if (cmdVec.size () >= 3)
            {
                res = (*broker)->query (cmdVec[1], cmdVec[2]);
            }
            auto qvec = vectorizeQueryResult (std::move (res));
            std::cout << "results: ";
            for (const auto &vres : qvec)
            {
                std::cout << vres << '\n';
            }
        }
    }
}
