/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.h"
#include "helics/core/CoreFactory.h"
#include "helics/core/core-exceptions.h"
#include "helics/core/core-types.h"
#include "helics/core/core.h"
#include "helics/core/coreFederateInfo.h"

BOOST_AUTO_TEST_SUITE (TestCore_tests)

using helics::Core;
using namespace helics::CoreFactory;

BOOST_AUTO_TEST_CASE (testcore_initialization_test)
{
    std::string initializationString = "4";
    auto core = create (helics::core_type::TEST, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());

    core->connect ();
    BOOST_CHECK (core->isConnected ());
    core->disconnect ();
    BOOST_CHECK_EQUAL (core->isConnected (), false);
}

BOOST_AUTO_TEST_CASE (testcore_pubsub_value_test)
{
    const char *initializationString = "1";
    auto core = create (helics::core_type::TEST, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());
    BOOST_CHECK_EQUAL (core->getFederationSize (), 0);
    core->connect ();
    BOOST_REQUIRE (core->isConnected ());

    Core::federate_id_t id = core->registerFederate ("sim1", helics::CoreFederateInfo ());

    BOOST_CHECK_EQUAL (core->getFederationSize (), 1);
    BOOST_CHECK_EQUAL (core->getFederateName (id), "sim1");
    BOOST_CHECK_EQUAL (core->getFederateId ("sim1"), id);

    core->setTimeDelta (id, 1.0);

    Core::Handle sub1 =
      core->registerSubscription (id, "sim1_pub", "type", "units", helics::handle_check_mode::optional);
    BOOST_CHECK_EQUAL (core->getSubscription (id, "sim1_pub"), sub1);
    BOOST_CHECK_EQUAL (core->getType (sub1), "type");
    BOOST_CHECK_EQUAL (core->getUnits (sub1), "units");

    Core::Handle pub1 = core->registerPublication (id, "sim1_pub", "type", "units");
    BOOST_CHECK_EQUAL (core->getPublication (id, "sim1_pub"), pub1);
    BOOST_CHECK_EQUAL (core->getType (pub1), "type");
    BOOST_CHECK_EQUAL (core->getUnits (pub1), "units");

    core->enterInitializingState (id);

    core->enterExecutingState (id);

    core->timeRequest (id, 50.0);
    std::string str1 = "hello world";
    core->setValue (pub1, str1.data (), str1.size ());
    auto valueUpdates = core->getValueUpdates (id);
    BOOST_CHECK (valueUpdates.empty ());
    auto data = core->getValue (sub1);
    BOOST_CHECK (data == nullptr);

    core->timeRequest (id, 100.0);
    valueUpdates = core->getValueUpdates (id);
    BOOST_CHECK_EQUAL (valueUpdates[0], sub1);
    BOOST_CHECK_EQUAL (valueUpdates.size (), 1u);
    data = core->getValue (sub1);
    std::string str2 (data->to_string ());
    BOOST_CHECK_EQUAL (str1, str2);
    BOOST_CHECK_EQUAL (data->to_string (), "hello world");
    BOOST_CHECK_EQUAL (data->size (), str1.size ());

    core->setValue (pub1, "hello\n\0helloAgain", 17);
    core->timeRequest (id, 150.0);
    valueUpdates = core->getValueUpdates (id);
    BOOST_CHECK_EQUAL (valueUpdates[0], sub1);
    BOOST_CHECK_EQUAL (valueUpdates.size (), 1u);
    data = core->getValue (sub1);
    BOOST_CHECK_EQUAL (data->to_string (), std::string ("hello\n\0helloAgain", 17));
    BOOST_CHECK_EQUAL (data->size (), 17u);

    core->timeRequest (id, 200.0);
    valueUpdates = core->getValueUpdates (id);
    BOOST_CHECK (valueUpdates.empty ());
    core->finalize (id);
    core->disconnect ();
}

BOOST_AUTO_TEST_CASE (testcore_send_receive_test)
{
    const char *initializationString = "1";
    auto core = create (helics::core_type::TEST, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());

    BOOST_CHECK_EQUAL (core->getFederationSize (), 0);
    core->connect ();
    BOOST_REQUIRE (core->isConnected ());
    Core::federate_id_t id = core->registerFederate ("sim1", helics::CoreFederateInfo ());

    BOOST_CHECK_EQUAL (core->getFederateName (id), "sim1");
    BOOST_CHECK_EQUAL (core->getFederateId ("sim1"), id);

    core->setTimeDelta (id, 1.0);

    Core::Handle end1 = core->registerEndpoint (id, "end1", "type");
    BOOST_CHECK_EQUAL (core->getType (end1), "type");

    Core::Handle end2 = core->registerEndpoint (id, "end2", "type");
    BOOST_CHECK_EQUAL (core->getType (end2), "type");

    core->enterInitializingState (id);

    core->enterExecutingState (id);

    std::string str1 = "hello world";
    core->timeRequest (id, 50.0);
    core->send (end1, "end2", str1.data (), str1.size ());

    core->timeRequest (id, 100.0);
    BOOST_CHECK_EQUAL (core->receiveCount (end1), 0);
    BOOST_CHECK_EQUAL (core->receiveCount (end2), 1u);
    auto msg = core->receive (end1);
    BOOST_CHECK (msg == nullptr);
    msg = core->receive (end2);
    BOOST_CHECK_EQUAL (core->receiveCount (end2), 0);
    std::string str2 (msg->data.to_string ());
    BOOST_CHECK_EQUAL (str1, str2);
    BOOST_CHECK_EQUAL (msg->data.size (), str1.size ());
    core->disconnect ();
}

