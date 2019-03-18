/*
Copyright � 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (config_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (control_file_test)
{
    helics_broker broker = AddBroker ("zmq", 1);
    BOOST_CHECK (nullptr != broker);

    std::string testFile (TEST_DIR);
    testFile.append ("Control_test.json");

    auto cfed = helicsCreateCombinationFederateFromConfig (testFile.c_str (), &err);

    BOOST_CHECK (helicsFederateIsValid (cfed));

    BOOST_CHECK_EQUAL (helicsFederateGetEndpointCount (cfed), 6);
    BOOST_CHECK_EQUAL (helicsFederateGetFilterCount (cfed), 6);
    BOOST_CHECK_EQUAL (helicsFederateGetInputCount (cfed), 7);

    auto ept = helicsFederateGetEndpointByIndex (cfed, 0, &err);

    BOOST_CHECK_EQUAL (helicsEndpointGetName (ept), "EV_Controller/EV6");

    auto filt = helicsFederateGetFilterByIndex (cfed, 3, &err);

    BOOST_CHECK_EQUAL (helicsFilterGetName (filt), "EV_Controller/filterEV3");

    auto ipt = helicsFederateGetInputByIndex (cfed, 4, &err);
    BOOST_CHECK_EQUAL (helicsSubscriptionGetKey (ipt), "IEEE_123_feeder_0/charge_EV3");

    helicsFederateDestroy (cfed);
    helicsBrokerDestroy (broker);
}

BOOST_AUTO_TEST_SUITE_END ()
