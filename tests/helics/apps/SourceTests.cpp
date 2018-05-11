/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <cstdio>

#include "exeTestHelper.h"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/apps/Source.hpp"
#include "helics/common/stringToCmdLine.h"
#include "helics/core/BrokerFactory.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE (source_tests, *boost::unit_test::disabled())

BOOST_AUTO_TEST_CASE (simple_source_test )
{
    helics::FederateInfo fi ("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    fi.name = "block1";
    auto index = src1.addSignalGenerator ("ramp", "ramp");
    auto gen = src1.getGenerator (index);
    BOOST_CHECK (gen);
    gen->set ("ramp", 0.3);
    gen->set ("level", 1.0);
    src1.addPublication ("pub1", helics::helics_type_t::helicsDouble, 1.0);
    src1.setStartTime ("pub1", 1.0);
    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.3);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.6);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 4.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.2);

    retTime = vfed.requestTime (6);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.5);
    vfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (simple_source_test2)
{
    helics::FederateInfo fi ("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    fi.name = "block1";
    auto index = src1.addSignalGenerator ("ramp", "ramp");
    auto index2 = src1.addSignalGenerator ("ramp2", "ramp");
    auto gen = src1.getGenerator (index);
    auto gen2 = src1.getGenerator (index2);
    BOOST_CHECK (gen);
    BOOST_CHECK (gen2);
    gen->set ("ramp", 0.3);
    gen->set ("level", 1.0);
    src1.addPublication ("pub1", "ramp", helics::helics_type_t::helicsDouble, 1.0);
    src1.setStartTime ("pub1", 1.0);
    gen2->set ("ramp", 0.6);
    gen2->set ("level", 2.0);
    src1.addPublication ("pub2", "ramp2", helics::helics_type_t::helicsDouble, 2.0);
    src1.setStartTime ("pub2", 3.0);
    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    helics::Subscription sub2 (&vfed, "pub2");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (1.1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.3);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.6);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.9);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 3.8);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 4.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.2);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 3.8);

    retTime = vfed.requestTime (6);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.5);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 5.0);
    vfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (sine_source_test)
{
    helics::FederateInfo fi ("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    fi.name = "block1";
    auto index = src1.addSignalGenerator ("sine", "sine");
    auto gen = src1.getGenerator (index);
    BOOST_CHECK (gen);
    gen->set ("freq", 0.5);
    gen->set ("amplitude", 1.0);
    src1.addPublication ("pub1", helics::helics_type_t::helicsDouble, 0.5);
    src1.setStartTime ("pub1", 1.0);
    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (5);

    BOOST_CHECK_EQUAL (retTime, 1.0);
    double val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 1.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, -1.0, 1e-9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, 1.0, 1e-9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, -1.0, 1e-9);
    vfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (simple_source_test_file)
{
    helics::FederateInfo fi ("source1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "corep";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    src1.loadFile (std::string (TEST_DIR) + "/test_files/simple_source_test.json");

    fi.name = "block1";
    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (1.1);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.3);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.6);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 4.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.2);

    retTime = vfed.requestTime (6);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.5);
    vfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (simple_source_test2_file)
{
    helics::FederateInfo fi ("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core2";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    fi.name = "block1";

    src1.loadFile (std::string (TEST_DIR) + "/test_files/simple_source_test2.json");
    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    helics::Subscription sub2 (&vfed, "pub2");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 1.0);
    auto val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.3);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.6);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 1.9);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 3.8);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 4.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.2);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 3.8);

    retTime = vfed.requestTime (6);
    BOOST_CHECK_EQUAL (retTime, 5.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 2.5);
    val = sub2.getValue<double> ();
    BOOST_CHECK_EQUAL (val, 5.0);
    vfed.finalize ();
    fut.get ();
}

BOOST_AUTO_TEST_CASE (sine_source_test_file)
{
    helics::FederateInfo fi ("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::apps::Source src1 (fi);
    src1.loadFile (std::string (TEST_DIR) + "/test_files/simple_sine_source.json");

    fi.name = "block1";

    helics::ValueFederate vfed (fi);
    helics::Subscription sub1 (&vfed, "pub1");
    auto fut = std::async (std::launch::async, [&src1]() {
        src1.runTo (5);
        src1.finalize ();
    });
    vfed.enterExecutionState ();
    auto retTime = vfed.requestTime (5);

    BOOST_CHECK_EQUAL (retTime, 1.0);
    double val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 1.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, -1.0, 1e-9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 2.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, 1.0, 1e-9);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.0);
    val = sub1.getValue<double> ();
    BOOST_CHECK_SMALL (val, 1e-12);

    retTime = vfed.requestTime (5);
    BOOST_CHECK_EQUAL (retTime, 3.5);
    val = sub1.getValue<double> ();
    BOOST_CHECK_CLOSE (val, -1.0, 1e-9);
    vfed.finalize ();
    fut.get ();
}
BOOST_AUTO_TEST_SUITE_END ()