BOOST_AUTO_TEST_CASE (testcore_messagefilter_source_test)
{
    const char *initializationString = "1";
    auto core = create (helics::core_type::TEST, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());
    core->connect ();
    BOOST_REQUIRE (core->isConnected ());
    Core::federate_id_t id = core->registerFederate ("sim1", helics::CoreFederateInfo ());

    Core::Handle end1 = core->registerEndpoint (id, "end1", "type");
    Core::Handle end2 = core->registerEndpoint (id, "end2", "type");
    Core::Handle endp = core->registerEndpoint (id, "end_filt", "type");

    Core::Handle srcFilter = core->registerSourceFilter (id, "srcFilter", "end1", "type", "type");

    core->enterInitializingState (id);
    core->enterExecutingState (id);

    std::string msgData = "hello world";
    std::string srcFilterName = "sourceFilter";
    core->send (end1, "end2", msgData.data (), msgData.size ());

    core->timeRequest (id, 50.0);

    // Get message to filter. Update src and send to destination.
    BOOST_CHECK_EQUAL (core->receiveFilterCount (id), 1u);

    BOOST_CHECK_EQUAL (core->receiveCount (srcFilter), 1u);
    Core::Handle filter_id;
    auto msgAny = core->receiveAnyFilter (id, filter_id);
    BOOST_CHECK_EQUAL (msgAny->origsrc, "end1");
    BOOST_CHECK_EQUAL (msgAny->src, "end1");
    msgAny->src = srcFilterName;
    // use a dummy message to test code the error return
    BOOST_CHECK_THROW (core->sendMessage (helics::invalid_Handle, std::make_unique<helics::Message> ()),
                       helics::invalidIdentifier);
    // we are also testing that the error did not change the message
    core->sendMessage (endp, std::move (msgAny));
    core->timeRequest (id, 60.0);
    // Receive the filtered message
    BOOST_CHECK_EQUAL (core->receiveCount (end2), 1u);
    BOOST_CHECK_EQUAL (core->receiveCount (endp), 0);
    auto msg = core->receive (end2);
    BOOST_CHECK_EQUAL (msg->origsrc, "end1");
    BOOST_CHECK_EQUAL (msg->src, "end_filt");
    core->disconnect ();
}

BOOST_AUTO_TEST_CASE (testcore_messagefilter_callback_test)
{
    // Create filter operator
    class TestOperator : public helics::FilterOperator
    {
      public:
        TestOperator (const std::string &name) : filterName (name) {}

        std::unique_ptr<helics::Message> process (std::unique_ptr<helics::Message> msg) override
        {
            msg->src = filterName;

            if (msg->data.size () > 0)
            {
                ++msg->data[0];
            }
            return msg;
        }

        std::string filterName;
    };

    std::string initializationString = "1";
    auto core = create (helics::core_type::TEST, initializationString);

    BOOST_REQUIRE (core != nullptr);
    BOOST_CHECK (core->isInitialized ());
    core->connect ();
    BOOST_REQUIRE (core->isConnected ());
    Core::federate_id_t id = core->registerFederate ("sim1", helics::CoreFederateInfo ());

    Core::Handle end1 = core->registerEndpoint (id, "end1", "type");
    Core::Handle end2 = core->registerEndpoint (id, "end2", "type");

    Core::Handle srcFilter = core->registerSourceFilter (id, "srcFilter", "end1", "type", "type");
    Core::Handle dstFilter = core->registerDestinationFilter (id, "dstFilter", "end2", "type", "type");

    auto testSrcFilter = std::make_shared<TestOperator> ("sourceFilter");
    BOOST_CHECK_EQUAL (testSrcFilter->filterName, "sourceFilter");

    auto testDstFilter = std::make_shared<TestOperator> ("destinationFilter");
    BOOST_CHECK_EQUAL (testDstFilter->filterName, "destinationFilter");

    core->setFilterOperator (srcFilter, testSrcFilter);
    core->setFilterOperator (dstFilter, testDstFilter);

    core->enterInitializingState (id);
    core->enterExecutingState (id);

    std::string msgData = "hello world";
    core->send (end1, "end2", msgData.data (), msgData.size () + 1);

    core->timeRequest (id, 50.0);

    // All filters are using callbacks, there should be none to filter
    BOOST_CHECK_EQUAL (core->receiveFilterCount (id), 0u);

    // Receive the filtered message
    BOOST_CHECK_EQUAL (core->receiveCount (end2), 1u);
    auto msg = core->receive (end2);
    BOOST_CHECK_EQUAL (msg->origsrc, "end1");
    auto res = msg->data.to_string ();
    BOOST_CHECK_EQUAL (res.compare (0, 11, "jello world"), 0);
    core->disconnect ();
}

BOOST_AUTO_TEST_SUITE_END ()
