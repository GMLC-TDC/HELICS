/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/special_federates/recorder.h"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.h"
#include "helics/common/stringToCmdLine.h"
#include <future>

BOOST_AUTO_TEST_SUITE (recorder_tests)



BOOST_AUTO_TEST_CASE(simple_recorder_test)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);
    fi.name = "block1";
    rec1.addSubscription("pub1");

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL,&vfed, "pub1",helics::helicsType_t::helicsDouble);
    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(4); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);
    

    retTime = vfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    
    vfed.finalize();
    fut.get();
    rec1.finalize();
    auto cnt = rec1.pointCount();
    BOOST_CHECK_EQUAL(cnt, 2);



}


BOOST_AUTO_TEST_CASE(simple_recorder_test2)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);
    fi.name = "block1";
    rec1.addSubscription("pub1");

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::helicsType_t::helicsDouble);
    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(4); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);


    retTime = vfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);

    vfed.finalize();
    fut.get();
    rec1.finalize();
    auto v1 = rec1.getValue(0);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.4));

    v1 = rec1.getValue(1);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(4.7));

    v1 = rec1.getValue(2);
    BOOST_CHECK_EQUAL(v1.first, std::string());
    BOOST_CHECK_EQUAL(v1.second, std::string());

    auto m2 = rec1.getMessage(4);
    BOOST_CHECK(!m2);

}


BOOST_AUTO_TEST_CASE(recorder_test_message)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);
    fi.name = "block1";

    helics::MessageFederate mfed(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");

    rec1.addEndpoint("src1");

    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(5.0); });
    mfed.enterExecutionState();


    auto retTime = mfed.requestTime(1.0);
    e1.send("src1", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);
    retTime = mfed.requestTime(2.0);
    e1.send("src1", "this is a test message2");
    BOOST_CHECK_EQUAL(retTime, 2.0);

    mfed.finalize();
    fut.get();
    BOOST_CHECK_EQUAL(rec1.messageCount(), 2);

    auto m = rec1.getMessage(0);
    BOOST_CHECK_EQUAL(m->data.to_string(), "this is a test message");
}

const std::vector<std::string> simple_files
{ "example1.recorder",  "example2.record" , "example3rec.json" };

BOOST_DATA_TEST_CASE(simple_recorder_test_files, boost::unit_test::data::make(simple_files), file)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);

    rec1.loadFile(std::string(TEST_DIR) + "/test_files/"+file);
    fi.name = "block1";

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::helicsType_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &vfed, "pub2", helics::helicsType_t::helicsDouble);

    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(4); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(1.5);
    BOOST_CHECK_EQUAL(retTime, 1.5);
    pub2.publish(5.7);

    retTime = vfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(3.0);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    pub2.publish("3.9");

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);

    vfed.finalize();
    fut.get();
    rec1.finalize();
    BOOST_CHECK_EQUAL(rec1.pointCount(), 4);
    auto v1 = rec1.getValue(0);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.4));
    v1 = rec1.getValue(1);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(5.7));

    v1 = rec1.getValue(2);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(4.7));

    v1 = rec1.getValue(3);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.9));

    

}

const std::vector<std::string> simple_message_files
{ "example4.recorder" ,  "example5.record" , "example6rec.json" };

BOOST_DATA_TEST_CASE(simple_recorder_test_message_files, boost::unit_test::data::make(simple_message_files), file)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);

    rec1.loadFile(std::string(TEST_DIR) + "/test_files/" + file);
    fi.name = "block1";

    helics::CombinationFederate cfed(fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::helicsType_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::helicsType_t::helicsDouble);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(5); });
    cfed.enterExecutionState();
    auto retTime = cfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);
    e1.send("src1", "this is a test message");

    retTime = cfed.requestTime(1.5);
    BOOST_CHECK_EQUAL(retTime, 1.5);
    pub2.publish(5.7);

    retTime = cfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    e1.send("src1", "this is a test message2");
    pub1.publish(4.7);
    
    
    retTime = cfed.requestTime(3.0);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    pub2.publish("3.9");

    retTime = cfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);

    cfed.finalize();
    fut.get();
    rec1.finalize();
    BOOST_CHECK_EQUAL(rec1.pointCount(), 4);
    BOOST_CHECK_EQUAL(rec1.messageCount(), 2);

    auto v1 = rec1.getValue(0);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.4));
    v1 = rec1.getValue(1);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(5.7));

    v1 = rec1.getValue(2);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(4.7));

    v1 = rec1.getValue(3);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.9));

    auto m = rec1.getMessage(1);
    BOOST_CHECK_EQUAL(m->data.to_string(), "this is a test message2");
}

BOOST_DATA_TEST_CASE(simple_recorder_test_message_files_cmd, boost::unit_test::data::make(simple_message_files), file)
{

    auto brk = helics::BrokerFactory::create(helics::core_type::IPC, "ipc_broker", "2");
    brk->connect();
    std::string exampleFile = std::string(TEST_DIR) + "/test_files/" + file;

    stringToCmdLine cmdArg("--name=rec --broker=ipc_broker --core=ipc " + exampleFile);


    helics::recorder rec1(cmdArg.getArgCount(), cmdArg.getArgV());

    helics::FederateInfo fi("obj");
    fi.coreType = helics::core_type::IPC;
    fi.coreInitString = "1 --broker=ipc_broker";


    helics::CombinationFederate cfed(fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::helicsType_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::helicsType_t::helicsDouble);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(5); });
    cfed.enterExecutionState();
    auto retTime = cfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);
    e1.send("src1", "this is a test message");

    retTime = cfed.requestTime(1.5);
    BOOST_CHECK_EQUAL(retTime, 1.5);
    pub2.publish(5.7);

    retTime = cfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    e1.send("src1", "this is a test message2");
    pub1.publish(4.7);


    retTime = cfed.requestTime(3.0);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    pub2.publish("3.9");

    retTime = cfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);

    cfed.finalize();
    fut.get();
    rec1.finalize();
    BOOST_CHECK_EQUAL(rec1.pointCount(), 4);
    BOOST_CHECK_EQUAL(rec1.messageCount(), 2);

    auto v1 = rec1.getValue(0);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.4));
    v1 = rec1.getValue(1);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(5.7));

    v1 = rec1.getValue(2);
    BOOST_CHECK_EQUAL(v1.first, "pub1");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(4.7));

    v1 = rec1.getValue(3);
    BOOST_CHECK_EQUAL(v1.first, "pub2");
    BOOST_CHECK_EQUAL(v1.second, std::to_string(3.9));

    auto m = rec1.getMessage(1);
    BOOST_CHECK_EQUAL(m->data.to_string(), "this is a test message2");
}
BOOST_AUTO_TEST_SUITE_END ()
