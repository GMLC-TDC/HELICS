/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../application_api/testFixtures.hpp"
#include "helics/core/core-exceptions.hpp"
#include <complex>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */
#include "../application_api/testFixtures.hpp"
#include "helics/application_api.hpp"
#include <future>

namespace utf = boost::unit_test;
namespace bdata = boost::unit_test::data;

BOOST_FIXTURE_TEST_SUITE (error_tests, FederateTestFixture)
#define CORE_TYPE_TO_TEST helics::core_type::TEST

BOOST_AUTO_TEST_CASE (duplicate_federate_names)
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "-f 2";

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    BOOST_CHECK_THROW (std::make_shared<helics::Federate> ("test1", fi), helics::RegistrationFailure);
    Fed->finalize ();
}

BOOST_AUTO_TEST_CASE (duplicate_federate_names2)
{
    auto broker = AddBroker ("test", 3);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    auto fed2 = GetFederateAs<helics::ValueFederate> (1);
    helics::FederateInfo fi;
    // get the core pointer from fed2 and using the name of fed1 should be an error
    BOOST_CHECK_THROW (helics::ValueFederate fed3 (fed1->getName (), fed2->getCorePointer (), fi),
                       helics::RegistrationFailure);
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (already_init_broker)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate> (0);

    fed1->enterInitializingMode ();
    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreInitString = std::string ("--broker=") + broker->getIdentifier ();
    BOOST_CHECK_THROW (helics::ValueFederate fed3 ("fed222", fi), helics::RegistrationFailure);
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (already_init_core)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    fed1->enterExecutingMode ();
    // get the core pointer from fed2 and using the name of fed1 should be an error
    BOOST_CHECK_THROW (helics::ValueFederate fed2 ("fed2", fed1->getCorePointer (), helics::FederateInfo ()),
                       helics::RegistrationFailure);
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_publication_names)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::ValueFederate> ("test", 2, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    auto fed2 = GetFederateAs<helics::ValueFederate> (1);

    fed1->registerGlobalPublication ("testkey", "");
    BOOST_CHECK_THROW (fed2->registerGlobalPublication ("testkey", ""), helics::RegistrationFailure);
    fed1->finalize ();
    fed2->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_publication_names2)
{
    auto broker = AddBroker ("test", 2);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    auto fed2 = GetFederateAs<helics::ValueFederate> (1);

    fed1->registerGlobalPublication ("testkey", "");
    fed1->enterInitializingModeAsync ();

    fed2->registerGlobalPublication ("testkey", "");

    BOOST_CHECK_THROW (fed2->enterInitializingMode (), helics::RegistrationFailure);
    fed1->finalize ();
    fed2->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_publication_names3)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);

    fed1->registerPublication ("testkey", "");

    BOOST_CHECK_THROW (fed1->registerPublication ("testkey", ""), helics::RegistrationFailure);
    fed1->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_publication_names4)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);

    // all 3 of these should publish to the same thing
    auto &pubid = fed1->registerPublication ("testkey", "");

    helics::Publication pub (fed1, "testkey", helics::helics_type_t::helicsDouble);
    // copy constructor
    helics::Publication pub2 (pubid);

    auto &sub = fed1->registerSubscription (fed1->getPublicationKey (pubid));
    fed1->enterExecutingMode ();
    fed1->publish (pubid, 45.7);
    fed1->requestTime (1.0);
    auto res = sub.getValue<double> ();
    BOOST_CHECK_EQUAL (res, 45.7);
    pub.publish (99.2);

    fed1->requestTime (2.0);
    res = sub.getValue<double> ();
    BOOST_CHECK_EQUAL (res, 99.2);
    pub2.publish (103.8);

    fed1->requestTime (3.0);
    res = sub.getValue<double> ();
    BOOST_CHECK_EQUAL (res, 103.8);
    fed1->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_endpoint_names)
{
    auto broker = AddBroker ("test", 1);
    AddFederates<helics::MessageFederate> ("test", 2, broker, 1.0, "fed");
    auto fed1 = GetFederateAs<helics::MessageFederate> (0);
    auto fed2 = GetFederateAs<helics::MessageFederate> (1);

    fed1->registerGlobalEndpoint ("testEpt");
    BOOST_CHECK_THROW (fed2->registerGlobalEndpoint ("testEpt"), helics::RegistrationFailure);
    fed1->finalize ();
    fed2->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (duplicate_endpoint_names2)
{
    auto broker = AddBroker ("test", 2);
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "fed");
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::MessageFederate> (0);
    auto fed2 = GetFederateAs<helics::MessageFederate> (1);

    fed1->registerGlobalEndpoint ("testEpt");
    fed2->registerGlobalEndpoint ("testEpt");

    BOOST_CHECK_THROW (fed2->enterInitializingMode (), helics::RegistrationFailure);
    fed1->finalize ();
    fed2->finalize ();
    broker->disconnect ();
}

BOOST_AUTO_TEST_CASE (missing_required_pub)
{
    auto broker = AddBroker ("test", 2);

    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");
    AddFederates<helics::ValueFederate> ("test", 1, broker, 1.0, "fed");

    auto fed1 = GetFederateAs<helics::ValueFederate> (0);
    auto fed2 = GetFederateAs<helics::ValueFederate> (1);

    fed1->registerGlobalPublication ("t1", "");
    auto &i2 = fed2->registerSubscription ("abcd", "");
    i2.setOption (helics::defs::options::connection_required, true);
    // TODO:: add required flag back
    fed1->enterInitializingModeAsync ();
    BOOST_CHECK_THROW (fed2->enterInitializingMode (), helics::RegistrationFailure);
    fed1->finalize ();
    fed2->finalize ();
    broker->disconnect ();
}

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_duplicate_broker_name, bdata::make (core_types_simple), core_type)
{
    auto broker = AddBroker (core_type, "--name=brk1");
    BOOST_CHECK (broker->isConnected ());
    BOOST_CHECK_THROW (AddBroker (core_type, "--name=brk1 --timeout=500"), helics::RegistrationFailure);
    broker->disconnect ();
    helics::cleanupHelicsLibrary ();
}

const std::string networkCores[] = {"zmq", "tcp", "udp"};

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_duplicate_default_brokers, bdata::make (networkCores), core_type)
{
    auto broker = AddBroker (core_type, "");
    auto broker2 = AddBroker (core_type, "--timeout=500");
    BOOST_CHECK (!broker2->isConnected ());
    broker->disconnect ();
    helics::cleanupHelicsLibrary ();
}

/** test broker recovery*/
BOOST_DATA_TEST_CASE (test_broker_recovery, bdata::make (networkCores), core_type)
{
    auto broker = AddBroker (core_type, "");
    BOOST_REQUIRE (broker->isConnected ());
    auto res = std::async (std::launch::async, [&broker]() {
        std::this_thread::sleep_for (std::chrono::milliseconds (1400));
        broker->disconnect ();
    });
    auto broker2 = AddBroker (core_type, " --timeout=2500");
    BOOST_CHECK (!broker->isConnected ());
    BOOST_CHECK (broker2->isConnected ());
    broker2->disconnect ();
    helics::cleanupHelicsLibrary ();
}

BOOST_AUTO_TEST_SUITE_END ()
