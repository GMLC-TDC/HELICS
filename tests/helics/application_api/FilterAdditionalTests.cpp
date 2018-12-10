/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/FilterOperations.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/MessageOperators.hpp"
#include "testFixtures.hpp"
#include <helics/core/Broker.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
/** these test cases test out the message federates
 */

BOOST_FIXTURE_TEST_SUITE (additional_filter_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;

namespace utf = boost::unit_test;

/**
Test rerouter filter
This test case sets reroute filter on a source endpoint. This means message
sent from this endpoint will be rerouted to a new destination endpoint.
*/

BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_DATA_TEST_CASE (message_reroute_filter_object1, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);

    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");
    auto &p3 = mFed->registerGlobalEndpoint ("port3");

    auto &Filt = helics::make_filter (helics::filter_types::reroute, fFed.get (), "filter1");
    Filt.addSourceTarget ("port1");
    Filt.setString ("newdestination", "port3");

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    BOOST_CHECK (!mFed->hasMessage (p2));
    BOOST_REQUIRE (mFed->hasMessage (p3));

    auto m2 = mFed->getMessage (p3);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_dest, "port2");
    BOOST_CHECK_EQUAL (m2->dest, "port3");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());

    fFed->requestTimeAsync (2.0);
    mFed->requestTime (2.0);
    fFed->requestTimeComplete ();

    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test rerouter filter under condition
