/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#include "helics/external/filesystem.hpp"
#pragma warning(pop)
#else
#include "helics/external/filesystem.hpp"
#endif

#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/apps/Clone.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/core/BrokerFactory.hpp"
#include <cstdio>
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (clone_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (simple_clone_test_pub)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore1";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1 ("c1", fi);
    c1.setFederateToClone ("block1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async (std::launch::async, [&c1] () { c1.runTo (4); });
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
    c1.finalize ();
    auto cnt = c1.pointCount ();
    BOOST_CHECK_EQUAL (cnt, 2u);
}

/*
BOOST_AUTO_TEST_CASE (simple_clone_test2)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore1-t2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    rec1.addSubscription ("pub1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (4); });
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

BOOST_AUTO_TEST_CASE (clone_test_message)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore-tm";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    helics::MessageFederate mfed ("block1", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");

    rec1.addEndpoint ("src1");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5.0); });
    mfed.enterExecutingMode ();

    auto retTime = mfed.requestTime (1.0);
    e1.send ("src1", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);
    retTime = mfed.requestTime (2.0);
    e1.send ("src1", "this is a test message2");
    BOOST_CHECK_EQUAL (retTime, 2.0);

    mfed.finalize ();
    fut.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

static constexpr const char *simple_files[] = {"example1.recorder", "example2.record",    "example3rec.json",
                                               "example4rec.json",  "exampleCapture.txt", "exampleCapture.json"};

BOOST_DATA_TEST_CASE (simple_clone_test_files, boost::unit_test::data::make (simple_files), file)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = std::string ("coref") + file;
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.loadFile (std::string (TEST_DIR) + file);

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &vfed, "pub2", helics::data_type::helics_double);

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (4); });
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
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4u);
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

static constexpr const char *simple_message_files[] = {"example4.recorder", "example5.record", "example6rec.json"};

BOOST_DATA_TEST_CASE (simple_clone_test_message_files, boost::unit_test::data::make (simple_message_files), file)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = std::string ("rcoretmf") + file;
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.loadFile (std::string (TEST_DIR) + file);

    helics::CombinationFederate cfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5); });
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
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4u);
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);

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

#ifdef ENABLE_IPC_CORE
BOOST_DATA_TEST_CASE (simple_clone_test_message_files_cmd,
                      boost::unit_test::data::make (simple_message_files),
                      file)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (600));
    auto brk = helics::BrokerFactory::create (helics::core_type::IPC, "ipc_broker", "-f 2");
    brk->connect ();
    std::string exampleFile = std::string (TEST_DIR) + file;

    std::vector<std::string> args{"", "--name=rec", "--broker=ipc_broker", "--coretype=ipc", exampleFile};
    char *argv[5];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);
    argv[4] = &(args[4][0]);

    helics::apps::Recorder rec1 (5, argv);

    helics::FederateInfo fi (helics::core_type::IPC);
    fi.coreInitString = "-f 1 --broker=ipc_broker";

    helics::CombinationFederate cfed ("obj", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5); });
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
    BOOST_CHECK_EQUAL (rec1.pointCount (), 4u);
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);

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
#endif

BOOST_AUTO_TEST_CASE (recorder_test_srcendpoint_clone)
{
    helics::FederateInfo fi;
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "rcore2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);
    helics::MessageFederate mfed2 ("block2", fi);

    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addSourceEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d2");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5.0); });
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
    BOOST_CHECK_GE (rec1.messageCount (), 2u);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

BOOST_AUTO_TEST_CASE (recorder_test_endpoint_clone)
{
    helics::FederateInfo fi (helics::core_type::TEST);

    fi.coreName = "rcore3";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5.0); });
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
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);

    auto m = rec1.getMessage (0);
    BOOST_CHECK_EQUAL (m->data.to_string (), "this is a test message");
}

BOOST_AUTO_TEST_CASE (clone_test_saveFile1)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore5";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5.0); });
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
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);

    auto filename = ghc::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename));

    auto filename2 = ghc::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename2));
    ghc::filesystem::remove (filename);
    ghc::filesystem::remove (filename2);
}

BOOST_AUTO_TEST_CASE (recorder_test_saveFile2)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore6";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);

    rec1.addSubscription ("pub1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (4); });
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
    auto filename = ghc::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename));

    auto filename2 = ghc::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename2));
    ghc::filesystem::remove (filename);
    ghc::filesystem::remove (filename2);
}

BOOST_AUTO_TEST_CASE (clone_test_saveFile3)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "rcore7";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1);

    helics::CombinationFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");
    rec1.addSubscription ("pub1");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async (std::launch::async, [&rec1] () { rec1.runTo (5.0); });
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
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (rec1.pointCount (), 3u);

    auto filename = ghc::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename));

    auto filename2 = ghc::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename2));
    ghc::filesystem::remove (filename);
    ghc::filesystem::remove (filename2);
}

*/

BOOST_AUTO_TEST_CASE (recorder_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Clone c1 (args);

    BOOST_CHECK (!c1.isActive ());

    args.emplace_back ("-?");
    helics::apps::Clone c2 (args);

    BOOST_CHECK (!c2.isActive ());
}

BOOST_AUTO_TEST_SUITE_END ()
