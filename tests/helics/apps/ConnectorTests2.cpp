/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"
#include <cstdio>
#ifndef DISABLE_SYSTEM_CALL_TESTS
#    include "exeTestHelper.h"
#endif

#include "helics/apps/BrokerApp.hpp"
#include "helics/apps/Connector.hpp"
#include "helics/apps/CoreApp.hpp"
#include "helics/ValueFederates.hpp"

#include <future>
#include <thread>

TEST(connector_tests2, multibroker_connector)
{
    using helics::apps::InterfaceDirection;
    /** setup brokers and cores*/
    helics::BrokerApp brokerA(helics::CoreType::TEST,"brokerA","-f7");

    helics::BrokerApp subBroker1(helics::CoreType::TEST,"subBroker1","--broker=brokerA");
    helics::BrokerApp subBroker2(helics::CoreType::TEST,"subBroker2","--broker=brokerA");

    helics::BrokerApp subsubBroker2(helics::CoreType::TEST,"subsubBroker2","--broker=subBroker2");

   helics::CoreApp coreA(helics::CoreType::TEST,"--name=coreA --broker=brokerA");
   helics::CoreApp core_sub1(helics::CoreType::TEST,"--name=core_sub1 --broker=subBroker1");
    helics::CoreApp core_sub1B(helics::CoreType::TEST,"--name=core_sub1B --broker=subBroker1");

    helics::CoreApp core_sub2(helics::CoreType::TEST,"--name=core_sub2 --broker=subBroker2");

    helics::CoreApp core_subsub1(helics::CoreType::TEST,"--name=core_subsub1 --broker=subsubBroker2");
    /** setup the federates*/
    helics::FederateInfo fedInfo;
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    
    auto fedA=helics::ValueFederate("fedA",coreA,fedInfo);
    fedA.registerGlobalInput<double>("inputA");
    fedA.registerGlobalPublication<double>("pubD");

    auto fedSubA=helics::ValueFederate("fedSubA",core_sub1,fedInfo);

    fedSubA.registerGlobalInput<double>("inputB");
    fedSubA.registerGlobalPublication<double>("pubF");

    auto fedSubA2=helics::ValueFederate("fedSubA2",core_sub1,fedInfo);
    fedSubA2.registerGlobalInput<double>("inputC");
    fedSubA2.registerGlobalPublication<double>("pubA");

    auto fedSubA2b=helics::ValueFederate("fedSubA2b",core_sub1B,fedInfo);

    fedSubA2b.registerGlobalInput<double>("inputD");
    fedSubA2b.registerGlobalPublication<double>("pubE");

    auto fedSubB=helics::ValueFederate("fedSubB",core_sub2,fedInfo);
    fedSubB.registerGlobalInput<double>("inputE");
    fedSubB.registerGlobalPublication<double>("pubB");
    auto fedSubSub1=helics::ValueFederate("fedSubSub1",core_subsub1,fedInfo);
    fedSubSub1.registerGlobalInput<double>("inputF");
    fedSubSub1.registerGlobalPublication<double>("pubC");

    helics::apps::Connector conn1("connector1",coreA, fedInfo);
    conn1.addConnection("pubA", "inputA", InterfaceDirection::FROM_TO);
    conn1.addConnection("pubB", "inputB", InterfaceDirection::FROM_TO);
    conn1.addConnection("pubC", "inputC", InterfaceDirection::FROM_TO);
    conn1.addConnection("pubD", "inputD", InterfaceDirection::FROM_TO);
    conn1.addConnection("pubE", "inputE", InterfaceDirection::FROM_TO);
    conn1.addConnection("pubF", "inputF", InterfaceDirection::FROM_TO);

    auto futA=std::async(std::launch::async,[&fedA](){fedA.enterExecutingMode();fedA.finalize();});
    auto futSubA=std::async(std::launch::async,[&fedSubA](){fedSubA.enterExecutingMode();fedSubA.finalize();});
    auto futSubA2=std::async(std::launch::async,[&fedSubA2](){fedSubA2.enterExecutingMode();fedSubA2.finalize();});
    auto futSubB=std::async(std::launch::async,[&fedSubB](){fedSubB.enterExecutingMode();fedSubB.finalize();});
    auto futSubA2b=std::async(std::launch::async,[&fedSubA2b](){fedSubA2b.enterExecutingMode();fedSubA2b.finalize();});
    auto futSubSub1=std::async(std::launch::async,[&fedSubSub1](){fedSubSub1.enterExecutingMode();fedSubSub1.finalize();});
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });
    

    
    fut.get();
    futSubSub1.get();
    futSubA2b.get();
    futSubB.get();
    futSubA2.get();
    futSubA.get();
    futA.get();

    EXPECT_EQ(conn1.madeConnections(), 6);
}


