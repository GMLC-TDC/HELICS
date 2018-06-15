/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Player.hpp"
#include "helics/apps/Recorder.hpp"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE (combo_tests)

// this is the same as another test in test recorders
BOOST_AUTO_TEST_CASE (save_load_file1)
{
    helics::FederateInfo fi ("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "3";
    helics::apps::Recorder rec1 (fi);
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

    auto fut = std::async (std::launch::async, [&rec1]() { rec1.runTo (5.0); });
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


BOOST_AUTO_TEST_CASE(save_load_file_binary)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core3";
    fi.coreInitString = "3";
    helics::apps::Recorder rec1(fi);
    fi.period = 1.0;
    fi.name = "block1";

    helics::CombinationFederate mfed(fi); 
    fi.name = "block2";

    helics::MessageFederate mfed2(fi);
    helics::Endpoint e1(helics::GLOBAL, &mfed, "d1");
    helics::Endpoint e2(helics::GLOBAL, &mfed2, "d2");

    rec1.addDestEndpointClone("d1");
    rec1.addSourceEndpointClone("d1");
    rec1.addSubscription("pub1");

    helics::Publication pub1(helics::GLOBAL, &mfed, "pub1", helics::helics_type_t::helicsDouble);

    auto fut = std::async(std::launch::async, [&rec1]() { rec1.runTo(5.0); });
    mfed2.enterExecutionStateAsync();
    mfed.enterExecutionState();
    mfed2.enterExecutionStateComplete();
    pub1.publish(3.4);

    mfed2.requestTimeAsync(1.0);
    auto retTime = mfed.requestTime(1.0);
    mfed2.requestTimeComplete();
    helics::data_block n5(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    e1.send("d2", n5);
    pub1.publish(4.7);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    helics::data_block n6(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255-ii;
    }
    e2.send("d1", n6);

    mfed2.requestTimeAsync(2.0);
    retTime = mfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);

    mfed2.requestTimeComplete();
    pub1.publish(4.7);

    mfed.finalize();
    mfed2.finalize();
    fut.get();
    BOOST_CHECK_EQUAL(rec1.messageCount(), 2);
    BOOST_CHECK_EQUAL(rec1.pointCount(), 3);

    auto filename = boost::filesystem::temp_directory_path() / "savefile_binary.txt";
    rec1.saveFile(filename.string());

    BOOST_CHECK(boost::filesystem::exists(filename));

    auto filename2 = boost::filesystem::temp_directory_path() / "savefile_binary.json";
    rec1.saveFile(filename2.string());

    BOOST_CHECK(boost::filesystem::exists(filename2));
}

BOOST_AUTO_TEST_CASE (check_created_files1, *boost::unit_test::depends_on ("combo_tests/save_load_file1"))
{
    helics::FederateInfo fi ("play1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core4";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::apps::Player play1 (fi);
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
    fi.coreName = "core5";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::apps::Player play1 (fi);
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


BOOST_AUTO_TEST_CASE(check_created_files_binary1, *boost::unit_test::depends_on("combo_tests/save_load_file_binary"))
{
    helics::FederateInfo fi("play1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core6";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::apps::Player play1(fi);
    auto filename = boost::filesystem::temp_directory_path() / "savefile_binary.txt";
    play1.loadFile(filename.string());

    play1.initialize();
    BOOST_CHECK_EQUAL(play1.pointCount(), 3);
    BOOST_CHECK_EQUAL(play1.publicationCount(), 1);
    BOOST_CHECK_EQUAL(play1.messageCount(), 2);
    BOOST_CHECK_EQUAL(play1.endpointCount(), 2);

    auto &b1 = play1.getMessage(0);
    helics::data_block n5(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    BOOST_CHECK_EQUAL(b1.mess.data.to_string(), n5.to_string());

    auto &b2 = play1.getMessage(1);
    helics::data_block n6(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255 - ii;
    }
    BOOST_CHECK_EQUAL(b2.mess.data.to_string(), n6.to_string());
    play1.finalize();
    boost::filesystem::remove(filename);
}

BOOST_AUTO_TEST_CASE(check_created_files_binary2, *boost::unit_test::depends_on("combo_tests/save_load_file_binary"))
{
    helics::FederateInfo fi("play1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core7";
    fi.coreInitString = "1";
    fi.period = 1.0;

    helics::apps::Player play1(fi);
    auto filename = boost::filesystem::temp_directory_path() / "savefile_binary.json";
    play1.loadFile(filename.string());

    play1.initialize();
    BOOST_CHECK_EQUAL(play1.pointCount(), 3);
    BOOST_CHECK_EQUAL(play1.publicationCount(), 1);
    BOOST_CHECK_EQUAL(play1.messageCount(), 2);
    
    BOOST_CHECK_EQUAL(play1.endpointCount(), 2);

    auto &b1 = play1.getMessage(0);
    helics::data_block n5(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n5[ii] = ii;
    }
    BOOST_CHECK_EQUAL(b1.mess.data.to_string(), n5.to_string());

    auto &b2 = play1.getMessage(1);
    helics::data_block n6(256);
    for (int ii = 0; ii < 256; ++ii)
    {
        n6[ii] = 255 - ii;
    }
    BOOST_CHECK_EQUAL(b2.mess.data.to_string(), n6.to_string());

    play1.finalize();
    boost::filesystem::remove(filename);
}
BOOST_AUTO_TEST_SUITE_END ()
