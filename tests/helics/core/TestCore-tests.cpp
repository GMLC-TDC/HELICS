/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/core/TestCore.h"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/core-types.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (TestCore_tests, *utf::label("ci") *utf::label("daily") *utf::label("release"))

using helics::Core;
using namespace helics::CoreFactory;

BOOST_AUTO_TEST_CASE (testcore_initialization_test)
{
    auto broker = helics::BrokerFactory::create (helics::core_type::TEST, std::string ());
    BOOST_REQUIRE (broker);
    BOOST_CHECK (broker->isConnected ());
    std::string initializationString = std::string ("4") + " --broker=" + broker->getIdentifier ();
    auto core = create (helics::core_type::TEST, initializationString);

    auto Tcore = std::dynamic_pointer_cast<helics::testcore::TestCore> (core);

    BOOST_REQUIRE (core);
    BOOST_REQUIRE (Tcore);
    BOOST_CHECK (core->isInitialized ());

    core->connect ();

    BOOST_CHECK (core->isConnected ());
    core->disconnect ();
    broker->disconnect ();
    BOOST_CHECK_EQUAL (core->isConnected (), false);
    BOOST_CHECK_EQUAL (broker->isConnected (), false);
    core = nullptr;
    broker = nullptr;
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

    Core::handle_id_t sub1 =
      core->registerSubscription (id, "sim1_pub", "type", "units", helics::handle_check_mode::optional);
    BOOST_CHECK_EQUAL (core->getSubscription (id, "sim1_pub"), sub1);
    BOOST_CHECK_EQUAL (core->getType (sub1), "type");
    BOOST_CHECK_EQUAL (core->getUnits (sub1), "units");

    Core::handle_id_t pub1 = core->registerPublication (id, "sim1_pub", "type", "units");
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
    core = nullptr;
    helics::CoreFactory::cleanUpCores ();
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

    Core::handle_id_t end1 = core->registerEndpoint (id, "end1", "type");
    BOOST_CHECK_EQUAL (core->getType (end1), "type");

    Core::handle_id_t end2 = core->registerEndpoint (id, "end2", "type");
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
    core = nullptr;
    helics::CoreFactory::cleanUpCores ();
}

BOOST_AUTO_TEST_CASE (testcore_messagefilter_callback_test)
{
    // Create filter operator
    class TestOperator : public helics::FilterOperator
    {
      public:
        explicit TestOperator (const std::string &name) : filterName (name) {}

        std::unique_ptr<helics::Message> process (std::unique_ptr<helics::Message> msg) override
        {
            msg->source = filterName;

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

    Core::handle_id_t end1 = core->registerEndpoint (id, "end1", "type");
    Core::handle_id_t end2 = core->registerEndpoint (id, "end2", "type");

    Core::handle_id_t srcFilter = core->registerSourceFilter ("srcFilter", "end1", "type", "type");
    Core::handle_id_t dstFilter = core->registerDestinationFilter ("dstFilter", "end2", "type", "type");

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

    // Receive the filtered message
    BOOST_CHECK_EQUAL (core->receiveCount (end2), 1u);
    auto msg = core->receive (end2);
    BOOST_CHECK_EQUAL (msg->original_source, "end1");
    auto res = msg->data.to_string ();
    BOOST_CHECK_EQUAL (res.compare (0, 11, "jello world"), 0);
    core->disconnect ();
    core = nullptr;
    helics::CoreFactory::cleanUpCores ();
}

BOOST_AUTO_TEST_SUITE_END ()
