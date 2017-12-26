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
#include "helics/player/recorder.h"
#include "helics/application_api/Publications.hpp"
#include <future>

BOOST_AUTO_TEST_SUITE (recorder_tests)



BOOST_AUTO_TEST_CASE(simple_recorder_test)
{
    helics::FederateInfo fi("rec1");
    fi.coreType = helics::core_type::TEST;
    fi.coreName = "core1";
    fi.coreInitString = "2";
    helics::recorder rec1(fi);
    fi.name = "block1";
    rec1.addSubscription("pub1");

    helics::ValueFederate vfed(fi);
    helics::Publication pub1(helics::GLOBAL,&vfed, "pub1",helics::helicsType_t::helicsDouble);
    auto fut = std::async(std::launch::async, [&rec1]() {rec1.run(4); });
    vfed.enterExecutionState();
    auto retTime = vfed.requestTime(1);
    BOOST_CHECK_EQUAL(retTime, 1.0);
    pub1.publish(3.4);
    

    retTime = vfed.requestTime(2.0);
    BOOST_CHECK_EQUAL(retTime, 2.0);
    pub1.publish(4.7);

    retTime = vfed.requestTime(5);
    BOOST_CHECK_EQUAL(retTime, 5.0);
    
    vfed.finalize();
    fut.get();
    auto cnt = rec1.pointCount();
    BOOST_CHECK_EQUAL(cnt, 2);



}

BOOST_AUTO_TEST_SUITE_END ()
