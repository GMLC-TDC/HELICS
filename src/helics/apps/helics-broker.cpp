/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../application_api/BrokerApp.hpp"
#include "../application_api/Federate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../core/Broker.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "MultiBroker.hpp"
#include "gmlc/utilities/stringOps.h"

#ifdef HELICS_ENABLE_WEBSERVER
#    include "../apps/RestApiConnection.hpp"
#    include "../apps/helicsWebServer.hpp"
#endif

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** function to run the online terminal program*/
void terminalFunction(std::vector<std::string> args);

/** function to run the remote terminal program*/
void remoteTerminalFunction(std::vector<std::string> args);

static const bool amb = helics::allowMultiBroker();

int main(int argc, char* argv[])  // NOLINT
{
    int ret{0};
    bool runterminal{false};
    bool runremote{false};
    bool autorestart{false};

    bool http_server{false};
    bool websocket_server{false};
#ifdef HELICS_ENABLE_WEBSERVER
    std::string mHttpArgs;
    std::string mWebSocketArgs;
#endif
    helics::helicsCLI11App cmdLine("helics broker command line");
    auto* term =
        cmdLine
            .add_subcommand(
                "term",
                "helics_broker term <broker args...> will start a broker and open a terminal control window "
                "for the broker run help in a terminal for more commands\n")
            ->prefix_command();
    term->callback([&runterminal]() { runterminal = true; });

    auto* remote =
        cmdLine
            .add_subcommand(
                "remote",
                "helics_broker remote <connection args>  will start a terminal and connect to a remote HELICS rest API and allow sending API commands through a terminal")
            ->prefix_command();
    remote->callback([&runremote]() { runremote = true; });

    cmdLine.add_flag(
        "--autorestart",
        autorestart,
        "helics_broker --autorestart <broker args ...> will start a continually regenerating broker "
        "there is a 3 second countdown on broker completion to halt the program via ctrl-C\n");

    cmdLine.add_flag("--http,--web",
                     http_server,
                     "start a webserver to respond to http rest api requests");
    cmdLine.add_flag("--websocket",
                     websocket_server,
                     "start a websocket server to respond to api requests");
#ifdef HELICS_ENABLE_WEBSERVER
    cmdLine
        .add_option("--http_server_args", mHttpArgs, "command line arguments for the http server")
        ->envname("HELICS_HTTP_ARGS");
    cmdLine
        .add_option("--websocket_server_args",
                    mWebSocketArgs,
                    "command line arguments for the websocket server")
        ->envname("HELICS_WEBSOCKET_ARGS");
#endif

    cmdLine
        .footer(
            "helics_broker <broker args ..> starts a broker with the given args and waits for it to "
            "complete\n")
        ->footer([]() {
            [[maybe_unused]] helics::BrokerApp app{"-?"};
            return std::string{};
        });
    cmdLine.allow_extras();
    cmdLine.set_config();
    cmdLine.addSystemInfoCall();
    auto parseResult = cmdLine.helics_parse(argc, argv);
    if (parseResult != helics::helicsCLI11App::ParseOutput::OK) {
        return (static_cast<int>(parseResult) > 0) ? 0 : static_cast<int>(parseResult);
    }
#ifdef HELICS_ENABLE_WEBSERVER
    std::shared_ptr<helics::apps::WebServer> webserver;
    if (http_server || websocket_server) {
        webserver = std::make_shared<helics::apps::WebServer>();
        webserver->enableHttpServer(http_server);
        if (!mHttpArgs.empty()) {
            webserver->processArgs(mHttpArgs);
        }
        webserver->enableWebSocketServer(websocket_server);
        if (!mWebSocketArgs.empty()) {
            webserver->processArgs(mWebSocketArgs);
        }
        webserver->startServer(nullptr, webserver);
    }
#else
    if (http_server || websocket_server) {
        std::cout
            << "the http webserver and websocket server are not available in this build please recompile with webserver enabled to utilize "
            << std::endl;
    }
#endif
    try {
        if (runterminal) {
            terminalFunction(cmdLine.remainArgs());
        } else if (runremote) {
            remoteTerminalFunction(cmdLine.remainArgs());
        } else if (autorestart) {
            while (true) {
                // I am purposely making an object that creates and destroys itself on the same line
                // because this will run until termination so will take a while
                {
                    helics::BrokerKeeper brkapp{cmdLine.remainArgs()};
                }
                std::cout << "broker restart in 3 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "broker restart in 2 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "broker restart in 1 seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "broker restarting" << std::endl;
            }
        } else {
            helics::BrokerKeeper broker(cmdLine.remainArgs());
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
#ifdef HELICS_ENABLE_WEBSERVER
    if (webserver) {
        webserver->stopServer();
    }
#endif
    helics::cleanupHelicsLibrary();
    return ret;
}

/** function to control a user terminal for the broker*/
void terminalFunction(std::vector<std::string> args)
{
    std::cout << "starting broker\n";
    auto broker = std::make_unique<helics::BrokerApp>(args);
    auto closeBroker = [&broker]() {
        if (!broker) {
            std::cout << "Broker has terminated\n";
            return;
        }
        broker->forceTerminate();
        while (broker->isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "Broker has terminated\n";
    };

    auto restartBroker = [&broker, &args](const std::vector<std::string>& broker_args, bool force) {
        if (!broker_args.empty()) {
            args = broker_args;
        }
        if (!broker) {
            broker = std::make_unique<helics::BrokerApp>(args);
            std::cout << "broker has started\n";
        } else if (broker->isConnected()) {
            if (force) {
                broker->forceTerminate();
                broker = nullptr;
                broker = std::make_unique<helics::BrokerApp>(args);
                std::cout << "broker was forcibly terminated and restarted\n";
            } else {
                std::cout << "broker is currently running unable to restart\n";
            }
        } else {
            broker = nullptr;
            broker = std::make_unique<helics::BrokerApp>(args);
            std::cout << "broker has restarted\n";
        }
    };

    auto status = [&broker](bool addAddress) {
        if (!broker) {
            std::cout << "Broker is not available\n";
            return;
        }
        auto accepting = (*broker)->isOpenToNewFederates();
        auto connected = (*broker)->isConnected();
        auto id = (*broker)->getIdentifier();
        if (connected) {
            std::cout << "Broker (" << id << ") is connected and "
                      << ((accepting) ? "is" : "is not") << "accepting new federates\n";
            if (addAddress) {
                auto address = (*broker)->getAddress();
                std::cout << address << '\n';
            } else {
                auto cts = (*broker)->query("broker", "counts");
                std::cout << cts << '\n';
            }
        } else {
            std::cout << "Broker (" << id << ") is not connected \n";
        }
    };
    bool cmdcont = true;
    helics::helicsCLI11App termProg("helics broker command line terminal");
    termProg.ignore_case();
    termProg.add_flag("-q{false},--quit{false},--exit{false}",
                      cmdcont,
                      "close the terminal and wait for the broker to exit");
    termProg.add_subcommand("quit", "close the terminal and  wait for the broker to exit")
        ->callback([&cmdcont]() { cmdcont = false; });
    termProg.add_subcommand("terminate", "terminate the broker")->callback(closeBroker);

    termProg.add_subcommand("terminate!", "forcibly terminate the broker and exit")
        ->callback([closeBroker, &cmdcont]() {
            cmdcont = false;
            closeBroker();
        });

    auto* restart =
        termProg.add_subcommand("restart", "restart the broker if it is not currently executing")
            ->allow_extras();
    restart->callback([restartBroker, &restart]() {
        restartBroker(restart->remaining_for_passthrough(), false);
    });

    auto* frestart =
        termProg.add_subcommand("restart!", "forcibly terminate the broker and restart it")
            ->allow_extras();
    frestart->callback(
        [restartBroker, &restart]() { restartBroker(restart->remaining_for_passthrough(), true); });

    termProg.add_subcommand("status", "generate the current status of the broker")
        ->callback([&status]() { status(false); });
    termProg.add_subcommand("info", "get the current broker status and connection info")
        ->callback([&status]() { status(true); });
    termProg.add_subcommand("help", "display the help")->callback([&termProg]() {
        termProg.helics_parse("-?");
    });
    std::string target;
    std::string query;

    auto queryCall = [&broker, &target, &query]() {
        if (!broker) {
            std::cout << "Broker is not available\n";
            return;
        }
        std::string res;
        if (target.empty()) {
            res = (*broker)->query("broker", query);
        } else {
            res = (*broker)->query(target, query);
        }
        auto qvec = helics::vectorizeQueryResult(std::move(res));
        std::cout << "results: ";
        for (const auto& vres : qvec) {
            std::cout << vres << '\n';
        }
    };
    auto* querySub = termProg.add_subcommand(
        "query",
        "make a query of some target >>query <target> <query> or query <query> to query the broker");
    auto* qgroup1 = querySub->add_option_group("targetGroup")->enabled_by_default();
    qgroup1->add_option("target", target, "the name of object to target");
    auto* qgroup2 = querySub->add_option_group("queryGroup");
    qgroup2->add_option("query", query, "the query to make")->required();
    querySub->preparse_callback([qgroup1, &target](size_t argcount) {
        if (argcount < 2) {
            target.clear();
            qgroup1->disabled();
        }
    });
    querySub->callback(queryCall);
    while (cmdcont) {
        std::string cmdin;
        std::cout << "\nhelics_broker>>";
        std::getline(std::cin, cmdin);
        if (cmdin == "exit" || cmdin == "q") {  // provide a fast path to exit without going through
                                                // the terminal command line processor
            cmdcont = false;
            continue;
        }
        termProg.helics_parse(cmdin);
    }
}
#ifdef HELICS_ENABLE_WEBSERVER
/** function to control a user terminal for the broker*/
void remoteTerminalFunction(std::vector<std::string> args)
{
    // args order is reversed from parsing
    std::string port = std::to_string(helics::apps::WebServer::defaultHttpPort);
    std::string server{"localhost"};

    std::cout << "starting broker remote terminal \n";
    auto connection = std::make_unique<helics::apps::RestApiConnection>("local");
    if (args.size() >= 2) {
        port = args[0];
        server = args[1];
    }
    if (!args.empty()) {
        server = args[0];
    }
    connection->connect(server, port);
    auto closeConnection = [&connection]() {
        if (connection) {
            connection->disconnect();
        }
        std::cout << "connection has terminated\n";
    };

    auto reConnect = [&](const std::vector<std::string>& connectArgs) {
        if (!connection) {
            connection = std::make_unique<helics::apps::RestApiConnection>("local");
            std::cout << "connection has started\n";
        }
        if (connectArgs.size() > 2) {
            connection->connect(connectArgs[1], connectArgs[0]);
        } else if (connectArgs.size() == 1) {
            connection->connect(connectArgs[0], port);
        } else {
            connection->connect(server, port);
        }
    };

    auto get = [&connection](const std::string& query) {
        if (!connection) {
            std::cout << "connection is not available\n";
            return;
        }
        std::cout << connection->sendGet(query) << std::endl;
    };

    auto put = [&connection](const std::string& target, const std::string& body) {
        if (!connection) {
            std::cout << "connection is not available\n";
            return;
        }
        std::cout << connection->sendCommand(boost::beast::http::verb::put, target, body)
                  << std::endl;
    };

    auto post = [&connection](const std::string& target, const std::string& body) {
        if (!connection) {
            std::cout << "connection is not available\n";
            return;
        }
        std::cout << connection->sendCommand(boost::beast::http::verb::post, target, body)
                  << std::endl;
    };

    auto search = [&connection](const std::string& target) {
        if (!connection) {
            std::cout << "connection is not available\n";
            return;
        }
        std::cout << connection->sendCommand(boost::beast::http::verb::search, target, "")
                  << std::endl;
    };

    auto del = [&connection](const std::string& target) {
        if (!connection) {
            std::cout << "connection is not available\n";
            return;
        }
        std::cout << connection->sendCommand(boost::beast::http::verb::delete_, target, "")
                  << std::endl;
    };

    bool cmdcont = true;
    helics::helicsCLI11App termProg("helics remote API command line terminal");
    termProg.ignore_case();
    termProg.add_flag("-q{false},--quit{false},--exit{false}",
                      cmdcont,
                      "close the terminal and wait for the broker to exit");
    termProg.add_subcommand("quit", "close the terminal and  wait for the broker to exit")
        ->callback([&cmdcont]() { cmdcont = false; });
    termProg.add_subcommand("terminate", "terminate the broker")->callback(closeConnection);

    auto* restart =
        termProg.add_subcommand("restart", "restart the broker if it is not currently executing")
            ->allow_extras();
    restart->callback(
        [&reConnect, &restart]() { reConnect(restart->remaining_for_passthrough()); });

    std::string target;
    std::string body;
    // GET call
    auto* getsub = termProg.add_subcommand("get", "run a get query on the server")
                       ->callback([&get, &target]() { get(target); });
    getsub->add_option("target,--target", target, "specify the target uri to GET")->required();
    // POST call
    auto* postsub = termProg.add_subcommand("post", "post data to the server")
                        ->callback([&post, &target, &body]() { post(target, body); });
    postsub->add_option("target,--target", target, "specify the target uri to POST")->required();
    postsub->add_option("body,--body", body, "specify the body of the POST message");

    // PUT call
    auto* putsub = termProg.add_subcommand("put", "put data to the server")
                       ->callback([&put, &target, &body]() { put(target, body); });
    putsub->add_option("target,--target", target, "specify the target uri to PUT")->required();
    putsub->add_option("body,--body", body, "specify the body of the PUT message");

    // DELETE call
    auto* delsub =
        termProg.add_subcommand("delete", "delete data from a server")->callback([&del, &target]() {
            del(target);
        });
    delsub->add_option("target,--target", target, "specify the target uri to DELETE")->required();

    // DELETE call
    auto* searchsub = termProg.add_subcommand("search", "run a search on the server")
                          ->callback([&search, &target]() { search(target); });
    searchsub->add_option("target,--target", target, "specify the target uri to SEARCH")
        ->required();

    termProg.add_subcommand("help", "display the help")->callback([&termProg]() {
        termProg.helics_parse("-?");
    });

    while (cmdcont) {
        std::string cmdin;
        std::cout << "\nhelics_remote>>";
        std::getline(std::cin, cmdin);
        if (cmdin == "exit" || cmdin == "q") {  // provide a fast path to exit without going through
                                                // the terminal command line processor
            cmdcont = false;
            continue;
        }
        termProg.helics_parse(cmdin);
    }
}

#else
void remoteTerminalFunction(std::vector<std::string> /*args*/)
{
    std::cout << "webserver not enabled terminating broker" << std::endl;
}
#endif
