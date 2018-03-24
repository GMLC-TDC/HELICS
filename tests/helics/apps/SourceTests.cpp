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

BOOST_AUTO_TEST_SUITE(source_tests)

BOOST_AUTO_TEST_CASE(simple_source_test)
{
    helics::FederateInfo fi("player1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::apps::Source src1(fi);
    fi.name = "block1";
    auto index=src1.addSignalGenerator("ramp", "ramp");
    auto gen = src1.getGenerator(index);
    BOOST_CHECK(gen);
    gen->set("ramp", 0.3);
    gen->set("level", 1.0);
    src1.addPublication("pub1", helics::helics_type_t::helicsDouble, 1.0);
    src1.setStartTime("pub1", 1.0);
    helics::ValueFederate vfed(fi);
    helics::Subscription sub1(&vfed, "pub1");
    auto fut = std::async(std::launch::async, [&src1]() { src1.run(5); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    auto val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 1.3);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 1.6);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 3.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 1.9);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 4.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 2.2);

    retTime = vfed.requestTime(6);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    val = sub1.getValue<double>();
    BOOST_CHECK_EQUAL(val, 2.5);
    vfed.finalize();
    fut.get();
}

BOOST_AUTO_TEST_SUITE_END()

