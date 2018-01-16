/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/player.h"
#include "helics/apps/recorder.h"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.h"
#include <future>

BOOST_AUTO_TEST_SUITE (combo_tests)

// this is the same as another test in test recorders
BOOST_AUTO_TEST_CASE (save_load_file1)
{
    helics::FederateInfo fi ("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "3";
    helics::Recorder rec1 (fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::CombinationFederate mfed (fi);

    fi.name = "block2";

    helics::MessageFederate mfed2 (fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");
    rec1.addSubscription ("pub1");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::helics_type_t::helicsDouble);

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.run (5.0); });
    mfed2.enterExecutionStateAsync ();
    mfed.enterExecutionState ();
    mfed2.enterExecutionStateComplete ();
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
}

BOOST_AUTO_TEST_CASE (check_created_files1, *boost::unit_test::depends_on ("combo_tests/save_load_file1"))
{
    helics::FederateInfo fi ("play1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::Player play1 (fi);
    auto filename = boost::filesystem::temp_directory_path () / "savefile.txt";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2);
    BOOST_CHECK_EQUAL (play1.endpointCount (), 2);

    play1.finalize ();
    boost::filesystem::remove (filename);
}

BOOST_AUTO_TEST_CASE (check_created_files2, *boost::unit_test::depends_on ("combo_tests/save_load_file1"))
{
    helics::FederateInfo fi ("play1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::Player play1 (fi);
    auto filename = boost::filesystem::temp_directory_path () / "savefile.json";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2);
    BOOST_CHECK_EQUAL (play1.endpointCount (), 2);

    play1.finalize ();
    boost::filesystem::remove (filename);
}

BOOST_AUTO_TEST_SUITE_END ()
