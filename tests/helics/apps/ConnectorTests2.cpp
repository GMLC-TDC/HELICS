/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"
#include <cstdio>
#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif

#include "helics/ValueFederates.hpp"
#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Connector.hpp"
#include "helics/apps/CoreApp.hpp"

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class MultibrokerFixture: public ::testing::Test {
  public:
    MultibrokerFixture(): brokerA(helics::CoreType::TEST, "-f7")
    {
        //** setup brokers and cores*/

        subBroker1 = helics::BrokerApp(helics::CoreType::TEST,
                                       std::string("--broker=") + brokerA.getIdentifier());
        subBroker2 = helics::BrokerApp(helics::CoreType::TEST,
                                       std::string("--broker=") + brokerA.getIdentifier());

        subsubBroker2 = helics::BrokerApp(helics::CoreType::TEST,
                                          std::string("--broker=") + subBroker2.getIdentifier());

        coreA = helics::CoreApp(helics::CoreType::TEST,
                                std::string("--broker=") + brokerA.getIdentifier());
        core_sub1 = helics::CoreApp(helics::CoreType::TEST,
                                    std::string("--broker=") + subBroker1.getIdentifier());
        core_sub1B = helics::CoreApp(helics::CoreType::TEST,
                                     std::string("--broker=") + subBroker1.getIdentifier());

        core_sub2 = helics::CoreApp(helics::CoreType::TEST,
                                    std::string("--broker=") + subBroker2.getIdentifier());

        core_subsub1 = helics::CoreApp(helics::CoreType::TEST,
                                       std::string("--broker=") + subsubBroker2.getIdentifier());
        /** setup the federates*/

        fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);

        fedA = std::make_shared<helics::ValueFederate>("fedA", coreA, fedInfo);
        fedA->registerGlobalInput<double>("inputA");
        fedA->registerGlobalPublication<double>("pubD");

        fedSubA = std::make_shared<helics::ValueFederate>("fedSubA", core_sub1, fedInfo);

        fedSubA->registerGlobalInput<double>("inputB");
        fedSubA->registerGlobalPublication<double>("pubF");

        fedSubA2 = std::make_shared<helics::ValueFederate>("fedSubA2", core_sub1, fedInfo);
        fedSubA2->registerGlobalInput<double>("inputC");
        fedSubA2->registerGlobalPublication<double>("pubA");

        fedSubA2b = std::make_shared<helics::ValueFederate>("fedSubA2b", core_sub1B, fedInfo);

        fedSubA2b->registerGlobalInput<double>("inputD");
        fedSubA2b->registerGlobalPublication<double>("pubE");

        fedSubB = std::make_shared<helics::ValueFederate>("fedSubB", core_sub2, fedInfo);
        fedSubB->registerGlobalInput<double>("inputE");
        fedSubB->registerGlobalPublication<double>("pubB");
        fedSubSub1 = std::make_shared<helics::ValueFederate>("fedSubSub1", core_subsub1, fedInfo);
        fedSubSub1->registerGlobalInput<double>("inputF");
        fedSubSub1->registerGlobalPublication<double>("pubC");

        conn1 = std::make_shared<helics::apps::Connector>("connector1", coreA, fedInfo);
    }

    void SetUp()
    {
        // code here will execute just before the test ensues
    }

    void TearDown() { helics::cleanupHelicsLibrary(); }

    void launchThreads()
    {
        threads.resize(7);
        threads[0] = std::thread([this]() {
            fedA->enterExecutingMode();
            fedA->finalize();
        });

        threads[1] = std::thread([this]() {
            fedSubA->enterExecutingMode();
            fedSubA->finalize();
        });
        threads[2] = std::thread([this]() {
            fedSubA2->enterExecutingMode();
            fedSubA2->finalize();
        });
        threads[3] = std::thread([this]() {
            fedSubA2b->enterExecutingMode();
            fedSubA2b->finalize();
        });
        threads[4] = std::thread([this]() {
            fedSubB->enterExecutingMode();
            fedSubB->finalize();
        });
        threads[5] = std::thread([this]() {
            fedSubSub1->enterExecutingMode();
            fedSubSub1->finalize();
        });
        threads[6] = std::thread([this]() { conn1->run(); });
    }

    void closeThreads()
    {
        threads[6].join();

        threads[0].join();
        threads[1].join();
        threads[2].join();
        threads[3].join();
        threads[4].join();
        threads[5].join();
    }

    ~MultibrokerFixture() {}

    helics::BrokerApp brokerA;
    helics::BrokerApp subBroker1;
    helics::BrokerApp subBroker2;
    helics::BrokerApp subsubBroker2;
    helics::CoreApp coreA;
    helics::CoreApp core_sub1;
    helics::CoreApp core_sub1B;
    helics::CoreApp core_sub2;
    helics::CoreApp core_subsub1;
    std::shared_ptr<helics::ValueFederate> fedA;
    std::shared_ptr<helics::ValueFederate> fedSubA;
    std::shared_ptr<helics::ValueFederate> fedSubA2;
    std::shared_ptr<helics::ValueFederate> fedSubA2b;
    std::shared_ptr<helics::ValueFederate> fedSubB;
    std::shared_ptr<helics::ValueFederate> fedSubSub1;
    helics::FederateInfo fedInfo;
    std::shared_ptr<helics::apps::Connector> conn1;
    std::vector<std::thread> threads;
};