TEST(connector_tests2, multibroker_connector_aliases)
{
    using helics::apps::InterfaceDirection;
    /** setup brokers and cores*/
    helics::BrokerApp brokerA(helics::CoreType::TEST,"brokerA","-f7");

    helics::BrokerApp subBroker1(helics::CoreType::TEST,"subBroker1","--broker=brokerA");
    helics::BrokerApp subBroker2(helics::CoreType::TEST,"subBroker2","--broker=brokerA");

    helics::BrokerApp subsubBroker2(helics::CoreType::TEST,"subsubBroker2","--broker=subBroker2");

    helics::CoreApp coreA(helics::CoreType::TEST,"--name=coreA --broker=brokerA");
    helics::CoreApp core_sub1(helics::CoreType::TEST,"--name=core_sub1 --broker=subBroker1");
    helics::CoreApp core_sub1B(helics::CoreType::TEST,"--name=core_sub1B --broker=subBroker1");

    helics::CoreApp core_sub2(helics::CoreType::TEST,"--name=core_sub2 --broker=subBroker2");

    helics::CoreApp core_subsub1(helics::CoreType::TEST,"--name=core_subsub1 --broker=subsubBroker2");
    /** setup the federates*/
    helics::FederateInfo fedInfo;
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);

    auto fedA=helics::ValueFederate("fedA",coreA,fedInfo);
    fedA.registerGlobalInput<double>("inputA");
    fedA.registerGlobalPublication<double>("pubD");

    auto fedSubA=helics::ValueFederate("fedSubA",core_sub1,fedInfo);

    fedSubA.registerGlobalInput<double>("inputB");
    fedSubA.registerGlobalPublication<double>("pubF");

    auto fedSubA2=helics::ValueFederate("fedSubA2",core_sub1,fedInfo);
    fedSubA2.registerGlobalInput<double>("inputC");
    fedSubA2.registerGlobalPublication<double>("pubA");

    auto fedSubA2b=helics::ValueFederate("fedSubA2b",core_sub1B,fedInfo);

    fedSubA2b.registerGlobalInput<double>("inputD");
    fedSubA2b.registerGlobalPublication<double>("pubE");

    auto fedSubB=helics::ValueFederate("fedSubB",core_sub2,fedInfo);
    fedSubB.registerGlobalInput<double>("inputE");
    fedSubB.registerGlobalPublication<double>("pubB");
    auto fedSubSub1=helics::ValueFederate("fedSubSub1",core_subsub1,fedInfo);
    fedSubSub1.registerGlobalInput<double>("inputF");
    fedSubSub1.registerGlobalPublication<double>("pubC");

    helics::apps::Connector conn1("connector1",coreA, fedInfo);
    conn1.addConnection("publicationA", "inpA", InterfaceDirection::FROM_TO);
    conn1.addConnection("publicationB", "inpB", InterfaceDirection::FROM_TO);
    conn1.addConnection("publicationC", "inpC", InterfaceDirection::FROM_TO);
    conn1.addConnection("publicationD", "inpD", InterfaceDirection::FROM_TO);
    conn1.addConnection("publicationE", "inpE", InterfaceDirection::FROM_TO);
    conn1.addConnection("publicationF", "inpF", InterfaceDirection::FROM_TO);

    brokerA.addAlias("publicationA","pubA");
    brokerA.addAlias("publicationB","pubB");
    brokerA.addAlias("publicationC","pubC");
    brokerA.addAlias("publicationD","pubD");
    brokerA.addAlias("publicationE","pubE");
    brokerA.addAlias("publicationF","pubF");

    brokerA.addAlias("inputA","inpA");
    brokerA.addAlias("inputB","inpB");
    brokerA.addAlias("inputC","inpC");
    brokerA.addAlias("inputD","inpD");
    brokerA.addAlias("inputE","inpE");
    brokerA.addAlias("inputF","inpF");

    auto futA=std::async(std::launch::async,[&fedA](){fedA.enterExecutingMode();fedA.finalize();});
    auto futSubA=std::async(std::launch::async,[&fedSubA](){fedSubA.enterExecutingMode();fedSubA.finalize();});
    auto futSubA2=std::async(std::launch::async,[&fedSubA2](){fedSubA2.enterExecutingMode();fedSubA2.finalize();});
    auto futSubB=std::async(std::launch::async,[&fedSubB](){fedSubB.enterExecutingMode();fedSubB.finalize();});
    auto futSubA2b=std::async(std::launch::async,[&fedSubA2b](){fedSubA2b.enterExecutingMode();fedSubA2b.finalize();});
    auto futSubSub1=std::async(std::launch::async,[&fedSubSub1](){fedSubSub1.enterExecutingMode();fedSubSub1.finalize();});
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });



    fut.get();
    futSubSub1.get();
    futSubA2b.get();
    futSubB.get();
    futSubA2.get();
    futSubA.get();
    futA.get();

    EXPECT_EQ(conn1.madeConnections(), 6);
}


