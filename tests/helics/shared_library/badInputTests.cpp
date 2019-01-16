/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ctestFixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (bad_input_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_bad_fed)
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt (0);

    // this number is a completely garbage value to test bad input and not give a system fault
    auto vFed2 = reinterpret_cast<helics_federate> (reinterpret_cast<uint64_t> (vFed1) + 8);
    // register the publications

    CE (helicsFederateEnterInitializingMode (vFed1, &err));
    helicsFederateEnterInitializingMode (vFed2, &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_error_invalid_object);
    helicsErrorClear (&err);
    // auto core = helicsFederateGetCoreObject(vFed1);

    CE (helicsFederateFinalize (vFed1, &err));
    helicsFederateFinalize (vFed2, &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_error_invalid_object);

    helicsFederateFree (vFed1);
    helicsFederateGetCurrentTime (vFed1, &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_error_invalid_object);
    helicsErrorClear (&err);
    // just make sure this doesn't crash
    helicsFederateFree (vFed1);
    // and make sure this doesn't crash
    helicsFederateFree (vFed2);
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_mistaken_free)
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt (0);
    auto fi = helicsCreateFederateInfo ();
    CE (helicsFederateInfoSetBroker (fi, "broker test", &err));
    CE (helicsFederateEnterInitializingMode (vFed1, &err));
    CE (helicsFederateFinalize (vFed1, &err));

    helicsFederateInfoFree (vFed1);  // this is totally wrong but we are testing it
    helicsFederateFree (fi);  // this is also backwards

    helicsQueryFree (fi);  // also bad
    helicsQueryFree (vFed1);

    helicsFederateInfoFree (fi);  // now do the correct frees
    helicsFederateFree (vFed1);
}

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (test_mistaken_finalize)
{
    SetupTest (helicsCreateValueFederate, "test", 1);
    auto vFed1 = GetFederateAt (0);
    auto fi = helicsCreateFederateInfo ();
    CE (helicsFederateInfoSetBroker (fi, "broker test", &err));
    CE (helicsFederateEnterInitializingMode (vFed1, &err));
    helicsFederateFinalize (fi, &err);

    BOOST_CHECK_NE (err.error_code, 0);

    helicsFederateInfoFree (vFed1);  // this is totally wrong but we are testing it
    helicsFederateFree (fi);  // this is also backwards

    helicsQueryFree (fi);  // also bad
    helicsQueryFree (vFed1);

    helicsFederateInfoFree (fi);  // now do the correct frees
    helicsFederateFree (vFed1);
}
BOOST_AUTO_TEST_SUITE_END ()