TEST_F(MultibrokerFixture, multibroker_connector)
{
    using helics::apps::InterfaceDirection;

    conn1->addConnection("pubA", "inputA", InterfaceDirection::FROM_TO);
    conn1->addConnection("pubB", "inputB", InterfaceDirection::FROM_TO);
    conn1->addConnection("pubC", "inputC", InterfaceDirection::FROM_TO);
    conn1->addConnection("pubD", "inputD", InterfaceDirection::FROM_TO);
    conn1->addConnection("pubE", "inputE", InterfaceDirection::FROM_TO);
    conn1->addConnection("pubF", "inputF", InterfaceDirection::FROM_TO);

    launchThreads();
    closeThreads();

    EXPECT_EQ(conn1->madeConnections(), 6);
}

TEST_F(MultibrokerFixture, multibroker_connector_aliases)
{
    using helics::apps::InterfaceDirection;

    conn1->addConnection("publicationA", "inpA", InterfaceDirection::FROM_TO);
    conn1->addConnection("publicationB", "inpB", InterfaceDirection::FROM_TO);
    conn1->addConnection("publicationC", "inpC", InterfaceDirection::FROM_TO);
    conn1->addConnection("publicationD", "inpD", InterfaceDirection::FROM_TO);
    conn1->addConnection("publicationE", "inpE", InterfaceDirection::FROM_TO);
    conn1->addConnection("publicationF", "inpF", InterfaceDirection::FROM_TO);

    brokerA.addAlias("publicationA", "pubA");
    brokerA.addAlias("publicationB", "pubB");
    brokerA.addAlias("publicationC", "pubC");
    brokerA.addAlias("publicationD", "pubD");
    brokerA.addAlias("publicationE", "pubE");
    brokerA.addAlias("publicationF", "pubF");

    brokerA.addAlias("inputA", "inpA");
    brokerA.addAlias("inputB", "inpB");
    brokerA.addAlias("inputC", "inpC");
    brokerA.addAlias("inputD", "inpD");
    brokerA.addAlias("inputE", "inpE");
    brokerA.addAlias("inputF", "inpF");

    launchThreads();
    closeThreads();

    EXPECT_EQ(conn1->madeConnections(), 6);
}

TEST_F(MultibrokerFixture, multibroker_connector_regex)
{
    using helics::apps::InterfaceDirection;

    conn1->addConnection("REGEX:publication(?<value>.)",
                         "REGEX:inp(?<value>.)",
                         InterfaceDirection::FROM_TO);

    brokerA.addAlias("publicationA", "pubA");
    brokerA.addAlias("publicationB", "pubB");
    brokerA.addAlias("publicationC", "pubC");
    brokerA.addAlias("publicationD", "pubD");
    brokerA.addAlias("publicationE", "pubE");
    brokerA.addAlias("publicationF", "pubF");

    brokerA.addAlias("inputA", "inpA");
    brokerA.addAlias("inputB", "inpB");
    brokerA.addAlias("inputC", "inpC");
    brokerA.addAlias("inputD", "inpD");
    brokerA.addAlias("inputE", "inpE");
    brokerA.addAlias("inputF", "inpF");
    launchThreads();
    closeThreads();

    EXPECT_EQ(conn1->madeConnections(), 6);
}

TEST_F(MultibrokerFixture, multibroker_connector_regex2)
{
    using helics::apps::InterfaceDirection;
    conn1->addConnection("REGEX:(?<letter>.)publication(?<value>.)",
                         "REGEX:inp(?<value>.)(?<letter>.)",
                         InterfaceDirection::FROM_TO);

    brokerA.addAlias("pubA", "Apublication1");
    brokerA.addAlias("pubB", "Bpublication2");
    brokerA.addAlias("pubC", "Cpublication3");
    brokerA.addAlias("pubD", "Dpublication4");
    brokerA.addAlias("pubE", "Epublication5");
    brokerA.addAlias("pubF", "Fpublication6");

    brokerA.addAlias("inputA", "inp1A");
    brokerA.addAlias("inputB", "inp2B");
    brokerA.addAlias("inputC", "inp3C");
    brokerA.addAlias("inputD", "inp4D");
    brokerA.addAlias("inputE", "inp5E");
    brokerA.addAlias("inputF", "inp6F");

    launchThreads();
    closeThreads();

    EXPECT_EQ(conn1->madeConnections(), 6);
}

TEST_F(MultibrokerFixture, multibroker_connector_regex_default_tag)
{
    using helics::apps::InterfaceDirection;

    conn1->addConnection("REGEX:publication(?<value>.)",
                         "REGEX:inp(?<value>.)",
                         InterfaceDirection::FROM_TO,
                         {"default"});

    brokerA.addAlias("publicationA", "pubA");
    brokerA.addAlias("publicationB", "pubB");
    brokerA.addAlias("publicationC", "pubC");
    brokerA.addAlias("publicationD", "pubD");
    brokerA.addAlias("publicationE", "pubE");
    brokerA.addAlias("publicationF", "pubF");

    brokerA.addAlias("inputA", "inpA");
    brokerA.addAlias("inputB", "inpB");
    brokerA.addAlias("inputC", "inpC");
    brokerA.addAlias("inputD", "inpD");
    brokerA.addAlias("inputE", "inpE");
    brokerA.addAlias("inputF", "inpF");

    launchThreads();
    closeThreads();

    EXPECT_EQ(conn1->madeConnections(), 6);
}
