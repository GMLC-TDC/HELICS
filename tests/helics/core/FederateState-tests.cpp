/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include <future>

#include "helics/core/EndpointInfo.h"
#include "helics/core/FederateState.h"
#include "helics/core/FilterInfo.h"
#include "helics/core/PublicationInfo.h"
#include "helics/core/SubscriptionInfo.h"

BOOST_FIXTURE_TEST_SUITE (FederateState_tests, federateStateTestFixture)

BOOST_AUTO_TEST_CASE (constructor_test)
{
    // Check setting of name, initial state, and info by the constructor
    BOOST_CHECK_EQUAL (fs->getIdentifier (), "fed_name");
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_CREATED);

    BOOST_CHECK_EQUAL (fs->getInfo ().timeDelta, helics::Time::epsilon ());
    BOOST_CHECK_EQUAL (fs->getInfo ().lookAhead, helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getInfo ().impactWindow, helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getInfo ().period, helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getInfo ().observer, false);
    BOOST_CHECK_EQUAL (fs->getInfo ().uninterruptible, false);
    BOOST_CHECK_EQUAL (fs->getInfo ().source_only, false);

    // Check other default state values
    BOOST_CHECK_EQUAL (fs->getQueueSize (), 0);
    BOOST_CHECK_EQUAL (fs->getEvents ().size (), 0);

    // BOOST_CHECK_EQUAL(fs->message_queue.size(), 0);
    // BOOST_CHECK_EQUAL(fs->dependencies.size(), 0);
    BOOST_CHECK_EQUAL (fs->getDependents ().size (), 0);
    BOOST_CHECK_EQUAL (fs->local_id, helics::invalid_fed_id);
    BOOST_CHECK_EQUAL (fs->global_id, helics::invalid_fed_id);
    BOOST_CHECK_EQUAL (fs->init_requested, false);
    // BOOST_CHECK_EQUAL(fs->processing, false);
    BOOST_CHECK_EQUAL (fs->iterating, false);
    BOOST_CHECK_EQUAL (fs->hasEndpoints, false);
    BOOST_CHECK_EQUAL (fs->getCurrentIteration (), 0);
    BOOST_CHECK_EQUAL (fs->grantedTime (), helics::Time::minVal ());
    // BOOST_CHECK_EQUAL(fs->time_requested, helics::Time::zeroVal());
    // BOOST_CHECK_EQUAL(fs->time_next, helics::Time::zeroVal());
    // BOOST_CHECK_EQUAL(fs->time_minDe, helics::Time::zeroVal());
    // BOOST_CHECK_EQUAL(fs->time_minTe, helics::Time::zeroVal());
    // BOOST_CHECK_EQUAL(fs->time_event, helics::Time::zeroVal());
    BOOST_CHECK_EQUAL (fs->getInfo ().max_iterations, 3);
}

BOOST_AUTO_TEST_CASE (create_subscription_test)
{
    using namespace helics;
    fs->createSubscription (0, "first!", "type", "units", handle_check_mode::required);
    fs->createSubscription (1, "second", "type", "units", handle_check_mode::required);
    fs->createSubscription (3, "last", "type", "units", handle_check_mode::optional);
    fs->createSubscription (2, "cut-in-line", "type", "units", handle_check_mode::optional);

    helics::SubscriptionInfo *info;

    // Check first subscription
    info = fs->getSubscription ("first!");
    BOOST_CHECK_EQUAL (info->id, 0);

    info = fs->getSubscription (0);
    BOOST_CHECK (info->key.compare ("first!") == 0);

    // Check second subscription
    info = fs->getSubscription ("second");
    BOOST_CHECK_EQUAL (info->id, 1);

    info = fs->getSubscription (1);
    BOOST_CHECK (info->key.compare ("second") == 0);

    // Check the out of order subscription
    info = fs->getSubscription ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id, 2);

    info = fs->getSubscription (2);
    BOOST_CHECK (info->key.compare ("cut-in-line") == 0);

    // Check the displaced (last) subscription
    info = fs->getSubscription ("last");
    BOOST_CHECK_EQUAL (info->id, 3);

    info = fs->getSubscription (3);
    BOOST_CHECK (info->key.compare ("last") == 0);
}

BOOST_AUTO_TEST_CASE (create_publication_test)
{
    fs->createPublication (0, "first!", "type", "units");
    fs->createPublication (1, "second", "type", "units");
    fs->createPublication (3, "last", "type", "units");
    fs->createPublication (2, "cut-in-line", "type", "units");

    helics::PublicationInfo *info;

    // Check first publication
    info = fs->getPublication ("first!");
    BOOST_CHECK_EQUAL (info->id, 0);

    info = fs->getPublication (0);
    BOOST_CHECK (info->key.compare ("first!") == 0);

    // Check second publication
    info = fs->getPublication ("second");
    BOOST_CHECK_EQUAL (info->id, 1);

    info = fs->getPublication (1);
    BOOST_CHECK (info->key.compare ("second") == 0);

    // Check the out of order publication
    info = fs->getPublication ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id, 2);

    info = fs->getPublication (2);
    BOOST_CHECK (info->key.compare ("cut-in-line") == 0);

    // Check the displaced (last) publication
    info = fs->getPublication ("last");
    BOOST_CHECK_EQUAL (info->id, 3);

    info = fs->getPublication (3);
    BOOST_CHECK (info->key.compare ("last") == 0);
}

