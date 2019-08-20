/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../application_api/Federate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../apps/BrokerServer.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "gmlc/utilities/stringOps.h"
#include <iostream>
#include <thread>

/** function to run the online terminal program*/
void terminalFunction (std::vector<std::string> args);

int main (int argc, char *argv[])
{
    int ret = 0;
    bool runterminal = false;
    bool autorestart = false;

    helics::helicsCLI11App cmdLine ("helics broker command line");
    auto term = cmdLine
                  .add_subcommand ("term", "helics-broker-server term <broker args...> will start a broker server "
                                           "and open a terminal control window "
                                           "for the broker run help in a terminal for more commands\n")
                  ->prefix_command ();
    term->callback ([&runterminal] () { runterminal = true; });
    cmdLine.add_flag ("--autorestart", autorestart,
                      "helics_broker autorestart <broker args ...> will start a continually regenerating broker "
                      "there is a 3 second countdown on broker completion to halt the program via ctrl-C\n");
    cmdLine
      .footer ("helics_broker <broker args ..> starts a broker with the given args and waits for it to "
               "complete\n")
      ->footer ([] () {
          helics::apps::BrokerServer ("-?");
          return std::string{};
      });
    cmdLine.allow_extras ();
    auto res = cmdLine.helics_parse (argc, argv);
    if (res != helics::helicsCLI11App::parse_output::ok)
    {
        switch (res)
        {
        case helics::helicsCLI11App::parse_output::help_call:
        case helics::helicsCLI11App::parse_output::help_all_call:
        case helics::helicsCLI11App::parse_output::version_call:
            return 0;
        default:
            return static_cast<int> (res);
        }
    }
    try
    {
        if (runterminal)
        {
            terminalFunction (cmdLine.remaining_for_passthrough ());
        }
        else
        {
            helics::apps::BrokerServer brokerServer (cmdLine.remaining_for_passthrough ());
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
void terminalFunction (std::vector<std::string> args)
{
    std::cout << "starting broker server\n";
    auto brokerServer = std::make_unique<helics::apps::BrokerServer> (args);

    auto closeBrokerServer = [&brokerServer] () {
        if (!brokerServer)
        {
            std::cout << "Broker has terminated\n";
            return;
        }
        brokerServer->forceTerminate ();
        while (brokerServer->hasActiveBrokers ())
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
        if (!brokerServer->hasActiveBrokers ())
        {
            std::cout << "Broker has terminated\n";
        }
    };
    /*
    auto restartBroker = [&broker, &args] (std::vector<std::string> broker_args, bool force) {
        if (!broker_args.empty ())
        {
            args = broker_args;
        }
        if (!broker)
        {
            broker = std::make_unique<helics::apps::BrokerApp> (args);
            std::cout << "broker has started\n";
        }
        else if (broker->isActive ())
        {
            if (force)
            {
                broker->forceTerminate ();
                broker = nullptr;
                broker = std::make_unique<helics::apps::BrokerApp> (args);
                std::cout << "broker was forceably terminated and restarted\n";
            }
            else
            {
                std::cout << "broker is currently running unable to restart\n";
            }
        }
        else
        {
            broker = nullptr;
            broker = std::make_unique<helics::apps::BrokerApp> (args);
            std::cout << "broker has restarted\n";
        }
    };

    auto status = [&broker] (bool addAddress) {
        if (!broker)
        {
            std::cout << "Broker is not available\n";
            return;
        }
        auto accepting = (*broker)->isOpenToNewFederates ();
        auto connected = (*broker)->isConnected ();
        auto id = (*broker)->getIdentifier ();
        if (connected)
        {
            std::cout << "Broker (" << id << ") is connected and " << ((accepting) ? "is" : "is not")
                      << "accepting new federates\n";
            if (addAddress)
            {
                auto address = (*broker)->getAddress ();
                std::cout << address << '\n';
            }
            else
            {
                auto cts = (*broker)->query ("broker", "counts");
                std::cout << cts << '\n';
            }
        }
        else
        {
            std::cout << "Broker (" << id << ") is not connected \n";
        }
    };
    bool cmdcont = true;
    helics::helicsCLI11App termProg ("helics broker command line terminal");
    termProg.ignore_case ();
    termProg.add_flag ("-q,--quit,--exit", cmdcont, "close the terminal and wait for the broker to exit");
    termProg.add_subcommand ("quit", "close the terminal and  wait for the broker to exit")
      ->callback ([&cmdcont] () { cmdcont = false; });
    termProg.add_subcommand ("terminate", "terminate the broker")->callback (closeBroker);

    termProg.add_subcommand ("terminate!", "forceably terminate the broker and exit")
      ->callback ([closeBroker, &cmdcont] () {
          cmdcont = false;
          closeBroker ();
      });

    auto restart =
      termProg.add_subcommand ("restart", "restart the broker if it is not currently executing")->allow_extras ();
    restart->callback (
      [restartBroker, &restart] () { restartBroker (restart->remaining_for_passthrough (), false); });

    auto frestart =
      termProg.add_subcommand ("restart!", "forceably terminate the broker and restart it")->allow_extras ();
    frestart->callback (
      [restartBroker, &restart] () { restartBroker (restart->remaining_for_passthrough (), true); });

    termProg.add_subcommand ("status", "generate the current status of the broker")->callback ([&status] () {
        status (false);
    });
    termProg.add_subcommand ("info", "get the current broker status and connection info")->callback ([&status] () {
        status (true);
    });
    termProg.add_subcommand ("help", "display the help")->callback ([&termProg] () {
        termProg.helics_parse ("-?");
    });
    std::string target;
    std::string query;

    auto queryCall = [&broker, &target, &query] () {
        if (!broker)
        {
            std::cout << "Broker is not available\n";
            return;
        }
        std::string res;
        if (target.empty ())
        {
            res = (*broker)->query ("broker", query);
        }
        else
        {
            res = (*broker)->query (target, query);
        }
        auto qvec = vectorizeQueryResult (std::move (res));
        std::cout << "results: ";
        for (const auto &vres : qvec)
        {
            std::cout << vres << '\n';
        }
    };
    auto querySub = termProg.add_subcommand (
      "query", "make a query of some target >>query <target> <query> or query <query> to query the broker");
    auto qgroup1 = querySub->add_option_group ("targetGroup")->enabled_by_default ();
    qgroup1->add_option ("target", target, "the name of object to target");
    auto qgroup2 = querySub->add_option_group ("queryGroup");
    qgroup2->add_option ("query", query, "the query to make")->required ();
    querySub->preparse_callback ([qgroup1, &target] (size_t argcount) {
        if (argcount < 2)
        {
            target.clear ();
            qgroup1->disabled ();
        }
    });
    querySub->callback (queryCall);
    while (cmdcont)
    {
        std::string cmdin;
        std::cout << "\nhelics>>";
        std::getline (std::cin, cmdin);
        termProg.helics_parse (cmdin);
    }
    */
}
