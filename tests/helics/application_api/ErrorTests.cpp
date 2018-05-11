/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"
#include <complex>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueConverter.hpp"
#include <future>

BOOST_FIXTURE_TEST_SUITE (error_tests, FederateTestFixture)
#define CORE_TYPE_TO_TEST helics::core_type::TEST

BOOST_AUTO_TEST_CASE (duplicate_federate_names)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "2";

    auto Fed = std::make_shared<helics::Federate> (fi);

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> (fi), helics::RegistrationFailure);
}

BOOST_AUTO_TEST_CASE (duplicate_federate_names2)
{
    auto broker = AddBroker ("test", 3);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    auto fed2 = GetFederateAs<helics::ValueFederate> (1);
    helics::FederateInfo fi (fed1->getName ());
    // get the core pointer from fed2 and using the name of fed1 should be an error
    BOOST_CHECK_THROW (helics::ValueFederate fed3 (fed2->getCorePointer (), fi), helics::RegistrationFailure);
    broker->disconnect ();
}


BOOST_AUTO_TEST_CASE(already_init_broker)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate>(0);

    fed1->enterInitializationState();
    helics::FederateInfo fi("fed222",helics::core_type::TEST);
    fi.coreInitString=std::string("--broker=") + broker->getIdentifier();
    BOOST_CHECK_THROW(helics::ValueFederate fed3(fi), helics::RegistrationFailure);
    broker->disconnect();

}

BOOST_AUTO_TEST_CASE(already_init_core)
{
    auto broker = AddBroker("test", 1);
    AddFederates<helics::ValueFederate>("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate>(0);
    fed1->enterExecutionState();
    helics::FederateInfo fi("fed2");
    // get the core pointer from fed2 and using the name of fed1 should be an error
    BOOST_CHECK_THROW(helics::ValueFederate fed2(fed1->getCorePointer(), fi), helics::RegistrationFailure);
    broker->disconnect();

}

BOOST_AUTO_TEST_SUITE_END ()