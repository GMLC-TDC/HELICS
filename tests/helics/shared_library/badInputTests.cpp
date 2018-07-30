/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ctestFixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (bad_input_tests, FederateTestFixture, *utf::label("ci"))

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_bad_fed)
{
    SetupTest(helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt(0);
    
	// this number is a completely garbage value to test bad input and not give a system fault
	auto vFed2 = reinterpret_cast<helics_federate> (reinterpret_cast<uint64_t>(vFed1)+8); 
    // register the publications

    CE(helicsFederateEnterInitializationMode(vFed1));
    auto status = helicsFederateEnterInitializationMode (vFed2);
    BOOST_CHECK_EQUAL (status, helics_invalid_object);
   // auto core = helicsFederateGetCoreObject(vFed1);

    CE(helicsFederateFinalize(vFed1));
    status=helicsFederateFinalize(vFed2);
    BOOST_CHECK_EQUAL (status, helics_invalid_object);

	helicsFederateFree (vFed1);
    helics_time_t gtime;
    status = helicsFederateGetCurrentTime (vFed1, &gtime);
    BOOST_CHECK_EQUAL (status, helics_invalid_object);
	//just make sure this doesn't crash
    helicsFederateFree (vFed1);
	//and make sure this doesn't crash
    helicsFederateFree (vFed2);
}

BOOST_AUTO_TEST_SUITE_END ()
