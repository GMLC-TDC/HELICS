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
#include "helics/apps/Player.hpp"
#include "helics/core/BrokerFactory.hpp"
#include <cstdio>
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (clone_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (simple_clone_test_pub)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "clone_core1";
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

BOOST_AUTO_TEST_CASE (simple_clone_test_pub2)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "clone_core2";
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1 ("c1", fi);

    c1.setFederateToClone ("block1");

    helics::ValueFederate vfed ("block1", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);

    auto &pub2 = vfed.registerPublication ("pub2", "double", "m");

    auto fut = std::async (std::launch::async, [&c1] () { c1.runTo (4); });
    vfed.enterExecutingMode ();
    auto retTime = vfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);
    pub2.publish (3.3);
    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);

    vfed.finalize ();
    fut.get ();
    c1.finalize ();
    auto cnt = c1.pointCount ();
    BOOST_CHECK_EQUAL (cnt, 3u);
    auto icnt = c1.accessUnderlyingFederate ().getInputCount ();
    BOOST_CHECK_EQUAL (icnt, 2);
    c1.saveFile ("pubtest2.json");

    BOOST_CHECK (ghc::filesystem::exists ("pubtest2.json"));
}

BOOST_AUTO_TEST_CASE (simple_clone_test_pub2_file,
                      *boost::unit_test::depends_on ("clone_tests/simple_clone_test_pub2"))
{
    auto fi = helics::loadFederateInfo ("pubtest2.json");
    fi.coreName = "clone_core3";
    fi.coreInitString = "--autobroker";
    fi.coreType = helics::core_type::TEST;
    helics::apps::Player c1 ("c1", fi);
    c1.loadFile ("pubtest2.json");

    c1.initialize ();

    BOOST_CHECK_EQUAL (c1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (c1.publicationCount (), 2u);
    c1.finalize ();
    ghc::filesystem::remove ("pubtest2.json");
}

BOOST_AUTO_TEST_CASE (simple_clone_test_message)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "clone_core4";
    fi.setProperty (helics_property_time_period, 1.0);
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1 ("c1", fi);
    c1.setFederateToClone ("block1");

    helics::MessageFederate mfed ("block1", fi);
    auto &ept = mfed.registerGlobalEndpoint ("ept1", "etype");
    auto &ept2 = mfed.registerGlobalEndpoint ("ept3");
    mfed.registerEndpoint ("e3");

    auto fut = std::async (std::launch::async, [&c1] () { c1.runTo (4); });
    mfed.enterExecutingMode ();
    auto retTime = mfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    ept.send ("ept3", "message");

    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    ept2.send ("ept1", "reply");

    retTime = mfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);

    mfed.finalize ();
    fut.get ();
    c1.finalize ();
    auto cnt = c1.messageCount ();
    BOOST_CHECK_EQUAL (cnt, 2u);
    c1.saveFile ("eptsave.json");
}

BOOST_AUTO_TEST_CASE (simple_clone_test_message_file,
                      *boost::unit_test::depends_on ("clone_tests/simple_clone_test_message"))
{
    auto fi = helics::loadFederateInfo ("eptsave.json");
    fi.coreName = "clone_core5";
    fi.coreInitString = "--autobroker";
    fi.coreType = helics::core_type::TEST;
    helics::apps::Player c1 ("c1", fi);
    c1.loadFile ("eptsave.json");

    c1.initialize ();

    BOOST_CHECK_EQUAL (c1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (c1.endpointCount (), 3u);
    c1.finalize ();
    ghc::filesystem::remove ("eptsave.json");
}

BOOST_AUTO_TEST_CASE (simple_clone_test_combo)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "clone_core6";
    fi.setProperty (helics_property_time_period, 1.0);
    fi.coreInitString = "-f 2 --autobroker";
    helics::apps::Clone c1 ("c1", fi);
    c1.setFederateToClone ("block1");

    helics::CombinationFederate mfed ("block1", fi);
    auto &ept = mfed.registerGlobalEndpoint ("ept1", "etype");
    auto &ept2 = mfed.registerGlobalEndpoint ("ept3");
    mfed.registerEndpoint ("e3");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto &pub2 = mfed.registerPublication ("pub2", "double", "m");

    auto fut = std::async (std::launch::async, [&c1] () { c1.runTo (4); });
    mfed.enterExecutingMode ();
    auto retTime = mfed.requestTime (1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    ept.send ("ept3", "message");
    pub1.publish (3.4);
    retTime = mfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    ept2.send ("ept1", "reply");
    pub1.publish (4.7);
    pub2.publish (3.3);
    retTime = mfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);

    mfed.finalize ();
    fut.get ();
    c1.finalize ();
    auto cnt = c1.messageCount ();
    BOOST_CHECK_EQUAL (cnt, 2u);

    cnt = c1.pointCount ();
    BOOST_CHECK_EQUAL (cnt, 3u);

    c1.saveFile ("combsave.json");
}

