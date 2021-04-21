/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/apps/BrokerServer.hpp"

#include "helics/application_api.hpp"
#include "helics/core/CoreFactory.hpp"

#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    std::vector<std::string> args{"--zmq", "--http"};
    helics::apps::BrokerServer bserv(args);
    bserv.startServers();

    auto crA1 = helics::CoreFactory::create(helics::core_type::ZMQ, "--brokername=brokerA");

    crA1->connect();
    helics::FederateInfo fi;

    helics::CombinationFederate fedA1("fedA_1", crA1, fi);
    auto crA2 = helics::CoreFactory::create(helics::core_type::ZMQ, "--brokername=brokerA");

    fedA1.registerGlobalPublication<double>("pub1", "V");
    fedA1.registerPublication<std::string>("pub_string", "");
    fedA1.registerEndpoint("endpoint_local");
    auto& cf = fedA1.registerCloningFilter("cloner");
    cf.addSourceTarget("endpointA2_1");

    crA2->connect();
    helics::CombinationFederate fedA2("fedA_2", crA2, fi);
    fedA2.registerGlobalEndpoint("endpointA2_1");
    fedA2.registerGlobalEndpoint("endpointA2_2");
    auto& I2 = fedA2.registerGlobalInput<double>("input_A2");
    I2.addTarget("pub1");

    fedA2.registerSubscription("fedA_1/pub_string");

    auto crB1 = helics::CoreFactory::create(helics::core_type::ZMQ, "--brokername=brokerB");
    crB1->connect();
    helics::CombinationFederate fedB1("fedB_1", crB1, fi);

    auto crB2 = helics::CoreFactory::create(helics::core_type::ZMQ, "--brokername=brokerB");
    crB2->connect();
    helics::CombinationFederate fedB2("fedB_2", crB2, fi);
    std::chrono::seconds timeout(30);
    if (argc >= 2) {
        auto res = atoi(argv[1]);
        timeout = std::chrono::seconds(res);
    }
    fedA1.enterExecutingModeAsync();
    fedA2.enterExecutingMode();
    fedA1.enterExecutingModeComplete();

    std::this_thread::sleep_for(timeout);

    fedA1.finalize();
    fedA2.finalize();
    fedB1.finalize();
    fedB2.finalize();

    crB2->disconnect();
    crB1->disconnect();
    crA1->disconnect();
    crA2->disconnect();

    crB2.reset();
    crB1.reset();
    crA1.reset();
    crA2.reset();
    bserv.closeServers();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    bserv.forceTerminate();
}
