/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <cstdio>

#include "../ThirdParty/libguarded/guarded.hpp"
#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/apps/Tracer.hpp"
#include "helics/common/stringOps.h"
#include "helics/core/BrokerFactory.hpp"
#include <algorithm>
#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (tracer_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (simple_tracer_test)
{
    std::atomic<double> lastVal{-1e49};
    std::atomic<double> lastTime{0.0};
    auto cb = [&lastVal, &lastTime] (helics::Time tm, const std::string &, const std::string &newval) {
        lastTime = static_cast<double> (tm);
        lastVal = std::stod (newval);
    };
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "tcore-simple-tracer";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);

    trace1.addSubscription ("pub1");
    trace1.setValueCallback (cb);
    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    BOOST_CHECK_CLOSE (lastVal.load (), 3.4, 0.000000001);
    pub1.publish (4.7);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    vfed.finalize ();
    fut.get ();
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.000000001);
    BOOST_CHECK_CLOSE (lastVal.load (), 4.7, 0.000000001);
    trace1.finalize ();
}

BOOST_AUTO_TEST_CASE (tracer_test_message)
{
    libguarded::guarded<std::unique_ptr<helics::Message>> mguard;
    std::atomic<double> lastTime{0.0};
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "tcore-tracer";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);

    auto cb = [&mguard, &lastTime] (helics::Time tm, const std::string &, std::unique_ptr<helics::Message> mess) {
        mguard = std::move (mess);
        lastTime = static_cast<double> (tm);
    };

    helics::MessageFederate mfed ("block1", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");

    trace1.addEndpoint ("src1");
    trace1.setEndpointMessageCallback (cb);
    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5.0); });
    mfed.enterExecutingMode ();

    auto retTime = mfed.requestTime (1.0);
    e1.send ("src1", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);
    retTime = mfed.requestTime (2.0);
    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->dest, "src1");
    }
    e1.send ("src1", "this is a test message2");
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed.finalize ();
    cnt = 0;
    while (lastTime < 1.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message2");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->dest, "src1");
    }

    fut.get ();
}

static constexpr const char *simple_files[] = {"example1.recorder", "example2.record",    "example3rec.json",
                                               "example4rec.json",  "exampleCapture.txt", "exampleCapture.json"};

BOOST_DATA_TEST_CASE (simple_tracer_test_files, boost::unit_test::data::make (simple_files), file)
{
    static char indx = 'a';
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = std::string ("tcore1") + file;
    fi.coreName.push_back (indx++);
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);

    std::atomic<int> counter{0};
    auto cb = [&counter] (helics::Time, const std::string &, const std::string &) { ++counter; };
    trace1.setValueCallback (cb);
    trace1.loadFile (std::string (TEST_DIR) + file);

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &vfed, "pub2", helics::data_type::helics_double);

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (4); });
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
    BOOST_CHECK_EQUAL (counter.load (), 4);
    trace1.finalize ();
}

static constexpr const char *simple_message_files[] = {"example4.recorder", "example5.record", "example6rec.json"};

BOOST_DATA_TEST_CASE (simple_tracer_test_message_files, boost::unit_test::data::make (simple_message_files), file)
{
    static char indx = 'a';
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = std::string ("tcore1b") + file;
    fi.coreName.push_back (indx++);
    fi.coreInitString = " -f 2 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);

    trace1.loadFile (std::string (TEST_DIR) + file);

    std::atomic<int> counter{0};
    auto cb = [&counter] (helics::Time, const std::string &, const std::string &) { ++counter; };
    trace1.setValueCallback (cb);

    std::atomic<int> mcounter{0};
    auto cbm = [&mcounter] (helics::Time, const std::string &, std::unique_ptr<helics::Message>) { ++mcounter; };
    trace1.setEndpointMessageCallback (cbm);

    helics::CombinationFederate cfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5); });
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
    BOOST_CHECK_EQUAL (counter.load (), 4);
    BOOST_CHECK_EQUAL (mcounter.load (), 2);
    trace1.finalize ();
}