BOOST_AUTO_TEST_CASE (simple_clone_test_combo_file,
                      *boost::unit_test::depends_on ("clone_tests/simple_clone_test_combo"))
{
    auto fi = helics::loadFederateInfo ("combsave.json");
    fi.coreName = "clone_core7";
    fi.coreInitString = "--autobroker";
    fi.coreType = helics::core_type::TEST;
    helics::apps::Player c1 ("c1", fi);
    c1.loadFile ("combsave.json");

    c1.initialize ();

    BOOST_CHECK_EQUAL (c1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (c1.endpointCount (), 3u);
    BOOST_CHECK_EQUAL (c1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (c1.publicationCount (), 2u);
    c1.finalize ();
    ghc::filesystem::remove ("combsave.json");
}

BOOST_AUTO_TEST_CASE (simple_clone_test_sub)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "clone_core8";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Clone c1 ("c1", fi);

    c1.setFederateToClone ("block1");

    helics::ValueFederate vfed ("block1", fi);
    helics::ValueFederate vfed2 ("block2", fi);
    helics::Publication pub1 (helics::GLOBAL, &vfed, "pub1", helics::data_type::helics_double);

    auto &pub2 = vfed.registerPublication ("pub2", "double", "m");

    vfed2.registerPublication<double> ("pub");

    vfed2.registerPublication<std::string> ("pub2");

    vfed.registerSubscription ("block2/pub2");

    vfed.registerSubscription ("block2/pub");

    auto fut = std::async (std::launch::async, [&c1] () { c1.runTo (4); });
    vfed2.enterExecutingModeAsync ();
    vfed.enterExecutingMode ();
    vfed2.enterExecutingModeComplete ();
    vfed2.requestTimeAsync (5);
    auto retTime = vfed.requestTime (1);
    vfed2.requestTimeComplete ();
    BOOST_CHECK_EQUAL (retTime, 1.0);
    pub1.publish (3.4);

    retTime = vfed.requestTime (2.0);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    pub1.publish (4.7);
    pub2.publish (3.3);
    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    vfed2.finalize ();
    vfed.finalize ();
    fut.get ();
    c1.finalize ();
    auto cnt = c1.pointCount ();
    BOOST_CHECK_EQUAL (cnt, 3u);
    auto icnt = c1.accessUnderlyingFederate ().getInputCount ();
    BOOST_CHECK_EQUAL (icnt, 2);
    c1.saveFile ("subtest.json");

    BOOST_CHECK (ghc::filesystem::exists ("subtest.json"));
}

BOOST_AUTO_TEST_CASE (simple_clone_test_sub_file,
                      *boost::unit_test::depends_on ("clone_tests/simple_clone_test_sub"))
{
    auto fi = helics::loadFederateInfo ("subtest.json");
    fi.coreName = "clone_core9";
    fi.coreInitString = "--autobroker";
    fi.coreType = helics::core_type::TEST;
    helics::apps::Player c1 ("c1", fi);
    c1.loadFile ("subtest.json");

    c1.initialize ();

    BOOST_CHECK_EQUAL (c1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (c1.publicationCount (), 2u);
    BOOST_CHECK_EQUAL (c1.accessUnderlyingFederate ().getInputCount (), 2);
    c1.finalize ();
    ghc::filesystem::remove ("subtest.json");
}

BOOST_AUTO_TEST_CASE (clone_test_help)
{
    std::vector<std::string> args{"--quiet", "--version"};
    helics::apps::Clone c1 (args);

    BOOST_CHECK (!c1.isActive ());

    args.emplace_back ("-?");
    helics::apps::Clone c2 (args);

    BOOST_CHECK (!c2.isActive ());
}

BOOST_AUTO_TEST_SUITE_END ()
