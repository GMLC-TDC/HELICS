/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include <future>

#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FederateState.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/NamedInputInfo.hpp"
#include "helics/core/PublicationInfo.hpp"
#include "helics/core/helics_definitions.hpp"

namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (FederateState_tests, federateStateTestFixture, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (constructor_test)
{
    // Check setting of name, initial state, and info by the constructor
    BOOST_CHECK_EQUAL (fs->getIdentifier (), "fed_name");
    BOOST_CHECK_EQUAL (fs->getState (), helics::federate_state_t::HELICS_CREATED);

    BOOST_CHECK_EQUAL (fs->getTimeProperty (helics::defs::properties::time_delta), helics::Time::epsilon ());
    BOOST_CHECK_EQUAL (fs->getTimeProperty (helics::defs::properties::output_delay), helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getTimeProperty (helics::defs::properties::input_delay), helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getTimeProperty (helics_property_time_period), helics::Time::zeroVal ());
    BOOST_CHECK_EQUAL (fs->getOptionFlag (helics::defs::flags::observer), false);
    BOOST_CHECK_EQUAL (fs->getOptionFlag (helics::defs::flags::uninterruptible), false);
    BOOST_CHECK_EQUAL (fs->getOptionFlag (helics::defs::flags::interruptible), true);
    BOOST_CHECK_EQUAL (fs->getOptionFlag (helics::defs::flags::source_only), false);

    // Check other default state values
    BOOST_CHECK_EQUAL (fs->getQueueSize (), 0);
    BOOST_CHECK_EQUAL (fs->getEvents ().size (), 0);

    // BOOST_CHECK_EQUAL(fs->message_queue.size(), 0);
    // BOOST_CHECK_EQUAL(fs->dependencies.size(), 0);
    BOOST_CHECK_EQUAL (fs->getDependents ().size (), 0);
    BOOST_CHECK (fs->local_id == helics::federate_id_t ());
    BOOST_CHECK (fs->global_id.load () == helics::global_federate_id ());
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
    BOOST_CHECK_EQUAL (fs->getIntegerProperty (helics::defs::properties::max_iterations), 50);
}

BOOST_AUTO_TEST_CASE (create_input_test)
{
    using namespace helics;
    fs->interfaces ().createInput (interface_handle (0), "first!", "type", "units");
    fs->interfaces ().createInput (interface_handle (1), "second", "type", "units");
    fs->interfaces ().createInput (interface_handle (3), "last", "type", "units");
    fs->interfaces ().createInput (interface_handle (2), "cut-in-line", "type", "units");

    helics::NamedInputInfo *info;

    // Check first subscription
    info = fs->interfaces ().getInput ("first!");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (0));

    info = fs->interfaces ().getInput (interface_handle (0));
    BOOST_CHECK_EQUAL (info->key, "first!");

    // Check second subscription
    info = fs->interfaces ().getInput ("second");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (1));

    info = fs->interfaces ().getInput (interface_handle (1));
    BOOST_CHECK_EQUAL (info->key, "second");

    // Check the out of order subscription
    info = fs->interfaces ().getInput ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (2));

    info = fs->interfaces ().getInput (interface_handle (2));
    BOOST_CHECK_EQUAL (info->key, "cut-in-line");

    // Check the displaced (last) subscription
    info = fs->interfaces ().getInput ("last");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (3));

    info = fs->interfaces ().getInput (interface_handle (3));
    BOOST_CHECK_EQUAL (info->key, "last");
}

BOOST_AUTO_TEST_CASE (create_publication_test)
{
    fs->interfaces ().createPublication (helics::interface_handle (0), "first!", "type", "units");
    fs->interfaces ().createPublication (helics::interface_handle (1), "second", "type", "units");
    fs->interfaces ().createPublication (helics::interface_handle (3), "last", "type", "units");
    fs->interfaces ().createPublication (helics::interface_handle (2), "cut-in-line", "type", "units");

    helics::PublicationInfo *info;

    // Check first publication
    info = fs->interfaces ().getPublication ("first!");
    BOOST_CHECK_EQUAL (info->id.handle, helics::interface_handle (0));

    info = fs->interfaces ().getPublication (helics::interface_handle (0));
    BOOST_CHECK_EQUAL (info->key, "first!");

    // Check second publication
    info = fs->interfaces ().getPublication ("second");
    BOOST_CHECK_EQUAL (info->id.handle, helics::interface_handle (1));

    info = fs->interfaces ().getPublication (helics::interface_handle (1));
    BOOST_CHECK_EQUAL (info->key, "second");

    // Check the out of order publication
    info = fs->interfaces ().getPublication ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id.handle, helics::interface_handle (2));

    info = fs->interfaces ().getPublication (helics::interface_handle (2));
    BOOST_CHECK_EQUAL (info->key, "cut-in-line");

    // Check the displaced (last) publication
    info = fs->interfaces ().getPublication ("last");
    BOOST_CHECK_EQUAL (info->id.handle, helics::interface_handle (3));

    info = fs->interfaces ().getPublication (helics::interface_handle (3));
    BOOST_CHECK_EQUAL (info->key, "last");
}