TEST(connector_tests2, multibroker_connector_regex)
{
    helics::cleanupHelicsLibrary();
    using helics::apps::InterfaceDirection;
    /** setup brokers and cores*/
    helics::BrokerApp brokerA(helics::CoreType::TEST,"brokerA","-f7");

    helics::BrokerApp subBroker1(helics::CoreType::TEST,"subBroker1","--broker=brokerA");
    helics::BrokerApp subBroker2(helics::CoreType::TEST,"subBroker2","--broker=brokerA");

    helics::BrokerApp subsubBroker2(helics::CoreType::TEST,"subsubBroker2","--broker=subBroker2");

    helics::CoreApp coreA(helics::CoreType::TEST,"--name=coreA --broker=brokerA");
    helics::CoreApp core_sub1(helics::CoreType::TEST,"--name=core_sub1 --broker=subBroker1");
    helics::CoreApp core_sub1B(helics::CoreType::TEST,"--name=core_sub1B --broker=subBroker1");

    helics::CoreApp core_sub2(helics::CoreType::TEST,"--name=core_sub2 --broker=subBroker2");

    helics::CoreApp core_subsub1(helics::CoreType::TEST,"--name=core_subsub1 --broker=subsubBroker2");
    /** setup the federates*/
    helics::FederateInfo fedInfo;
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);

    auto fedA=helics::ValueFederate("fedA",coreA,fedInfo);
    fedA.registerGlobalInput<double>("inputA");
    fedA.registerGlobalPublication<double>("pubD");

    auto fedSubA=helics::ValueFederate("fedSubA",core_sub1,fedInfo);

    fedSubA.registerGlobalInput<double>("inputB");
    fedSubA.registerGlobalPublication<double>("pubF");

    auto fedSubA2=helics::ValueFederate("fedSubA2",core_sub1,fedInfo);
    fedSubA2.registerGlobalInput<double>("inputC");
    fedSubA2.registerGlobalPublication<double>("pubA");

    auto fedSubA2b=helics::ValueFederate("fedSubA2b",core_sub1B,fedInfo);

    fedSubA2b.registerGlobalInput<double>("inputD");
    fedSubA2b.registerGlobalPublication<double>("pubE");

    auto fedSubB=helics::ValueFederate("fedSubB",core_sub2,fedInfo);
    fedSubB.registerGlobalInput<double>("inputE");
    fedSubB.registerGlobalPublication<double>("pubB");
    auto fedSubSub1=helics::ValueFederate("fedSubSub1",core_subsub1,fedInfo);
    fedSubSub1.registerGlobalInput<double>("inputF");
    fedSubSub1.registerGlobalPublication<double>("pubC");

    helics::apps::Connector conn1("connector1",coreA, fedInfo);
    conn1.addConnection("REGEX:publication(?<value>.)", "REGEX:inp(?<value>.)", InterfaceDirection::FROM_TO);

    brokerA.addAlias("publicationA","pubA");
    brokerA.addAlias("publicationB","pubB");
    brokerA.addAlias("publicationC","pubC");
    brokerA.addAlias("publicationD","pubD");
    brokerA.addAlias("publicationE","pubE");
    brokerA.addAlias("publicationF","pubF");

    brokerA.addAlias("inputA","inpA");
    brokerA.addAlias("inputB","inpB");
    brokerA.addAlias("inputC","inpC");
    brokerA.addAlias("inputD","inpD");
    brokerA.addAlias("inputE","inpE");
    brokerA.addAlias("inputF","inpF");

    auto futA=std::async(std::launch::async,[&fedA](){fedA.enterExecutingMode();fedA.finalize();});
    auto futSubA=std::async(std::launch::async,[&fedSubA](){fedSubA.enterExecutingMode();fedSubA.finalize();});
    auto futSubA2=std::async(std::launch::async,[&fedSubA2](){fedSubA2.enterExecutingMode();fedSubA2.finalize();});
    auto futSubB=std::async(std::launch::async,[&fedSubB](){fedSubB.enterExecutingMode();fedSubB.finalize();});
    auto futSubA2b=std::async(std::launch::async,[&fedSubA2b](){fedSubA2b.enterExecutingMode();fedSubA2b.finalize();});
    auto futSubSub1=std::async(std::launch::async,[&fedSubSub1](){fedSubSub1.enterExecutingMode();fedSubSub1.finalize();});
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });



    fut.get();
    futSubSub1.get();
    futSubA2b.get();
    futSubB.get();
    futSubA2.get();
    futSubA.get();
    futA.get();

    EXPECT_EQ(conn1.madeConnections(), 6);
}


