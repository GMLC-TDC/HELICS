/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/apps/Tracer.hpp"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE(tracer_tests)

BOOST_AUTO_TEST_CASE(simple_tracer_test)
{
    std::atomic<double> lastVal;
    std::atomic<double> lastTime;
    auto cb = [&lastVal,&lastTime](helics::Time tm, const std::string &, const std::string &newval) {
        lastTime = static_cast<double>(tm);
        lastVal = std::stod(newval);
    };
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Tracer trace1(fi);
    fi.name = "block1";
    trace1.addSubscription("pub1");
    trace1.setValueCallback(cb);
    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(4); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);

    retTime = vfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    BOOST_CHECK_CLOSE(lastTime.load(), 1.0,0.00000001);
    BOOST_CHECK_CLOSE(lastVal.load(), 3.4,0.000000001);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    BOOST_CHECK_CLOSE(lastTime.load(), 2.0,0.000000001);
    BOOST_CHECK_CLOSE(lastVal.load(), 4.7,0.000000001);
    vfed.finalize();
    fut.get();
    trace1.finalize();

}

BOOST_AUTO_TEST_CASE(simple_recorder_test2)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Tracer trace1(fi);
    fi.name = "block1";
    trace1.addSubscription("pub1");

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(4); });
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
    trace1.finalize();
   
}

BOOST_AUTO_TEST_CASE(recorder_test_message)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::Tracer trace1(fi);
    fi.name = "block1";

    helics::MessageFederate mfed(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");

    //trace1.addEndpoint("src1");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5.0); });
    mfed.enterExecutionState();

    auto retTime = mfed.requestTime(1.0);
    e1.send("src1", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);
    retTime = mfed.requestTime(2.0);
    e1.send("src1", "this is a test message2");
    BOOST_CHECK_EQUAL(retTime, 2.0);

    mfed.finalize();
    fut.get();
}

const std::vector<std::string> simple_files{ "example1.recorder", "example2.record",    "example3rec.json",
"example4rec.json",  "exampleCapture.txt", "exampleCapture.json" };

BOOST_DATA_TEST_CASE(simple_recorder_test_files, boost::unit_test::data::make(simple_files), file)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Tracer trace1(fi);

    trace1.loadFile(std::string(TEST_DIR) + "/test_files/" + file);
    fi.name = "block1";

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &vfed, "pub2", helics::helics_type_t::helicsDouble);

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(4); });
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
    trace1.finalize();
    
}

const std::vector<std::string> simple_message_files{ "example4.recorder", "example5.record", "example6rec.json" };

BOOST_DATA_TEST_CASE(simple_recorder_test_message_files,
    boost::unit_test::data::make(simple_message_files),
    file)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::Tracer trace1(fi);

    trace1.loadFile(std::string(TEST_DIR) + "/test_files/" + file);
    fi.name = "block1";

    helics::CombinationFederate cfed(fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::helics_type_t::helicsDouble);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5); });
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
    trace1.finalize();
   
}

BOOST_DATA_TEST_CASE(simple_recorder_test_message_files_cmd,
    boost::unit_test::data::make(simple_message_files),
    file)
{
    auto brk = helics::BrokerFactory::create(helics::core_type::IPC, "ipc_broker", "2");
    brk->connect();
    std::string exampleFile = std::string(TEST_DIR) + "/test_files/" + file;

    StringToCmdLine cmdArg("--name=rec --broker=ipc_broker --core=ipc " + exampleFile);

    helics::Tracer trace1(cmdArg.getArgCount(), cmdArg.getArgV());

    helics::FederateInfo fi("obj");
    fi.coreType = helics::core_type::IPC;
    fi.coreInitString = "1 --broker=ipc_broker";

    helics::CombinationFederate cfed(fi);
    helics::Publication pub1(helics::GLOBAL, &cfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2(helics::GLOBAL, &cfed, "pub2", helics::helics_type_t::helicsDouble);
    helics::Endpoint e1(helics::GLOBAL, &cfed, "d1");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5); });
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
    trace1.finalize();
}

BOOST_AUTO_TEST_CASE(recorder_test_destendpoint_clone)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "3";
    helics::Tracer trace1(fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::MessageFederate mfed(fi);

    fi.name = "block2";

    helics::MessageFederate mfed2(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    trace1.addDestEndpointClone("d1");
    trace1.addDestEndpointClone("d2");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5.0); });
    mfed2.enterExecutionStateAsync();
    mfed.enterExecutionState();
    mfed2.enterExecutionStateComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();

}

BOOST_AUTO_TEST_CASE(recorder_test_srcendpoint_clone)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "3";
    helics::Tracer trace1(fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::MessageFederate mfed(fi);

    fi.name = "block2";

    helics::MessageFederate mfed2(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    trace1.addSourceEndpointClone("d1");
    trace1.addSourceEndpointClone("d2");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5.0); });
    mfed2.enterExecutionStateAsync();
    mfed.enterExecutionState();
    mfed2.enterExecutionStateComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
}

BOOST_AUTO_TEST_CASE(recorder_test_endpoint_clone)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core3";
    fi.coreInitString = "3";
    helics::Tracer trace1(fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::MessageFederate mfed(fi);

    fi.name = "block2";

    helics::MessageFederate mfed2(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    trace1.addDestEndpointClone("d1");
    trace1.addSourceEndpointClone("d1");

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5.0); });
    mfed2.enterExecutionStateAsync();
    mfed.enterExecutionState();
    mfed2.enterExecutionStateComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
}

const std::vector<std::string> simple_clone_test_files{ "clone_example1.txt",  "clone_example2.txt",
"clone_example3.txt",  "clone_example4.txt",
"clone_example5.txt",  "clone_example6.txt",
"clone_example7.json", "clone_example8.JSON" };

BOOST_DATA_TEST_CASE(simple_clone_test_file, boost::unit_test::data::make(simple_clone_test_files), file)
{
    helics::FederateInfo fi("trace1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core4";
    fi.coreInitString = "3";
    helics::Tracer trace1(fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::MessageFederate mfed(fi);

    fi.name = "block2";

    helics::MessageFederate mfed2(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    trace1.loadFile(std::string(TEST_DIR) + "/test_files/" + file);

    auto fut = std::async(std::launch::async, [&trace1]() { trace1.run(5.0); });
    mfed2.enterExecutionStateAsync();
    mfed.enterExecutionState();
    mfed2.enterExecutionStateComplete();

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();

    e1.send("d2", "this is a test message");
    BOOST_CHECK_EQUAL(retTime, 1.0);

    e2.send("d1", "this is a test message2");

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    mfed2.requestTimeComplete();

    mfed.finalize();
    mfed2.finalize();
    fut.get();
}

BOOST_AUTO_TEST_SUITE_END()
