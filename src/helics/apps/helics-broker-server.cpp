/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../application_api/BrokerApp.hpp"
#include "../application_api/Federate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../apps/BrokerServer.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "gmlc/utilities/stringOps.h"

#include <chrono>
#include <iostream>
#include <thread>

/** function to run the online terminal program*/
void terminalFunction(std::vector<std::string> args);

int main(int argc, char* argv[])
{
    int ret{0};
    bool runterminal{false};

    helics::helicsCLI11App cmdLine("helics broker server command line");
    auto* term =
        cmdLine
            .add_subcommand("term",
                            "helics_broker_server term will start a broker server "
                            "and open a terminal control window "
                            "for the broker server, run help in a terminal for more commands\n")
            ->prefix_command();
    term->callback([&runterminal]() { runterminal = true; });
    helics::Time opTime(30, time_units::minutes);
    cmdLine
        .add_option(
            "--duration",
            opTime,
            "specify the length of time the server should run before closing the server and waiting for "
            "generated brokers to complete")
        ->default_str("30 minutes");
    cmdLine
        .footer(
            "helics_broker_server starts the broker servers with the given args and waits for a given duration "
            "to close the servers and wait until all generated brokers have finished\n")
        ->footer([]() {
            helics::apps::BrokerServer brk(std::vector<std::string>{"-?"});
            (void)brk;
            return std::string{};
        });
    cmdLine.allow_extras();
    cmdLine.set_config();
    auto res = cmdLine.helics_parse(argc, argv);
    if (res != helics::helicsCLI11App::parse_output::ok) {
        switch (res) {
            case helics::helicsCLI11App::parse_output::help_call:
            case helics::helicsCLI11App::parse_output::help_all_call:
            case helics::helicsCLI11App::parse_output::version_call:
                return 0;
            default:
                return static_cast<int>(res);
        }
    }
    try {
        if (runterminal) {
            terminalFunction(cmdLine.remaining_for_passthrough());
        } else {
            auto brokerServer =
                std::make_unique<helics::apps::BrokerServer>(cmdLine.remaining_for_passthrough());
            std::cout << "starting broker server\n";
            brokerServer->startServers();
            std::cout << "servers started\n";
            std::this_thread::sleep_for(opTime.to_ms());
            std::cout
                << "Helics broker server time limit reached, servers closing (to change duration use `--duration=X`)"
                << std::endl;
            brokerServer->closeServers();
            std::cout << "waiting for brokers to close" << std::endl;
            // once we have closed the servers now wait for the active brokers to finish
            auto brokers = helics::BrokerFactory::getAllBrokers();
            for (auto& broker : brokers) {
                broker->waitForDisconnect();
                broker.reset();
            }
            brokers.clear();
        }
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << ia.what() << std::endl;
        ret = -2;
    }
    catch (const helics::HelicsException& he) {
        std::cerr << he.what() << std::endl;
        ret = -4;
    }

    helics::cleanupHelicsLibrary();
    return ret;
}

