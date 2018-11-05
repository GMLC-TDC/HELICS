/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>

#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.hpp"
#include <cstdio>
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (recorder_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (simple_recorder_test)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    rec1.addSubscription ("pub1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    vfed.finalize ();
    fut.get ();
    rec1.finalize ();
    auto cnt = rec1.pointCount ();
    BOOST_CHECK_EQUAL (cnt, 2);
}

BOOST_AUTO_TEST_CASE (simple_recorder_test2)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    rec1.addSubscription ("pub1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    vfed.finalize ();
    fut.get ();
    rec1.finalize ();
    auto v1 = rec1.getValue (0);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.4));

    v1 = rec1.getValue (1);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (4.7));

    v1 = rec1.getValue (2);
    BOOST_CHECK_EQUAL (v1.first, std::string ());
    BOOST_CHECK_EQUAL (v1.second, std::string ());

    auto m2 = rec1.getMessage (4);
    BOOST_CHECK (!m2);
}

BOOST_AUTO_TEST_CASE (recorder_test_message)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    helics::MessageFederate mfed ("block1", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");

    rec1.addEndpoint ("src1");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed.enterExecutingMode ();

    auto retTime = mfed.requestTime (1.0);
    e1.send ("src1", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);
    retTime = mfed.requestTime (2.0);
    e1.send ("src1", "this is a test message2");
    BOOST_CHECK_EQUAL (retTime, 2.0);

    mfed.finalize ();
    fut.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

const std::vector<std::string> simple_files{"example1.recorder", "example2.record",    "example3rec.json",
                                            "example4rec.json",  "exampleCapture.txt", "exampleCapture.json"};

BOOST_DATA_TEST_CASE (simple_recorder_test_files, boost::unit_test::data::make (simple_files), file)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.loadFile (std::string (TEST_DIR) + "/test_files/" + file);

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2 (helics::GLOBAL, &vfed, "pub2", helics::helics_type_t::helicsDouble);

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (1.5);
    BOOST_CHECK_EQUAL (retTime, 1.5);
    pub2.publish (5.7);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);

    retTime = vfed.requestTime (3.0);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    pub2.publish ("3.9");

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    vfed.finalize ();
    fut.get ();
    rec1.finalize ();
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4);
    auto v1 = rec1.getValue (0);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.4));
    v1 = rec1.getValue (1);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (5.7));

    v1 = rec1.getValue (2);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (4.7));

    v1 = rec1.getValue (3);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.9));
}

const std::vector<std::string> simple_message_files{"example4.recorder", "example5.record", "example6rec.json"};

BOOST_DATA_TEST_CASE (simple_recorder_test_message_files,
                      boost::unit_test::data::make (simple_message_files),
                      file)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.loadFile (std::string (TEST_DIR) + "/test_files/" + file);

    helics::CombinationFederate cfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::helics_type_t::helicsDouble);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5); });
    cfed.enterExecutingMode ();
    auto retTime = cfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);
    e1.send ("src1", "this is a test message");

    retTime = cfed.requestTime (1.5);
    BOOST_CHECK_EQUAL (retTime, 1.5);
    pub2.publish (5.7);

    retTime = cfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    e1.send ("src1", "this is a test message2");
    pub1.publish (4.7);

    retTime = cfed.requestTime (3.0);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    pub2.publish ("3.9");

    retTime = cfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    cfed.finalize ();
    fut.get ();
    rec1.finalize ();
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4);
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);

    auto v1 = rec1.getValue (0);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.4));
    v1 = rec1.getValue (1);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (5.7));

    v1 = rec1.getValue (2);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (4.7));

    v1 = rec1.getValue (3);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.9));

    auto m = rec1.getMessage (1);
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message2");
}