BOOST_DATA_TEST_CASE (simple_tracer_test_message_files_cmd,
                      boost::unit_test::data::make (simple_message_files),
                      file)
{
    std::this_thread::sleep_for (300ms);
    auto brk = helics::BrokerFactory::create (helics::core_type::IPC, "ipc_broker", "-f 2");
    brk->connect ();
    std::string exampleFile = std::string (TEST_DIR) + file;
    std::vector<std::string> args{"", "--name=rec", "--coretype=ipc", exampleFile};

    char *argv[4];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);

    helics::apps::Tracer trace1 (4, argv);
    std::atomic<int> counter{0};
    auto cb = [&counter] (helics::Time, const std::string &, const std::string &) { ++counter; };
    trace1.setValueCallback (cb);

    helics::FederateInfo fi;
    fi.coreType = helics::core_type::IPC;
    fi.coreInitString = "";

    helics::CombinationFederate cfed ("obj", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint e1 (helics::GLOBAL, &cfed, "d1");

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5); });
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
    BOOST_CHECK_EQUAL (counter.load (), 4);
    trace1.finalize ();
    std::this_thread::sleep_for (300ms);
}

BOOST_AUTO_TEST_CASE (tracer_test_destendpoint_clone)
{
    libguarded::guarded<std::unique_ptr<helics::Message>> mguard;
    std::atomic<double> lastTime{0.0};
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "tcore-dep";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    auto cb = [&mguard, &lastTime] (helics::Time tm, std::unique_ptr<helics::Message> mess) {
        mguard = std::move (mess);
        lastTime = static_cast<double> (tm);
    };
    trace1.setClonedMessageCallback (cb);
    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    trace1.addDestEndpointClone ("d1");
    trace1.addDestEndpointClone ("d2");

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();

    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d2");
    }
    e2.send ("d1", "this is a test message2");
    mfed.finalize ();
    mfed2.finalize ();
    cnt = 0;
    while (lastTime < 1.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message2");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d2");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d1");
    }
    fut.get ();
}

BOOST_AUTO_TEST_CASE (tracer_test_srcendpoint_clone)
{
    libguarded::guarded<std::unique_ptr<helics::Message>> mguard;
    std::atomic<double> lastTime{0.0};
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "tcoresrc";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);
    auto cb = [&mguard, &lastTime] (helics::Time tm, std::unique_ptr<helics::Message> mess) {
        mguard = std::move (mess);
        lastTime = static_cast<double> (tm);
    };
    trace1.setClonedMessageCallback (cb);

    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    trace1.addSourceEndpointClone ("d1");
    trace1.addSourceEndpointClone ("d2");

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();
    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d2");
    }
    e2.send ("d1", "this is a test message2");

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message2");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d2");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d1");
    }
}

BOOST_AUTO_TEST_CASE (tracer_test_endpoint_clone)
{
    libguarded::guarded<std::unique_ptr<helics::Message>> mguard;
    std::atomic<double> lastTime{0.0};
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "tcore-ep";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);

    auto cb = [&mguard, &lastTime] (helics::Time tm, std::unique_ptr<helics::Message> mess) {
        mguard = std::move (mess);
        lastTime = static_cast<double> (tm);
    };
    trace1.setClonedMessageCallback (cb);

    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint &e1 = mfed.registerGlobalEndpoint ("d1");
    helics::Endpoint &e2 = mfed2.registerGlobalEndpoint ("d2");

    trace1.addDestEndpointClone ("d1");
    trace1.addSourceEndpointClone ("d1");

    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();
    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d2");
    }
    e2.send ("d1", "this is a test message2");

    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message2");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d2");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d1");
    }
}

static constexpr const char *simple_clone_test_files[] = {"clone_example1.txt",  "clone_example2.txt",
                                                          "clone_example3.txt",  "clone_example4.txt",
                                                          "clone_example5.txt",  "clone_example6.txt",
                                                          "clone_example7.json", "clone_example8.JSON"};