This test case sets reroute filter on a source endpoint with a condition parameter.
This means message sent from this endpoint will be rerouted to a new destination
endpoint only if condition matches.
*/
BOOST_TEST_DECORATOR (*utf::label ("ci"))
BOOST_DATA_TEST_CASE (message_reroute_filter_condition, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("endpt2");
    auto &p3 = mFed->registerGlobalEndpoint ("port3");

    auto &f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    auto filter_op = std::make_shared<helics::RerouteFilterOperation> ();
    filter_op->setString ("newdestination", "port3");
    filter_op->setString ("condition", "end");  // match all messages with a destination endpoint stating with
                                                // "end"
    fFed->setFilterOperator (f1, filter_op->getOperator ());

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "endpt2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();
    BOOST_CHECK (!mFed->hasMessage (p2));
    BOOST_REQUIRE (mFed->hasMessage (p3));
    auto m2 = mFed->getMessage (p3);

    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port3");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());

    fFed->requestTimeAsync (2.0);
    mFed->requestTime (2.0);
    fFed->requestTimeComplete ();

    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test rerouter filter
This test case sets reroute filter on a destination endpoint. This means message
sent to this endpoint will be rerouted to a new destination endpoint.
*/

BOOST_DATA_TEST_CASE (message_reroute_filter_object2, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);

    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");
    auto &p3 = mFed->registerGlobalEndpoint ("port3");

    auto &f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    auto filter_op = std::make_shared<helics::RerouteFilterOperation> ();
    filter_op->setString ("newdestination", "port3");
    filter_op->setString ("condition",
                          "test");  // match all messages with a destination endpoint starting with "test"

    fFed->setFilterOperator (f1, filter_op->getOperator ());

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();
    // this one was delivered to the original destination
    BOOST_REQUIRE (mFed->hasMessage (p2));

    // this message should be delivered to the rerouted destination
    mFed->sendMessage (p1, "test324525", data);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    if (mFed->hasMessage (p3) == false)
    {
        BOOST_CHECK (mFed->hasMessage (p3));
    }
    else
    {
        auto m2 = mFed->getMessage (p3);
        BOOST_CHECK_EQUAL (m2->source, "port1");
        BOOST_CHECK_EQUAL (m2->dest, "port3");
        BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    }

    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test random drop filter
This test case sets random drop filter on a source endpoint with a particular
message drop probability. This means messages may be dropped randomly with a
probability of 0.75.
*/
BOOST_DATA_TEST_CASE (message_random_drop_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    auto &Filt = helics::make_filter (helics::filter_types::random_drop, fFed.get (), "filter1");
    Filt.addSourceTarget ("port1");
    double drop_prob = 0.75;
    Filt.set ("dropprob", drop_prob);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 200;
    int dropped = 0;
    for (int i = 0; i < max_iterations; i++)
    {
        mFed->sendMessage (p1, "port2", data);
        timestep += 1.0;
        mFed->requestTime (timestep);
        // Check if message is received
        if (!mFed->hasMessage (p2))
        {
            dropped++;
        }
        else
        {
            mFed->getMessage (p2);
        }
    }
    auto iterations = static_cast<double> (max_iterations);
    double pest = static_cast<double> (dropped) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt (drop_prob * (1.0 - drop_prob) / iterations);

    BOOST_CHECK_GE (pest, drop_prob - ebar);
    BOOST_CHECK_LE (pest, drop_prob + ebar);
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test random drop filter
This test case sets random drop filter on a source endpoint with a particular
message arrival probability. This means messages may be received randomly with a
probability of 0.9.
*/
BOOST_DATA_TEST_CASE (message_random_drop_object1, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    auto &f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    auto op = std::make_shared<helics::RandomDropFilterOperation> ();
    double prob = 0.45;
    op->set ("prob", prob);
    fFed->setFilterOperator (f1, op->getOperator ());

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int count = 0;
    for (int i = 0; i < max_iterations; i++)
    {
        mFed->sendMessage (p1, "port2", data);
        timestep += 1.0;
        mFed->requestTime (timestep);
        // Check if message is received
        if (mFed->hasMessage (p2))
        {
            count++;
            mFed->getMessage (p2);
        }
    }
    auto iterations = static_cast<double> (max_iterations);
    double pest = 1.0 - static_cast<double> (count) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt (prob * (1.0 - prob) / iterations);

    BOOST_CHECK_GE (pest, prob - ebar);
    BOOST_CHECK_LE (pest, prob + ebar);
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test random drop filter
This test case sets random drop filter on a destination endpoint with a particular
message drop probability. This means messages may be dropped randomly with a
probability of 0.75.
*/
BOOST_DATA_TEST_CASE (message_random_drop_dest_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    auto &Filt = helics::make_filter (helics::filter_types::random_drop, fFed.get (), "filter1");
    Filt.addDestinationTarget ("port2");
    double drop_prob = 0.25;
    Filt.set ("dropprob", drop_prob);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (100, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int dropped = 0;
    for (int i = 0; i < max_iterations; i++)
    {
        mFed->sendMessage (p1, "port2", data);
        timestep += 1.0;
        mFed->requestTime (timestep);
        // Check if message is received
        if (!mFed->hasMessage (p2))
        {
            dropped++;
        }
        else
        {
            // purposely dropping the messages
            mFed->getMessage (p2);
        }
    }

    auto iterations = static_cast<double> (max_iterations);
    double pest = static_cast<double> (dropped) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt (drop_prob * (1.0 - drop_prob) / iterations);

    BOOST_CHECK_GE (pest, drop_prob - ebar);
    BOOST_CHECK_LE (pest, drop_prob + ebar);
    mFed->finalize ();
    fFed->finalize ();
}

/**
Test random drop filter
This test case sets random drop filter on a destination endpoint with a particular
message arrival probability. This means messages may be received randomly with a
probability of 0.9.
*/
BOOST_DATA_TEST_CASE (message_random_drop_dest_object1, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    auto &f1 = fFed->registerFilter ("filter1");
    fFed->addDestinationTarget (f1, "port2");
    auto op = std::make_shared<helics::RandomDropFilterOperation> ();
    double prob = 0.1;
    op->set ("prob", prob);
    fFed->setFilterOperator (f1, op->getOperator ());

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');

    double timestep = 0.0;  // 1 second
    int max_iterations = 150;
    int count = 0;
    for (int i = 0; i < max_iterations; i++)
    {
        mFed->sendMessage (p1, "port2", data);
        timestep++;
        mFed->requestTime (timestep);
        if (mFed->hasMessage (p2))
        {
            count++;
            mFed->getMessage (p2);
        }
    }
    auto iterations = static_cast<double> (max_iterations);
    double pest = 1.0 - static_cast<double> (count) / iterations;
    // this should result in an expected error of 1 in 10K tests
    double ebar = 4.5 * std::sqrt (prob * (1.0 - prob) / iterations);

    BOOST_CHECK_GE (pest, prob - ebar);
    BOOST_CHECK_LE (pest, prob + ebar);
    mFed->finalize ();
    fFed->finalize ();
}

/**
Test random delay filter
This test case sets random delay filter on a source endpoint.
This means messages may be delayed by random delay based on
binomial distribution.
*/
BOOST_DATA_TEST_CASE (message_random_delay_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    auto &Filt = helics::make_filter (helics::filter_types::random_delay, fFed.get (), "filter1");
    Filt.addSourceTarget ("port1");
    Filt.setString ("distribution", "binomial");

    Filt.set ("param1", 4);  // max_delay=4
    Filt.set ("param2", 0.5);  // prob

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (100, 'a');
    mFed->sendMessage (p1, "port2", data);

    double timestep = 0.0;  // 1 second
    int max_iterations = 4;
    int count = 0;
    double actual_delay = 100.0;

    for (int i = 0; i < max_iterations; i++)
    {
        timestep += 1.0;
        mFed->requestTime (timestep);
        // Check if message is received
        if (mFed->hasMessage (p2))
        {
            auto m2 = mFed->getMessage (p2);
            BOOST_CHECK_EQUAL (m2->source, "port1");
            BOOST_CHECK_EQUAL (m2->dest, "port2");
            BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
            actual_delay = m2->time;
            count++;
        }
    }
    BOOST_CHECK_EQUAL (count, 1);
    BOOST_CHECK (actual_delay <= 4);

    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::states::finalize);
}

/**
Test filter info fields
*/
BOOST_DATA_TEST_CASE (test_filter_info_field, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto &p1 = mFed->registerGlobalEndpoint ("port1");
    auto &p2 = mFed->registerGlobalEndpoint ("port2");

    p1.setInfo ("p1_info");
    p2.setInfo ("p2_info");

    auto &f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    f1.setInfo ("f1_info");
    auto &f2 = fFed->registerFilter ("filter2");
    fFed->addDestinationTarget (f2, "port2");
    f2.setInfo ("f2_info");
    auto &ep1 = fFed->registerEndpoint ("fout");
    ep1.setInfo ("ep1_info");
    auto &f3 = fFed->registerFilter ();
    fFed->addSourceTarget (f3, "filter0/fout");
    f3.setInfo ("f3_info");

    // Test Endpoint info field
    BOOST_CHECK_EQUAL ("p1_info", p1.getInfo ());
    BOOST_CHECK_EQUAL ("p2_info", p2.getInfo ());
    BOOST_CHECK_EQUAL ("ep1_info", ep1.getInfo ());
    BOOST_CHECK_EQUAL ("p1_info", mFed->getInfo (p1.getHandle ()));
    BOOST_CHECK_EQUAL ("p2_info", mFed->getInfo (p2.getHandle ()));
    BOOST_CHECK_EQUAL ("ep1_info", fFed->getInfo (ep1.getHandle ()));

    // Test Filter info field
    BOOST_CHECK_EQUAL ("f1_info", f1.getInfo ());
    BOOST_CHECK_EQUAL ("f2_info", f2.getInfo ());
    BOOST_CHECK_EQUAL ("f3_info", f3.getInfo ());
    BOOST_CHECK_EQUAL ("f1_info", fFed->getInfo (f1.getHandle ()));
    BOOST_CHECK_EQUAL ("f2_info", fFed->getInfo (f2.getHandle ()));
    BOOST_CHECK_EQUAL ("f3_info", fFed->getInfo (f3.getHandle ()));

    mFed->finalize ();
    fFed->finalize ();
}

BOOST_AUTO_TEST_CASE (test_empty) {}
BOOST_AUTO_TEST_SUITE_END ()
