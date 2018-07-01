/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/common/barrier.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/core/core-types.hpp"
#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace utf = boost::unit_test;

static helics::Barrier barrier (4);

void simA (std::shared_ptr<helics::Core> core, const std::string &NAME)
{
    auto id = core->registerFederate (NAME, helics::CoreFederateInfo ());
    BOOST_CHECK_EQUAL (NAME, core->getFederateName (id));
    BOOST_CHECK_EQUAL (id, core->getFederateId (NAME));

    // simple barrier
    barrier.Wait ();

    core->setTimeDelta (id, 1);
    auto sub1 = core->registerSubscription (id, NAME, "type", "units", PUBLICATION_REQUIRED);
    BOOST_CHECK_EQUAL (sub1, core->getSubscription (id, NAME));

    auto pub1 = core->registerPublication (id, NAME, "type", "units");

    auto end1 = core->registerEndpoint (id, NAME, "type");

    core->enterInitializingState (id);

    core->enterExecutingState (id);

    // time loop
    core->timeRequest (id, 50);
    std::string str1 = "hello world";
    core->setValue (pub1, str1.data (), str1.size ());
    // data = core->getValue(pub1); // this would assert
    auto data = core->getValue (sub1);  // should be empty so far
    auto events = core->getValueUpdates (id);
    BOOST_CHECK (events.empty ());
    core->timeRequest (id, 100);
    events = core->getValueUpdates (id);
    BOOST_CHECK_EQUAL (1u, events.size ());
    data = core->getValue (sub1);

    std::string str2 (data->to_string ());

    BOOST_CHECK_EQUAL (str1, str2);
    BOOST_CHECK_EQUAL (data->size (), str1.size ());
    core->setValue (pub1, "hello\n\0helloAgain", 17);
    core->timeRequest (id, 150);
    events = core->getValueUpdates (id);
    BOOST_CHECK_EQUAL (1u, events.size ());
    data = core->getValue (sub1);
    BOOST_CHECK_EQUAL (data->to_string (), "hello\n\0helloAgain");
    BOOST_CHECK_EQUAL (data->size (), 17);

    core->timeRequest (id, 200);
    core->send (end1, "simA1", "test123", 8);
    core->timeRequest (id, 250);
    while (core->receiveCount (end1) > 0)
    {
        auto end1_recv = core->receive (end1);
        BOOST_CHECK_EQUAL (end1_recv->data.to_string (), "test123");
    }

    core->finalize (id);
}

void simB (std::shared_ptr<helics::Core> core, const std::string &NAME)
{
    auto id = core->registerFederate (NAME, helics::CoreFederateInfo ());
    BOOST_CHECK_EQUAL (NAME, core->getFederateName (id));
    BOOST_CHECK_EQUAL (id, core->getFederateId (NAME));

    // simple barrier
    barrier.Wait ();

    core->setTimeDelta (id, 1);
    auto sub1 = core->registerSubscription (id, NAME, "type", "units", PUBLICATION_REQUIRED);
    BOOST_CHECK_EQUAL (sub1, core->getSubscription (id, NAME));

    core->enterInitializingState (id);

    core->enterExecutingState (id);

    // time loop

    core->requestTimeIterative (id, 100, helics::iteration_request::force_iteration);
    core->requestTimeIterative (id, 100, helics::iteration_request::no_iterations);
    core->requestTimeIterative (id, 105, helics::iteration_request::force_iteration);
    core->requestTimeIterative (id, 105, helics::iteration_request::no_iterations);
    core->finalize (id);
}

BOOST_AUTO_TEST_SUITE (Node_Core_tests, *utf::label("ci") *utf::disabled ())

BOOST_AUTO_TEST_CASE (TestCore_node_tests)
{
    auto core = helics::CoreFactory::create (helics::core_type::TEST, "4");
    BOOST_CHECK (core != nullptr);
    BOOST_CHECK (core->isInitialized ());

    std::thread simA1_thread (simA, core, "simA1");
    std::thread simA2_thread (simA, core, "simA2");
    std::thread simB1_thread (simB, core, "simB1");
    std::thread simB2_thread (simB, core, "simB2");

    simA1_thread.join ();
    simA2_thread.join ();
    simB1_thread.join ();
    simB2_thread.join ();
}

BOOST_AUTO_TEST_CASE (ZeromqCore_node_tests)
{
    auto core = helics::CoreFactory::create (helics::core_type::ZMQ, "1");
    BOOST_CHECK (core != nullptr);
    BOOST_CHECK (core->isInitialized ());

    std::thread simA1_thread (simA, core, "simA1");
    std::thread simA2_thread (simA, core, "simA2");
    std::thread simB1_thread (simB, core, "simB1");
    std::thread simB2_thread (simB, core, "simB2");

    simA1_thread.join ();
    simA2_thread.join ();
    simB1_thread.join ();
    simB2_thread.join ();
}

BOOST_AUTO_TEST_SUITE_END ()