/** function to control a user terminal for the broker*/
void terminalFunction(std::vector<std::string> args)
{
    auto brokerServer = std::make_unique<helics::apps::BrokerServer>(args);
    std::cout << "starting broker server\n";
    brokerServer->startServers();
    std::cout << "broker servers started\n";
    auto closeBrokerServer = [&brokerServer]() {
        if (!brokerServer) {
            std::cout << "Broker servers have terminated\n";
            return;
        }
        brokerServer->forceTerminate();
        while (brokerServer->hasActiveBrokers()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (!brokerServer->hasActiveBrokers()) {
            std::cout << "Broker servers have terminated\n";
        }
    };
    auto lsbrokers = []() {
        auto brks = helics::BrokerFactory::getAllBrokers();
        int ii = 1;
        for (auto& brk : brks) {
            std::cout << ii << ": " << brk->getIdentifier() << " Connected:" << brk->isConnected()
                      << " open:" << brk->isOpenToNewFederates() << '\n';
        }
    };

    std::vector<std::string> bargs;
    auto newBroker = [&bargs]() {
        std::reverse(bargs.begin(), bargs.end());
        try {
            helics::BrokerApp brk(bargs);

            std::cout << "broker has started: " << brk->isConnected() << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
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
            std::cout << "Broker (" << id << ") is connected and " << ((accepting) ? "is" : "is
    not")
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
    */
    bool cmdcont = true;
    helics::helicsCLI11App termProg("helics broker server command line terminal");
    termProg.ignore_case();
    termProg.add_flag(
        "-q{false},--quit{false},--exit{false}",
        cmdcont,
        "stop the broker servers, close the terminal and wait for the brokers to exit");
    termProg.add_subcommand("quit", "close the terminal and  wait for the brokers to exit")
        ->callback([&cmdcont]() { cmdcont = false; });
    termProg.add_subcommand("ls", "list all brokers")->callback(lsbrokers);
    termProg.add_subcommand("terminate", "terminate the broker servers")
        ->callback(closeBrokerServer);
    auto* brokersub =
        termProg.add_subcommand("broker", "create a new broker with the given arguments")
            ->callback(newBroker);

    brokersub->add_option("args", bargs, "arguments for the query");

    termProg
        .add_subcommand("force_terminate",
                        "forcibly terminate the broker servers, shutdown all brokers and exit")
        ->callback([closeBrokerServer, &cmdcont]() {
            cmdcont = false;
            closeBrokerServer();
        });
    /*
    auto restart =
      termProg.add_subcommand ("restart", "restart the broker if it is not currently
    executing")->allow_extras (); restart->callback ( [restartBroker, &restart] () { restartBroker
    (restart->remaining_for_passthrough (), false); });

    auto frestart =
      termProg.add_subcommand ("restart!", "forceably terminate the broker and restart
    it")->allow_extras (); frestart->callback ( [restartBroker, &restart] () { restartBroker
    (restart->remaining_for_passthrough (), true); });

    termProg.add_subcommand ("status", "generate the current status of the broker")->callback
    ([&status] () { status (false);
    });
    termProg.add_subcommand ("info", "get the current broker status and connection info")->callback
    ([&status] () { status (true);
    });
    */
    termProg.add_subcommand("help", "display the help")->callback([&termProg]() {
        termProg.helics_parse("-?");
    });

    std::vector<std::string> qargs;
    auto queryCall = [&qargs]() {
        std::shared_ptr<helics::Broker> brk{nullptr};

        std::string target;
        std::string query;
        if (qargs.size() >= 3) {
            brk = helics::BrokerFactory::findBroker(qargs[0]);
            target = qargs[1];
            query = qargs[2];
        } else {
            brk = helics::BrokerFactory::getConnectedBroker();
            if (qargs.size() == 2) {
                target = qargs[0];
                query = qargs[1];
            } else if (qargs.size() == 1) {
                target = "root";
                query = qargs[0];
            } else {
                target = "root";
                query = "status";
            }
        }
        std::string res = (brk) ? brk->query(target, query) : "#invalid";
        std::cout << res << std::endl;
    };

    auto* querySub = termProg.add_subcommand(
        "query",
        "make a query of some target >>query <broker> <target> <query> or query <target> <query> to a target on the current broker or query <query> to target the root federation of the current broker");
    querySub->add_option("args", qargs, "arguments for the query");

    querySub->preparse_callback([&args](size_t /*unused*/) { args.clear(); });
    querySub->callback(queryCall);

    while (cmdcont) {
        std::string cmdin;
        std::cout << "\nhelics_broker_server>>";
        std::getline(std::cin, cmdin);
        if (cmdin == "exit" || cmdin == "q") {  // provide a fast path to exit without going through
                                                // the terminal command line processor
            cmdcont = false;
            continue;
        }
        termProg.helics_parse(cmdin);
    }
}