TEST(connector_tests2, multibroker_connector_regex2)
{
    helics::cleanupHelicsLibrary();
    using helics::apps::InterfaceDirection;
    /** setup brokers and cores*/
    helics::BrokerApp brokerA(helics::CoreType::TEST,"brokerA","-f7");

    helics::BrokerApp subBroker1(helics::CoreType::TEST,"subBroker1","--broker=brokerA");
    helics::BrokerApp subBroker2(helics::CoreType::TEST,"subBroker2","--broker=brokerA");

    helics::BrokerApp subsubBroker2(helics::CoreType::TEST,"subsubBroker2","--broker=subBroker2");

    helics::CoreApp coreA(helics::CoreType::TEST,"--name=coreA --broker=brokerA");
    helics::CoreApp core_sub1(helics::CoreType::TEST,"--name=core_sub1 --broker=subBroker1");
    helics::CoreApp core_sub1B(helics::CoreType::TEST,"--name=core_sub1B --broker=subBroker1");

    helics::CoreApp core_sub2(helics::CoreType::TEST,"--name=core_sub2 --broker=subBroker2");

    helics::CoreApp core_subsub1(helics::CoreType::TEST,"--name=core_subsub1 --broker=subsubBroker2");
    /** setup the federates*/
    helics::FederateInfo fedInfo;
    fedInfo.setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);

    auto fedA=helics::ValueFederate("fedA",coreA,fedInfo);
    fedA.registerGlobalInput<double>("inputA");
    fedA.registerGlobalPublication<double>("pubD");

    auto fedSubA=helics::ValueFederate("fedSubA",core_sub1,fedInfo);

    fedSubA.registerGlobalInput<double>("inputB");
    fedSubA.registerGlobalPublication<double>("pubF");

    auto fedSubA2=helics::ValueFederate("fedSubA2",core_sub1,fedInfo);
    fedSubA2.registerGlobalInput<double>("inputC");
    fedSubA2.registerGlobalPublication<double>("pubA");

    auto fedSubA2b=helics::ValueFederate("fedSubA2b",core_sub1B,fedInfo);

    fedSubA2b.registerGlobalInput<double>("inputD");
    fedSubA2b.registerGlobalPublication<double>("pubE");

    auto fedSubB=helics::ValueFederate("fedSubB",core_sub2,fedInfo);
    fedSubB.registerGlobalInput<double>("inputE");
    fedSubB.registerGlobalPublication<double>("pubB");
    auto fedSubSub1=helics::ValueFederate("fedSubSub1",core_subsub1,fedInfo);
    fedSubSub1.registerGlobalInput<double>("inputF");
    fedSubSub1.registerGlobalPublication<double>("pubC");

    helics::apps::Connector conn1("connector1",coreA, fedInfo);
    conn1.addConnection("REGEX:(?<letter>.)publication(?<value>.)", "REGEX:inp(?<value>.)(?<letter>.)", InterfaceDirection::FROM_TO);

    brokerA.addAlias("pubA","Apublication1");
    brokerA.addAlias("pubB","Bpublication2");
    brokerA.addAlias("pubC","Cpublication3");
    brokerA.addAlias("pubD","Dpublication4");
    brokerA.addAlias("pubE","Epublication5");
    brokerA.addAlias("pubF","Fpublication6");

    brokerA.addAlias("inputA","inp1A");
    brokerA.addAlias("inputB","inp2B");
    brokerA.addAlias("inputC","inp3C");
    brokerA.addAlias("inputD","inp4D");
    brokerA.addAlias("inputE","inp5E");
    brokerA.addAlias("inputF","inp6F");

    auto futA=std::async(std::launch::async,[&fedA](){fedA.enterExecutingMode();fedA.finalize();});
    auto futSubA=std::async(std::launch::async,[&fedSubA](){fedSubA.enterExecutingMode();fedSubA.finalize();});
    auto futSubA2=std::async(std::launch::async,[&fedSubA2](){fedSubA2.enterExecutingMode();fedSubA2.finalize();});
    auto futSubB=std::async(std::launch::async,[&fedSubB](){fedSubB.enterExecutingMode();fedSubB.finalize();});
    auto futSubA2b=std::async(std::launch::async,[&fedSubA2b](){fedSubA2b.enterExecutingMode();fedSubA2b.finalize();});
    auto futSubSub1=std::async(std::launch::async,[&fedSubSub1](){fedSubSub1.enterExecutingMode();fedSubSub1.finalize();});
    auto fut = std::async(std::launch::async, [&conn1]() { conn1.run(); });



    fut.get();
    futSubSub1.get();
    futSubA2b.get();
    futSubB.get();
    futSubA2.get();
    futSubA.get();
    futA.get();

    EXPECT_EQ(conn1.madeConnections(), 6);
}
