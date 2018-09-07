/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/application_api/Federate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/MessageOperators.hpp"
#include "testFixtures.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
/** these test cases test out the message federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (filter_tests, FederateTestFixture, *utf::label ("ci"))

/** test registration of filters*/
BOOST_DATA_TEST_CASE (message_filter_registration, bdata::make (core_types_all), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 2, broker, helics::timeZero, "filter");
    AddFederates<helics::MessageFederate> (core_type, 2, broker, helics::timeZero, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    mFed->registerGlobalEndpoint ("port1");
    mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto f2 = fFed->registerFilter ("filter2");
    fFed->addDestinationTarget (f2, "port2");
    BOOST_CHECK (f2 != f1);
    auto ep1 = fFed->registerEndpoint ("fout");
    BOOST_CHECK (ep1.value () != helics::invalid_id_value);
    auto f3 = fFed->registerFilter ();
    fFed->addSourceTarget (f3,"filter0/fout");
    BOOST_CHECK (f3 != f2);
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/
BOOST_DATA_TEST_CASE (message_filter_function, bdata::make (core_types_all), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1,"port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator (f1, timeOperator);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    mFed->requestTime (3.0);
    fFed->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_filter_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto Filt = helics::make_filter (helics::defined_filter_types::delay, fFed.get ());
    Filt->addSourceTarget ("port1");
    Filt->set ("delay", 2.5);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    mFed->requestTime (3.0);
    fFed->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_dest_filter_function, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerFilter ("filter1");
    fFed->addDestinationTarget (f1, "port2");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator (f1, timeOperator);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    mFed->requestTime (3.0);
    fFed->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a remote dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_dest_filter_function_t2, bdata::make (core_types_all), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 2, broker, 0.5, "message");

    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed1->registerGlobalEndpoint ("port1");
    auto p2 = mFed2->registerGlobalEndpoint ("port2");

    auto f1 = mFed2->registerFilter ("filter1");
    mFed2->addSourceTarget (f1, "port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
    mFed2->setFilterOperator (f1, timeOperator);

    mFed1->enterExecutingModeAsync ();
    mFed2->enterExecutingMode ();
    mFed1->enterExecutingModeComplete ();

    BOOST_CHECK (mFed2->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed1->sendMessage (p1, "port2", data);

    mFed1->requestTimeAsync (1.0);
    mFed2->requestTime (1.0);
    mFed1->requestTimeComplete ();

    auto res = mFed2->hasMessage ();
    BOOST_CHECK (!res);

    mFed1->requestTimeAsync (2.0);
    mFed2->requestTime (2.0);
    mFed1->requestTimeComplete ();
    BOOST_REQUIRE (!mFed2->hasMessage (p2));

    mFed1->requestTimeAsync (3.0);
    auto retTime = mFed2->requestTime (3.0);

    BOOST_CHECK (retTime == 2.5);
    BOOST_REQUIRE (mFed2->hasMessage (p2));

    auto m2 = mFed2->getMessage (p2);

    mFed2->requestTime (3.0);
    mFed1->requestTimeComplete ();
    mFed1->finalize ();
    mFed2->finalize ();
    BOOST_CHECK (mFed2->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a remove dest filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_dest_filter_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = helics::make_filter (helics::defined_filter_types::delay, fFed->getCorePointer ().get (), "filter1");
    f1->addDestinationTarget ("port2");
    f1->set ("delay", 2.5);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    mFed->requestTime (3.0);
    fFed->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_filter_function_two_stage, bdata::make (core_types_all), core_type)
{
    auto broker = AddBroker (core_type, 3);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto fFed2 = GetFederateAs<helics::MessageFederate> (1);
    auto mFed = GetFederateAs<helics::MessageFederate> (2);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 1.25; });
    fFed->setFilterOperator (f1, timeOperator);

    auto f2 = fFed2->registerFilter ("filter2");
    fFed2->addSourceTarget (f2, "port1");
    BOOST_CHECK (f2.value () != helics::invalid_id_value);

    fFed2->setFilterOperator (f2, timeOperator);

    fFed->enterExecutingModeAsync ();
    fFed2->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();
    fFed2->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTimeAsync (1.0);
    fFed2->requestTime (1.0);
    mFed->requestTimeComplete ();
    fFed->requestTimeComplete ();
    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed2->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    fFed2->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    fFed2->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);
    if (!mFed->hasMessage (p2))
    {
        printf ("missing message\n");
    }
    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    fFed->requestTimeComplete ();
    fFed2->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    fFed2->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_filter_function_two_stage_object, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 3);
    BOOST_REQUIRE (broker);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter2");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto fFed2 = GetFederateAs<helics::MessageFederate> (1);
    auto mFed = GetFederateAs<helics::MessageFederate> (2);

    BOOST_REQUIRE (fFed);
    BOOST_REQUIRE (fFed2);
    BOOST_REQUIRE (mFed);
    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = helics::make_filter (helics::defined_filter_types::delay, fFed.get (), "filter1");
    f1->addSourceTarget ("port1");
    f1->set ("delay", 1.25);

    auto f2 = helics::make_filter (helics::defined_filter_types::delay, fFed.get (), "filter2");
    f2->addSourceTarget ("port1");
    f2->set ("delay", 1.25);

    fFed->enterExecutingModeAsync ();
    fFed2->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();
    fFed2->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTimeAsync (1.0);
    fFed2->requestTime (1.0);
    mFed->requestTimeComplete ();
    fFed->requestTimeComplete ();
    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);

    mFed->requestTimeAsync (2.0);
    fFed2->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    fFed2->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    fFed->requestTimeAsync (3.0);
    fFed2->requestTimeAsync (3.0);
    /*auto retTime = */ mFed->requestTime (3.0);
    if (!mFed->hasMessage (p2))
    {
        printf ("missing message\n");
    }
    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    fFed->requestTimeComplete ();
    fFed2->requestTimeComplete ();
    mFed->finalize ();
    fFed->finalize ();
    fFed2->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}
