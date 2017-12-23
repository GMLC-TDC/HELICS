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
#include "helics/player/player.h"
#include "helics/application_api/Subscriptions.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE (player_tests)


BOOST_AUTO_TEST_CASE(simple_player_test)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::player play1(fi);
    fi.name = "block1";
    play1.addPublication("pub1", helics::helicsType_t::helicsDouble);
    play1.addPoint(1.0, "pub1", 0.5);
    play1.addPoint(2.0, "pub1", 0.7);
    play1.addPoint(3.0, "pub1", 0.8);

    helics::ValueFederate vfed(fi);
    helics::Subscription sub1(&vfed, "pub1");
    auto fut = std::async(std::launch::async, [&play1]() {play1.run(); });
    vfed.enterExecutionState();
    auto retTime=vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    auto val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.5);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.7);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.8);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    vfed.finalize();
    fut.get();


}

BOOST_AUTO_TEST_CASE(simple_player_test2)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::player play1(fi);
    fi.name = "block1";
    play1.addPublication<double>("pub1");
    play1.addPoint(1.0, "pub1", 0.5);
    play1.addPoint(2.0, "pub1", 0.7);
    play1.addPoint(3.0, "pub1", 0.8);

    play1.addPublication<double>("pub2");
    play1.addPoint(1.0, "pub2", 0.4);
    play1.addPoint(2.0, "pub2", 0.6);
    play1.addPoint(3.0, "pub2", 0.9);

    helics::ValueFederate vfed(fi);
    helics::Subscription sub1(&vfed, "pub1");
    helics::Subscription sub2(&vfed, "pub2");
    auto fut = std::async(std::launch::async, [&play1]() {play1.run(); });
    vfed.enterExecutionState();
    

    auto retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    auto val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.5);
    val = sub2.getValue<double>();
    BOOST_CHECK_CLOSE(val, 0.4,0.000001);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.7);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.6);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.8);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.9);


    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    vfed.finalize();
    fut.get();

}

const std::vector<std::string> simple_files
{ "example1.player",  "example2.player", "example3.player","example4.player", "example5.json"};

BOOST_DATA_TEST_CASE(simple_player_test_files, boost::unit_test::data::make(simple_files), file)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::player play1(fi);
    fi.name = "block1";
    play1.loadFile(std::string(TEST_DIR) + "/test_files/" + file);

    helics::ValueFederate vfed(fi);
    helics::Subscription sub1(&vfed, "pub1");
    helics::Subscription sub2(&vfed, "pub2");
    auto fut = std::async(std::launch::async, [&play1]() {play1.run(); });
    vfed.enterExecutionState();
    auto val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.3);

    auto retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.5);
    val = sub2.getValue<double>();
    BOOST_CHECK_CLOSE(val, 0.4, 0.000001);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.7);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.6);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.8);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.9);


    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    vfed.finalize();
    fut.get();


}

BOOST_AUTO_TEST_CASE(simple_player_testjson)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::player play1(fi);
    fi.name = "block1";
    play1.loadFile(std::string(TEST_DIR) + "/test_files/example6.json");

    helics::ValueFederate vfed(fi);
    helics::Subscription sub1(&vfed, "pub1");
    helics::Subscription sub2(&vfed, "pub2");
    auto fut = std::async(std::launch::async, [&play1]() {play1.run(); });
    vfed.enterExecutionState();


    auto retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    auto val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.5);
    val = sub2.getValue<double>();
    BOOST_CHECK_CLOSE(val, 0.4, 0.000001);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.7);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.6);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.8);
    val = sub2.getValue<double>();
    BOOST_CHECK_EQUAL(val, 0.9);


    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    vfed.finalize();
    fut.get();

}


BOOST_AUTO_TEST_CASE(player_test_message)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::player play1(fi);
    fi.name = "block1";

    helics::MessageFederate mfed(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "dest");

    play1.addMessage(1.0, "src", "dest", "this is a message");
    auto fut = std::async(std::launch::async, [&play1]() {play1.run(); });
    mfed.enterExecutionState();


    auto retTime = mfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    auto mess = e1.getMessage();
    BOOST_CHECK(mess);
    if (mess)
    {
        BOOST_CHECK_EQUAL(mess->src, "src");
        BOOST_CHECK_EQUAL(mess->dest, "dest");
        BOOST_CHECK_EQUAL(mess->data.to_string(), "this is a message");
    }
    
    mfed.finalize();
    fut.get();

}
/*
BOOST_AUTO_TEST_CASE (simple_player_test)
{
    static exeTestRunner playerExe ("", "helics_player");

    static exeTestRunner brokerExe ("", "helics_broker");

    auto res = brokerExe.runAsync ("1 --type=ipc --name=ipc_broker");
    std::string exampleFile = std::string (TEST_DIR) + "/test_files/example1.player";
    auto res2 = playerExe.runCaptureOutputAsync ("--name=player --broker=ipc_broker --core=ipc " + exampleFile);

    auto val = res2.get ();
    auto val2 = res.get ();
    BOOST_CHECK_EQUAL (val2, 0);
    std::string compareString = "read file 24 points for 1 tags";
    BOOST_CHECK (val.compare (0, compareString.size (), compareString) == 0);
}
*/
BOOST_AUTO_TEST_SUITE_END ()