BOOST_AUTO_TEST_CASE (create_endpoint_test)
{
    fs->createEndpoint (0, "first!", "type");
    fs->createEndpoint (1, "second", "type");
    fs->createEndpoint (3, "last", "type");
    fs->createEndpoint (2, "cut-in-line", "type");

    helics::EndpointInfo *info;

    // Check first endpoint
    info = fs->getEndpoint ("first!");
    BOOST_CHECK_EQUAL (info->id, 0);

    info = fs->getEndpoint (0);
    BOOST_CHECK (info->key.compare ("first!") == 0);

    // Check second endpoint
    info = fs->getEndpoint ("second");
    BOOST_CHECK_EQUAL (info->id, 1);

    info = fs->getEndpoint (1);
    BOOST_CHECK (info->key.compare ("second") == 0);

    // Check the out of order endpoint
    info = fs->getEndpoint ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id, 2);

    info = fs->getEndpoint (2);
    BOOST_CHECK (info->key.compare ("cut-in-line") == 0);

    // Check the displaced (last) endpoint
    info = fs->getEndpoint ("last");
    BOOST_CHECK_EQUAL (info->id, 3);

    info = fs->getEndpoint (3);
    BOOST_CHECK (info->key.compare ("last") == 0);
}

BOOST_AUTO_TEST_CASE (basic_processmessage_test)
{
    using namespace helics;
    ActionMessage cmd;

    // Test returning when the initialization state is entered
    cmd.setAction (helics::CMD_INIT_GRANT);
    auto fs_process = std::async (std::launch::async, [&]() { return fs->enterInitState (); });
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_CREATED);
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::next_step);
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_INITIALIZING);

    // Test returning when the finished state is entered
    cmd.setAction (helics::CMD_STOP);
    auto fs_process2 = std::async (std::launch::async,
                                   [&]() { return fs->enterExecutingState (iteration_request::no_iterations); });
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_INITIALIZING);
    fs->addAction (cmd);
    fs->global_id = 0;  // if it doesn't match the id in the command, this will hang
    fs_process2.wait ();
    fs->global_id = helics::invalid_fed_id;
    BOOST_CHECK (fs_process2.get () == iteration_result::halted);
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_FINISHED);

    // Return to created state
    fs->reset ();

    // Test CMD_FED_ACK message when no error
    cmd.setAction (helics::CMD_FED_ACK);
    cmd.dest_id = 22;
    cmd.name = "fed_name";
    CLEAR_ACTION_FLAG (cmd, error_flag);
    fs_process = std::async (std::launch::async, [&]() { return fs->waitSetup (); });
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::next_step);
    BOOST_CHECK_EQUAL (fs->global_id, 22);

    // Test CMD_FED_ACK message with an error
    cmd.setAction (helics::CMD_FED_ACK);
    cmd.dest_id = 23;
    SET_ACTION_FLAG (cmd, error_flag);
    fs_process = std::async (std::launch::async, [&]() { return fs->waitSetup (); });
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::error);
    BOOST_CHECK_EQUAL (fs->global_id, 22);
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_ERROR);

    // Return to initializing state
    fs->reInit ();

    // Test returning when an error occurs
    cmd.setAction (helics::CMD_ERROR);
    fs_process2 = std::async (std::launch::async,
                              [&]() { return fs->enterExecutingState (iteration_request::no_iterations); });
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_INITIALIZING);
    fs->addAction (cmd);
    fs_process2.wait ();
    BOOST_CHECK (fs_process2.get () == iteration_result::error);
    BOOST_CHECK_EQUAL (fs->getState (), helics_federate_state_type::HELICS_ERROR);

    fs->reset ();

    // Test CMD_EXEC_REQUEST/CMD_EXEC_GRANT returns 0 if dependencies/dependents aren't done; returns 1 if
    // fs->iterating is true, 2 otherwise; if 1 ret false, if 2 ret true
    cmd.setAction (helics::CMD_EXEC_GRANT);

    // Test CMD_TIME_REQUEST/CMD_TIME_GRANT
    cmd.setAction (helics::CMD_TIME_GRANT);

    /* CMD_TIME_REQUEST and CMD_TIME_GRANT; manipulate time factors in ways to ensure behavior is correct for
    deciding event to allow or not; test based on desired functionality time_next and time_minDe compared against
    everything else Time Tallow(std::max(time_next, time_minDe)); if (time_event <= Tallow)
    {
        return 2;  //we can grant the time request
    }
    return (update) ? 1 : 0;
    */
}

BOOST_AUTO_TEST_CASE (pubsub_test)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

BOOST_AUTO_TEST_CASE (message_test)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

// Test create filters, publications, subscriptions, endpoints
// Test queue functions
// Test dependencies
// Test core object usage (if any) - create a dummy core for test purposes

/*
Core::federate_id_t fedID;
bool grant=false;
bool converged=false;
bool exec_requested = false;
Time Tnext=timeZero;  //!<next time computation
Time Te=timeZero;		//!< execution time computation
Time Tdemin=timeZero;	//!< min dependency event time

DependencyInfo() = default;
DependencyInfo(Core::federate_id_t id) :fedID(id) {};
*/

/*
    uint64_t getQueueSize(Core::Handle id) const;
    uint64_t getQueueSize() const;
    uint64_t getFilterQueueSize() const;
    message_t *receive(Core::Handle id);
    std::pair<Core::Handle, message_t*> receive();
    std::pair<Core::Handle, message_t*> receiveForFilter();
    bool processQueue();
    void generateKnownDependencies();
    void addDependency(Core::federate_id_t);
    void addDependent(Core::federate_id_t);

    void setCoreObject(CommonCore *parent);

    std::pair<Time, bool> requestTime(Time nextTime, bool iterationRequested);
*/

BOOST_AUTO_TEST_SUITE_END ()