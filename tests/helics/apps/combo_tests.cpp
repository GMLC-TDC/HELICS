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
#include <helics_includes/filesystem.hpp>
#pragma warning(pop)
#else
#include <helics_includes/filesystem.hpp>
#endif

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Player.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/core/BrokerFactory.hpp"
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (combo_tests, *utf::label ("ci"))

// this is the same as another test in test recorders
BOOST_AUTO_TEST_CASE (save_load_file1)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ccore2";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::CombinationFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");
    rec1.addSubscription ("pub1");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

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
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (rec1.pointCount (), 3u);

    auto filename = ghc::filesystem::temp_directory_path () / "savefile.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename));

    auto filename2 = ghc::filesystem::temp_directory_path () / "savefile.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename2));
}

BOOST_AUTO_TEST_CASE (save_load_file_binary)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ccore3";
    fi.coreInitString = "-f 3 --autobroker";
    helics::apps::Recorder rec1 ("rec1", fi);
    fi.setProperty (helics_property_time_period, 1.0);

    helics::CombinationFederate mfed ("block1", fi);

    helics::MessageFederate mfed2 ("block2", fi);
    helics::Endpoint e1 (helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2 (helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone ("d1");
    rec1.addSourceEndpointClone ("d1");
    rec1.addSubscription ("pub1");

    helics::Publication pub1 (helics::GLOBAL, &mfed, "pub1", helics::data_type::helics_double);

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
    mfed2.enterExecutingModeAsync ();
    mfed.enterExecutingMode ();
    mfed2.enterExecutingModeComplete ();
    pub1.publish (3.4);

    mfed2.requestTimeAsync (1.0);
    auto retTime = mfed.requestTime (1.0);
    mfed2.requestTimeComplete ();
    helics::data_block n5 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    e1.send ("d2", n5);
    pub1.publish (4.7);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    helics::data_block n6 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255 - ii;
    }
    e2.send ("d1", n6);

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

    auto filename = ghc::filesystem::temp_directory_path () / "savefile_binary.txt";
    rec1.saveFile (filename.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename));

    auto filename2 = ghc::filesystem::temp_directory_path () / "savefile_binary.json";
    rec1.saveFile (filename2.string ());

    BOOST_CHECK (ghc::filesystem::exists (filename2));
}

BOOST_AUTO_TEST_CASE (check_created_files1, *boost::unit_test::depends_on ("combo_tests/save_load_file1"))
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ccore4";
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty (helics_property_time_period, 1.0);

    helics::apps::Player play1 ("play1", fi);
    auto filename = ghc::filesystem::temp_directory_path () / "savefile.txt";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1u);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (play1.endpointCount (), 2u);

    play1.finalize ();
    ghc::filesystem::remove (filename);
}

BOOST_AUTO_TEST_CASE (check_created_files2, *boost::unit_test::depends_on ("combo_tests/save_load_file1"))
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ccore5";
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty (helics_property_time_period, 1.0);

    helics::apps::Player play1 ("play1", fi);
    auto filename = ghc::filesystem::temp_directory_path () / "savefile.json";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1u);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (play1.endpointCount (), 2u);

    play1.finalize ();
    ghc::filesystem::remove (filename);
}

BOOST_AUTO_TEST_CASE (check_created_files_binary1,
                      *boost::unit_test::depends_on ("combo_tests/save_load_file_binary"))
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "ccore6";
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty (helics_property_time_period, 1.0);

    helics::apps::Player play1 ("play1", fi);
    auto filename = ghc::filesystem::temp_directory_path () / "savefile_binary.txt";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1u);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (play1.endpointCount (), 2u);

    auto &b1 = play1.getMessage (0);
    helics::data_block n5 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    BOOST_CHECK_EQUAL (b1.mess.data.to_string (), n5.to_string ());

    auto &b2 = play1.getMessage (1);
    helics::data_block n6 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255 - ii;
    }
    BOOST_CHECK_EQUAL (b2.mess.data.to_string (), n6.to_string ());
    play1.finalize ();
    ghc::filesystem::remove (filename);
}

BOOST_AUTO_TEST_CASE (check_created_files_binary2,
                      *boost::unit_test::depends_on ("combo_tests/save_load_file_binary"))
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "ccore7";
    fi.coreInitString = "-f 1 --autobroker";
    fi.setProperty (helics_property_time_period, 1.0);

    helics::apps::Player play1 ("play1", fi);
    auto filename = ghc::filesystem::temp_directory_path () / "savefile_binary.json";
    play1.loadFile (filename.string ());

    play1.initialize ();
    BOOST_CHECK_EQUAL (play1.pointCount (), 3u);
    BOOST_CHECK_EQUAL (play1.publicationCount (), 1u);
    BOOST_CHECK_EQUAL (play1.messageCount (), 2u);

    BOOST_CHECK_EQUAL (play1.endpointCount (), 2u);

    auto &b1 = play1.getMessage (0);
    helics::data_block n5 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    BOOST_CHECK_EQUAL (b1.mess.data.to_string (), n5.to_string ());

    auto &b2 = play1.getMessage (1);
    helics::data_block n6 (256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255 - ii;
    }
    BOOST_CHECK_EQUAL (b2.mess.data.to_string (), n6.to_string ());

    play1.finalize ();
    ghc::filesystem::remove (filename);
}

BOOST_AUTO_TEST_CASE (check_combination_file_load)
{
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreName = "ccore_combo";
    fi.coreInitString = "-f 3 --autobroker";

    helics::apps::Player play1 ("play1", fi);
    play1.loadFile (TEST_DIR "test_HELICS_player.json");

    helics::apps::Recorder rec1 ("rec1", fi);
    rec1.loadFile (TEST_DIR "test_HELICS_recorder.json");

    helics::CombinationFederate fed ("", TEST_DIR "federate_config.json");
    auto fut_rec = std::async (std::launch::async, [&rec1]() { rec1.run (); });
    auto fut_play = std::async (std::launch::async, [&play1]() { play1.run (); });
    fed.enterExecutingMode ();
    auto tm = fed.getCurrentTime ();
    while (tm < 1300.0)
    {
        tm = fed.requestNextStep ();
        if (tm == 120.0)
        {
            fed.getEndpoint (0).send ("73.4");
        }
        if (tm == 730.0)
        {
            fed.getEndpoint (1).send ("on");
        }
        if (tm == 230.0)
        {
            fed.getPublication (0).publish (34.6);
        }
        if (tm == 640.0)
        {
            fed.getPublication (1).publish (1);
        }
    }
    BOOST_CHECK_EQUAL (fed.pendingMessages (), 3u);
    fed.finalize ();
    fut_play.get ();
    fut_rec.get ();
    BOOST_CHECK_EQUAL (rec1.messageCount (), 2u);
    BOOST_CHECK_EQUAL (rec1.pointCount (), 2u);
}

BOOST_AUTO_TEST_SUITE_END ()