/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the simulation
*/

BOOST_DATA_TEST_CASE (message_filter_function2, bdata::make (core_types), core_type)
{
    auto broker = AddBroker (core_type, 2);
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "filter");
    AddFederates<helics::MessageFederate> (core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAs<helics::MessageFederate> (0);
    auto mFed = GetFederateAs<helics::MessageFederate> (1);
    BOOST_REQUIRE (fFed);
    BOOST_REQUIRE (mFed);

    auto p1 = mFed->registerGlobalEndpoint ("port1");
    auto p2 = mFed->registerGlobalEndpoint ("port2");

    auto f1 = fFed->registerFilter ("filter1");
    fFed->addSourceTarget (f1, "port1");
    auto f2 = fFed->registerFilter ("filter2");
    fFed->addSourceTarget (f2, "port2");
    BOOST_CHECK (f1.value () != helics::invalid_id_value);
    auto timeOperator = std::make_shared<helics::MessageTimeOperator> ();
    timeOperator->setTimeFunction ([](helics::Time time_in) { return time_in + 2.5; });
    fFed->setFilterOperator ({f1, f2}, timeOperator);

    fFed->enterExecutingModeAsync ();
    mFed->enterExecutingMode ();
    fFed->enterExecutingModeComplete ();

    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    mFed->sendMessage (p1, "port2", data);

    mFed->requestTimeAsync (1.0);
    fFed->requestTime (1.0);
    mFed->requestTimeComplete ();

    auto res = mFed->hasMessage ();
    BOOST_CHECK (!res);
    mFed->sendMessage (p2, "port1", data);
    mFed->requestTimeAsync (2.0);
    fFed->requestTime (2.0);
    mFed->requestTimeComplete ();
    BOOST_REQUIRE (!mFed->hasMessage (p2));

    std::this_thread::yield ();
    mFed->requestTime (3.0);

    BOOST_REQUIRE (mFed->hasMessage (p2));

    auto m2 = mFed->getMessage (p2);
    BOOST_CHECK_EQUAL (m2->source, "port1");
    BOOST_CHECK_EQUAL (m2->original_source, "port1");
    BOOST_CHECK_EQUAL (m2->dest, "port2");
    BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    BOOST_CHECK_EQUAL (m2->time, 2.5);

    BOOST_CHECK (!mFed->hasMessage (p1));
    mFed->requestTime (4.0);
    BOOST_CHECK (mFed->hasMessage (p1));
    mFed->finalize ();
    fFed->finalize ();
    BOOST_CHECK (fFed->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_AUTO_TEST_CASE (message_clone_test)
{
    auto broker = AddBroker ("test", 3);
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "source");
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate> (0);
    auto dFed = GetFederateAs<helics::MessageFederate> (1);
    auto dcFed = GetFederateAs<helics::MessageFederate> (2);

    auto p1 = sFed->registerGlobalEndpoint ("src");
    auto p2 = dFed->registerGlobalEndpoint ("dest");
    auto p3 = dcFed->registerGlobalEndpoint ("cm");

    helics::CloningFilter cFilt (dcFed.get ());
    cFilt.addSourceTarget ("src");
    cFilt.addDeliveryEndpoint ("cm");

    sFed->enterExecutingModeAsync ();
    dcFed->enterExecutingModeAsync ();
    dFed->enterExecutingMode ();
    sFed->enterExecutingModeComplete ();
    dcFed->enterExecutingModeComplete ();

    BOOST_CHECK (sFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    sFed->sendMessage (p1, "dest", data);

    sFed->requestTimeAsync (1.0);
    dcFed->requestTimeAsync (1.0);
    dFed->requestTime (1.0);
    sFed->requestTimeComplete ();
    dcFed->requestTimeComplete ();

    auto res = dFed->hasMessage ();
    BOOST_CHECK (res);

    if (res)
    {
        auto m2 = dFed->getMessage (p2);
        BOOST_CHECK_EQUAL (m2->source, "src");
        BOOST_CHECK_EQUAL (m2->original_source, "src");
        BOOST_CHECK_EQUAL (m2->dest, "dest");
        BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    }

    // now check the message clone
    res = dcFed->hasMessage ();
    BOOST_CHECK (res);

    if (res)
    {
        auto m2 = dcFed->getMessage (p3);
        BOOST_CHECK_EQUAL (m2->source, "src");
        BOOST_CHECK_EQUAL (m2->original_source, "src");
        BOOST_CHECK_EQUAL (m2->dest, "cm");
        BOOST_CHECK_EQUAL (m2->original_dest, "dest");
        BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
    }

    sFed->finalize ();
    dFed->finalize ();
    dcFed->finalize ();
    BOOST_CHECK (sFed->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_AUTO_TEST_CASE (message_multi_clone_test)
{
    auto broker = AddBroker ("test", 4);
    AddFederates<helics::MessageFederate> ("test", 2, broker, 1.0, "source");
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "dest");
    AddFederates<helics::MessageFederate> ("test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAs<helics::MessageFederate> (0);
    auto sFed2 = GetFederateAs<helics::MessageFederate> (1);
    auto dFed = GetFederateAs<helics::MessageFederate> (2);
    auto dcFed = GetFederateAs<helics::MessageFederate> (3);

    auto p1 = sFed->registerGlobalEndpoint ("src");
    auto p2 = sFed2->registerGlobalEndpoint ("src2");
    auto p3 = dFed->registerGlobalEndpoint ("dest");
    auto p4 = dcFed->registerGlobalEndpoint ("cm");

    helics::CloningFilter cFilt (dcFed.get ());
    cFilt.addSourceTarget ("src");
    cFilt.addSourceTarget ("src2");
    cFilt.addDeliveryEndpoint ("cm");

    sFed->enterExecutingModeAsync ();
    sFed2->enterExecutingModeAsync ();
    dcFed->enterExecutingModeAsync ();
    dFed->enterExecutingMode ();
    sFed->enterExecutingModeComplete ();
    sFed2->enterExecutingModeComplete ();
    dcFed->enterExecutingModeComplete ();

    BOOST_CHECK (sFed->getCurrentState () == helics::Federate::op_states::execution);
    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');
    sFed->sendMessage (p1, "dest", data);
    sFed2->sendMessage (p2, "dest", data2);
    sFed->requestTimeAsync (1.0);
    sFed2->requestTimeAsync (1.0);
    dcFed->requestTimeAsync (1.0);
    dFed->requestTime (1.0);
    sFed->requestTimeComplete ();
    sFed2->requestTimeComplete ();
    dcFed->requestTimeComplete ();

    auto mcnt = dFed->pendingMessages (p3);
    BOOST_CHECK_EQUAL (mcnt, 2);
    auto res = dFed->hasMessage ();
    BOOST_CHECK (res);

    if (res)
    {
        auto m2 = dFed->getMessage (p3);
        BOOST_CHECK_EQUAL (m2->source, "src");
        BOOST_CHECK_EQUAL (m2->original_source, "src");
        BOOST_CHECK_EQUAL (m2->dest, "dest");
        BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
        res = dFed->hasMessage ();
        BOOST_CHECK (res);

        if (res)
        {
            m2 = dFed->getMessage (p3);
            BOOST_CHECK_EQUAL (m2->source, "src2");
            BOOST_CHECK_EQUAL (m2->original_source, "src2");
            BOOST_CHECK_EQUAL (m2->dest, "dest");
            BOOST_CHECK_EQUAL (m2->data.size (), data2.size ());
        }
    }

    // now check the message clone
    mcnt = dcFed->pendingMessages (p4);
    BOOST_CHECK_EQUAL (mcnt, 2);
    res = dcFed->hasMessage ();
    BOOST_CHECK (res);

    if (res)
    {
        auto m2 = dcFed->getMessage (p4);
        BOOST_CHECK_EQUAL (m2->source, "src");
        BOOST_CHECK_EQUAL (m2->original_source, "src");
        BOOST_CHECK_EQUAL (m2->dest, "cm");
        BOOST_CHECK_EQUAL (m2->original_dest, "dest");
        BOOST_CHECK_EQUAL (m2->data.size (), data.size ());
        res = dcFed->hasMessage ();
        BOOST_CHECK (res);

        if (res)
        {
            m2 = dcFed->getMessage (p4);
            BOOST_CHECK_EQUAL (m2->source, "src2");
            BOOST_CHECK_EQUAL (m2->original_source, "src2");
            BOOST_CHECK_EQUAL (m2->dest, "cm");
            BOOST_CHECK_EQUAL (m2->original_dest, "dest");
            BOOST_CHECK_EQUAL (m2->data.size (), data2.size ());
        }
    }

    sFed->finalize ();
    sFed2->finalize ();
    dFed->finalize ();
    dcFed->finalize ();
    BOOST_CHECK (sFed->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_AUTO_TEST_SUITE_END ()