BOOST_AUTO_TEST_CASE (create_endpoint_test)
{
    using namespace helics;
    fs->interfaces ().createEndpoint (interface_handle (0), "first!", "type");
    fs->interfaces ().createEndpoint (interface_handle (1), "second", "type");
    fs->interfaces ().createEndpoint (interface_handle (3), "last", "type");
    fs->interfaces ().createEndpoint (interface_handle (2), "cut-in-line", "type");

    EndpointInfo *info;

    // Check first endpoint
    info = fs->interfaces ().getEndpoint ("first!");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (0));

    info = fs->interfaces ().getEndpoint (interface_handle (0));
    BOOST_CHECK_EQUAL (info->key, "first!");

    // Check second endpoint
    info = fs->interfaces ().getEndpoint ("second");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (1));

    info = fs->interfaces ().getEndpoint (interface_handle (1));
    BOOST_CHECK_EQUAL (info->key, "second");

    // Check the out of order endpoint
    info = fs->interfaces ().getEndpoint ("cut-in-line");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (2));

    info = fs->interfaces ().getEndpoint (interface_handle (2));
    BOOST_CHECK_EQUAL (info->key, "cut-in-line");

    // Check the displaced (last) endpoint
    info = fs->interfaces ().getEndpoint ("last");
    BOOST_CHECK_EQUAL (info->id.handle, interface_handle (3));

    info = fs->interfaces ().getEndpoint (interface_handle (3));
    BOOST_CHECK_EQUAL (info->key, "last");
}

BOOST_AUTO_TEST_CASE (basic_processmessage_test)
{
    using namespace helics;
    ActionMessage cmd;

    // Test returning when the initialization state is entered
    cmd.setAction (helics::CMD_INIT_GRANT);
    auto fs_process = std::async (std::launch::async, [&]() { return fs->enterInitializingMode (); });
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_CREATED);
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::next_step);
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_INITIALIZING);

    // Test returning when the finished state is entered
    cmd.setAction (helics::CMD_STOP);
    fs->addAction (cmd);
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_INITIALIZING);
    auto fs_process2 =
      std::async (std::launch::async, [&]() { return fs->enterExecutingMode (iteration_request::no_iterations); });

    fs->global_id = global_federate_id (0);  // if it doesn't match the id in the command, this will hang
    fs_process2.wait ();
    fs->global_id = helics::global_federate_id ();
    auto state = fs_process2.get ();

    BOOST_CHECK (state == iteration_result::halted);
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_FINISHED);

    // Return to created state
    fs->reset ();

    // Test CMD_FED_ACK message when no error
    cmd.setAction (helics::CMD_FED_ACK);
    global_federate_id fed22 (22);
    cmd.dest_id = fed22;
    cmd.name = "fed_name";
    clearActionFlag (cmd, error_flag);
    fs_process = std::async (std::launch::async, [&]() { return fs->waitSetup (); });
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::next_step);
    BOOST_CHECK_EQUAL (fs->global_id.load (), fed22);

    // Test CMD_FED_ACK message with an error
    cmd.setAction (helics::CMD_FED_ACK);
    cmd.dest_id = global_federate_id (23);
    setActionFlag (cmd, error_flag);
    fs_process = std::async (std::launch::async, [&]() { return fs->waitSetup (); });
    fs->addAction (cmd);
    fs_process.wait ();
    BOOST_CHECK (fs_process.get () == iteration_result::error);
    BOOST_CHECK_EQUAL (fs->global_id.load (), fed22);
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_ERROR);

    // Return to initializing state
    fs->reInit ();

    // Test returning when an error occurs
    cmd.setAction (helics::CMD_ERROR);
    fs_process2 =
      std::async (std::launch::async, [&]() { return fs->enterExecutingMode (iteration_request::no_iterations); });
    auto st = fs->getState ();
    BOOST_CHECK ((st == federate_state_t::HELICS_INITIALIZING) || (st == federate_state_t::HELICS_EXECUTING));
    fs->addAction (cmd);
    auto res = fs_process2.get ();
    if (res != iteration_result::error)
    {
        auto ittime = fs->requestTime (5.0, helics::iteration_request::no_iterations);
        res = ittime.state;
    }

    BOOST_CHECK (res == iteration_result::error);
    BOOST_CHECK_EQUAL (fs->getState (), federate_state_t::HELICS_ERROR);

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
    uint64_t getQueueSize(interface_handle id) const;
    uint64_t getQueueSize() const;
    uint64_t getFilterQueueSize() const;
    helics_message *receive(interface_handle id);
    std::pair<interface_handle, helics_message*> receive();
    std::pair<interface_handle, helics_message*> receiveForFilter();
    bool processQueue();
    void generateKnownDependencies();
    void addDependency(Core::federate_id_t);
    void addDependent(Core::federate_id_t);

    void setCoreObject(CommonCore *parent);

    std::pair<Time, bool> requestTime(Time nextTime, bool iterationRequested);
*/

BOOST_AUTO_TEST_SUITE_END ()