BOOST_DATA_TEST_CASE (simple_recorder_test_message_files_cmd,
                      boost::unit_test::data::make (simple_message_files),
                      file)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (600));
    auto brk = helics::BrokerFactory::create (helics::core_type::IPC, "ipc_broker", "-f 2");
    brk->connect ();
    std::string exampleFile = std::string (TEST_DIR) + "/test_files/" + file;

    StringToCmdLine cmdArg ("--name=rec --broker=ipc_broker --core=ipc " + exampleFile);

    helics::apps::Recorder rec1 (cmdArg.getArgCount (), cmdArg.getArgV ());

    helics::FederateInfo fi (helics::core_type::IPC);
    fi.coreInitString = "1 --broker=ipc_broker";

    helics::CombinationFederate cfed ("obj", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::helics_type_t::helicsDouble);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::helics_type_t::helicsDouble);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5); });
    cfed.enterExecutingMode ();
    auto retTime = cfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);
    e1.send ("src1", "this is a test message");

    retTime = cfed.requestTime (1.5);
    BOOST_CHECK_EQUAL (retTime, 1.5);
    pub2.publish (5.7);

    retTime = cfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    e1.send ("src1", "this is a test message2");
    pub1.publish (4.7);

    retTime = cfed.requestTime (3.0);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    pub2.publish ("3.9");

    retTime = cfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    cfed.finalize ();
    fut.get ();
    rec1.finalize ();
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4);
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);

    auto v1 = rec1.getValue (0);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.4));
    v1 = rec1.getValue (1);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (5.7));

    v1 = rec1.getValue (2);
    BOOST_CHECK_EQUAL (v1.first, "pub1");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (4.7));

    v1 = rec1.getValue (3);
    BOOST_CHECK_EQUAL (v1.first, "pub2");
    BOOST_CHECK_EQUAL (v1.second, std::to_string (3.9));

    auto m = rec1.getMessage (1);
    BOOST_REQUIRE (m);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message2");
    std::this_thread::sleep_for (std::chrono::milliseconds (500));
}

BOOST_AUTO_TEST_CASE (recorder_test_destendpoint_clone)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addDestEndpointClone ("d2");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_GE (rec1.messageCount (), 2);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

BOOST_AUTO_TEST_CASE (recorder_test_srcendpoint_clone)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::MessageFederate mfed ("block1", fi);
    helics::MessageFederate mfed2 ("block2", fi);

    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addSourceEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d2");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_GE (rec1.messageCount (), 2);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

BOOST_AUTO_TEST_CASE (recorder_test_endpoint_clone)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "core3";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

const std::vector<std::string> simple_clone_test_files{"clone_example1.txt",  "clone_example2.txt",
                                                       "clone_example3.txt",  "clone_example4.txt",
                                                       "clone_example5.txt",  "clone_example6.txt",
                                                       "clone_example7.json", "clone_example8.JSON"};

BOOST_DATA_TEST_CASE (simple_clone_test_file, boost::unit_test::data::make (simple_clone_test_files), file)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core4";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.loadFile (std::string (TEST_DIR) + "/test_files/" + file);

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_GE (rec1.messageCount (), 2);

    auto m = rec1.getMessage (0);
    BOOST_CHECK (m);
    if (m)
    {
        BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
    }
}

BOOST_AUTO_TEST_CASE (recorder_test_saveFile1)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core5";
    fi.coreInitString = "3";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);

    auto filename = boost::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (boost::filesystem::exists (filename));

    auto filename2 = boost::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (boost::filesystem::exists (filename2));
    boost::filesystem::remove (filename);
    boost::filesystem::remove (filename2);
}

BOOST_AUTO_TEST_CASE (recorder_test_saveFile2)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core6";
    fi.coreInitString = "2";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.addSubscription ("pub1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::helics_type_t::helicsDouble);
    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    vfed.finalize ();
    fut.get ();
    rec1.finalize ();

    auto m2 = rec1.getMessage (4);
    BOOST_CHECK (!m2);
    auto filename = boost::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (boost::filesystem::exists (filename));

    auto filename2 = boost::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (boost::filesystem::exists (filename2));
    boost::filesystem::remove (filename);
    boost::filesystem::remove (filename2);
}

BOOST_AUTO_TEST_CASE (recorder_test_saveFile3)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "core7";
    fi.coreInitString = "3";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setTimeProperty (PERIOD_PROPERTY, 1.0);

    helics::CombinationFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");
    rec1.addSubscription ("pub1");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::helics_type_t::helicsDouble);

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();
    pub1.publish (3.4);

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    pub1.publish (4.7);
    BOOST_CHECK_EQUAL (retTime, 1.0);

    e2.send ("d1", "this is a test message2");

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);

    mfed2.requestTimeComplete ();
    pub1.publish (4.7);

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2);
    BOOST_CHECK_EQUAL (rec1.pointCount (), 3);

    auto filename = boost::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (boost::filesystem::exists (filename));

    auto filename2 = boost::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (boost::filesystem::exists (filename2));
    boost::filesystem::remove (filename);
    boost::filesystem::remove (filename2);
}

BOOST_AUTO_TEST_CASE (recorder_test_help)
{
    StringToCmdLine cmdArg ("--version --quiet");
    helics::apps::Recorder rec1 (cmdArg.getArgCount (), cmdArg.getArgV ());

    BOOST_CHECK (!rec1.isActive ());

    StringToCmdLine cmdArg2 ("-? --quiet");
    helics::apps::Recorder rec2 (cmdArg2.getArgCount (), cmdArg2.getArgV ());

    BOOST_CHECK (!rec2.isActive ());
}

BOOST_AUTO_TEST_SUITE_END ()