BOOST_DATA_TEST_CASE (simple_clone_test_file, boost::unit_test::data::make (simple_clone_test_files), file)
{
    static char indx = 'a';
    libguarded::guarded<std::unique_ptr<helics::Message>> mguard;
    std::atomic<double> lastTime{0.0};
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = std::string ("tcore4") + file;
    fi.coreName.push_back (indx++);
    fi.coreInitString = "-f3 --autobroker";
    helics::apps::Tracer trace1 ("trace1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::MessageFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint &e1 = mfed.registerGlobalEndpoint ("d1");
    helics::Endpoint &e2 = mfed2.registerGlobalEndpoint ("d2");

    trace1.loadFile (std::string (TEST_DIR) + file);
    auto cb = [&mguard, &lastTime] (helics::Time tm, std::unique_ptr<helics::Message> mess) {
        mguard = std::move (mess);
        lastTime = static_cast<double> (tm);
    };
    trace1.setClonedMessageCallback (cb);
    auto fut = std::async (std::launch::async, [&trace1] () { trace1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();

    e1.send ("d2", "this is a test message");
    BOOST_CHECK_EQUAL (retTime, 1.0);

    mfed2.requestTimeAsync (2.0);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    mfed2.requestTimeComplete ();
    int cnt = 0;
    while (lastTime < 0.5)
    {
        std::this_thread::sleep_for (100ms);
        if (cnt++ > 10)
        {
            break;
        }
    }
    BOOST_CHECK_CLOSE (lastTime.load (), 1.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d1");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d2");
    }

    e2.send ("d1", "this is a test message2");
    mfed.finalize ();
    mfed2.finalize ();
    fut.get ();
    BOOST_CHECK_CLOSE (lastTime.load (), 2.0, 0.00000001);
    {
        auto mhandle = mguard.lock ();
        BOOST_REQUIRE (*mhandle);
        BOOST_CHECK_EQUAL ((*mhandle)->data.to_string (), "this is a test message2");
        BOOST_CHECK_EQUAL ((*mhandle)->source, "d2");
        BOOST_CHECK_EQUAL ((*mhandle)->original_dest, "d1");
    }
}

#ifndef DISABLE_SYSTEM_CALL_TESTS
BOOST_DATA_TEST_CASE (simple_tracer_test_message_files_exe,
                      boost::unit_test::data::make (simple_message_files),
                      file)
{
    std::this_thread::sleep_for (300ms);
    auto brk = helics::BrokerFactory::create (helics::core_type::IPC, "ipc_broker", "-f 2");
    brk->connect ();
    std::string exampleFile = std::string (TEST_DIR) + file;

    std::string cmdArg ("--name=tracer --coretype=ipc --stop=5 " + exampleFile);
    exeTestRunner tracerExe (HELICS_INSTALL_LOC, HELICS_BUILD_LOC "apps/", "helics_app");
    BOOST_REQUIRE (tracerExe.isActive ());
    auto out = tracerExe.runCaptureOutput (std::string ("tracer " + cmdArg));
    std::cout << out << std::endl;
    helics::FederateInfo fi (helics::core_type::IPC);
    fi.coreInitString = "";

    helics::CombinationFederate cfed ("obj", fi);
    helics::Publication pub1 (helics::GLOBAL, &cfed, "pub1", helics::data_type::helics_double);
    helics::Publication pub2 (helics::GLOBAL, &cfed, "pub2", helics::data_type::helics_double);
    helics::Endpoint &e1 = cfed.registerGlobalEndpoint ("d1");

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
    // std::string outAct = out.get ();
    auto outAct = out;
    int mcount = 0;
    int valcount = 0;
    auto vec = stringOps::splitline (outAct, "\n\r", stringOps::delimiter_compression::on);
    auto cnt = std::count_if (vec.begin (), vec.end (),
                              [] (const std::string &str) { return (!(str.empty ()) && (str[0] == '[')); });
    // 6 messages, 1 return line, 1 empty line
    BOOST_CHECK_EQUAL (cnt, 6);
    BOOST_CHECK_EQUAL (*(vec.end () - 2), "execution returned 0");
    for (const auto &line : vec)
    {
        if (line.find ("]value") != std::string::npos)
        {
            ++valcount;
        }
        if (line.find ("]message") != std::string::npos)
        {
            ++mcount;
        }
    }
    BOOST_CHECK_EQUAL (mcount, 2);
    BOOST_CHECK_EQUAL (valcount, 4);
}

#endif

BOOST_AUTO_TEST_SUITE_END ()
