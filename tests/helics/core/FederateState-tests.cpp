/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/core/EndpointInfo.hpp"
#include "helics/core/FederateState.hpp"
#include "helics/core/FilterInfo.hpp"
#include "helics/core/InputInfo.hpp"
#include "helics/core/PublicationInfo.hpp"
#include "helics/core/helics_definitions.hpp"

#include "gtest/gtest.h"
#include <future>
#include <memory>

struct federateStateTests: public ::testing::Test {
    federateStateTests():
        fs(std::make_unique<helics::FederateState>("fed_name", helics::CoreFederateInfo()))
    {
    }
    std::unique_ptr<helics::FederateState> fs;
};

TEST_F(federateStateTests, constructor_test)
{
    // Check setting of name, initial state, and info by the constructor
    EXPECT_EQ(fs->getIdentifier(), "fed_name");
    EXPECT_EQ(fs->getState(), helics::federate_state::HELICS_CREATED);

    EXPECT_EQ(fs->getTimeProperty(helics::defs::properties::time_delta), helics::Time::epsilon());
    EXPECT_EQ(fs->getTimeProperty(helics::defs::properties::output_delay), helics::Time::zeroVal());
    EXPECT_EQ(fs->getTimeProperty(helics::defs::properties::input_delay), helics::Time::zeroVal());
    EXPECT_EQ(fs->getTimeProperty(helics_property_time_period), helics::Time::zeroVal());
    EXPECT_EQ(fs->getOptionFlag(helics::defs::flags::observer), false);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::flags::uninterruptible), false);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::flags::interruptible), true);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::flags::source_only), false);

    // Check other default state values
    EXPECT_EQ(fs->getQueueSize(), 0U);
    EXPECT_EQ(fs->getEvents().size(), 0U);

    // EXPECT_EQ(fs->message_queue.size(), 0);
    // EXPECT_EQ(fs->dependencies.size(), 0);
    EXPECT_EQ(fs->getDependents().size(), 0U);
    EXPECT_TRUE(fs->local_id == helics::local_federate_id{});
    EXPECT_TRUE(fs->global_id.load() == helics::global_federate_id{});
    EXPECT_EQ(fs->init_requested, false);

    EXPECT_EQ(fs->getCurrentIteration(), 0);
    EXPECT_EQ(fs->grantedTime(), helics::Time::minVal());
    // EXPECT_EQ(fs->time_requested, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_next, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_minDe, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_minTe, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_event, helics::Time::zeroVal());
    EXPECT_EQ(fs->getIntegerProperty(helics::defs::properties::max_iterations), 50);
}

TEST_F(federateStateTests, create_input_test)
{
    using namespace helics;
    fs->interfaces().createInput(interface_handle(0), "first!", "type", "units");
    fs->interfaces().createInput(interface_handle(1), "second", "type", "units");
    fs->interfaces().createInput(interface_handle(3), "last", "type", "units");
    fs->interfaces().createInput(interface_handle(2), "cut-in-line", "type", "units");

    helics::InputInfo* info;

    // Check first subscription
    info = fs->interfaces().getInput("first!");
    EXPECT_EQ(info->id.handle, interface_handle(0));

    info = fs->interfaces().getInput(interface_handle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second subscription
    info = fs->interfaces().getInput("second");
    EXPECT_EQ(info->id.handle, interface_handle(1));

    info = fs->interfaces().getInput(interface_handle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order subscription
    info = fs->interfaces().getInput("cut-in-line");
    EXPECT_EQ(info->id.handle, interface_handle(2));

    info = fs->interfaces().getInput(interface_handle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) subscription
    info = fs->interfaces().getInput("last");
    EXPECT_EQ(info->id.handle, interface_handle(3));

    info = fs->interfaces().getInput(interface_handle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(federateStateTests, create_publication_test)
{
    fs->interfaces().createPublication(helics::interface_handle(0), "first!", "type", "units");
    fs->interfaces().createPublication(helics::interface_handle(1), "second", "type", "units");
    fs->interfaces().createPublication(helics::interface_handle(3), "last", "type", "units");
    fs->interfaces().createPublication(helics::interface_handle(2), "cut-in-line", "type", "units");

    helics::PublicationInfo* info;

    // Check first publication
    info = fs->interfaces().getPublication("first!");
    EXPECT_EQ(info->id.handle, helics::interface_handle(0));

    info = fs->interfaces().getPublication(helics::interface_handle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second publication
    info = fs->interfaces().getPublication("second");
    EXPECT_EQ(info->id.handle, helics::interface_handle(1));

    info = fs->interfaces().getPublication(helics::interface_handle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order publication
    info = fs->interfaces().getPublication("cut-in-line");
    EXPECT_EQ(info->id.handle, helics::interface_handle(2));

    info = fs->interfaces().getPublication(helics::interface_handle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) publication
    info = fs->interfaces().getPublication("last");
    EXPECT_EQ(info->id.handle, helics::interface_handle(3));

    info = fs->interfaces().getPublication(helics::interface_handle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(federateStateTests, create_endpoint_test)
{
    using namespace helics;
    fs->interfaces().createEndpoint(interface_handle(0), "first!", "type");
    fs->interfaces().createEndpoint(interface_handle(1), "second", "type");
    fs->interfaces().createEndpoint(interface_handle(3), "last", "type");
    fs->interfaces().createEndpoint(interface_handle(2), "cut-in-line", "type");

    EndpointInfo* info;

    // Check first endpoint
    info = fs->interfaces().getEndpoint("first!");
    EXPECT_EQ(info->id.handle, interface_handle(0));

    info = fs->interfaces().getEndpoint(interface_handle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second endpoint
    info = fs->interfaces().getEndpoint("second");
    EXPECT_EQ(info->id.handle, interface_handle(1));

    info = fs->interfaces().getEndpoint(interface_handle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order endpoint
    info = fs->interfaces().getEndpoint("cut-in-line");
    EXPECT_EQ(info->id.handle, interface_handle(2));

    info = fs->interfaces().getEndpoint(interface_handle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) endpoint
    info = fs->interfaces().getEndpoint("last");
    EXPECT_EQ(info->id.handle, interface_handle(3));

    info = fs->interfaces().getEndpoint(interface_handle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(federateStateTests, basic_processmessage_test)
{
    using namespace helics;
    ActionMessage cmd;

    // Test returning when the initialization state is entered
    cmd.setAction(helics::CMD_INIT_GRANT);
    auto fs_process = std::async(std::launch::async, [&]() { return fs->enterInitializingMode(); });
    EXPECT_EQ(fs->getState(), federate_state::HELICS_CREATED);
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == iteration_result::next_step);
    EXPECT_EQ(fs->getState(), federate_state::HELICS_INITIALIZING);

    // Test returning when the finished state is entered
    cmd.setAction(helics::CMD_STOP);
    fs->addAction(cmd);
    EXPECT_EQ(fs->getState(), federate_state::HELICS_INITIALIZING);
    auto fs_process2 = std::async(std::launch::async, [&]() {
        return fs->enterExecutingMode(iteration_request::no_iterations);
    });

    fs->global_id =
        global_federate_id(0);  // if it doesn't match the id in the command, this will hang
    fs_process2.wait();
    fs->global_id = helics::global_federate_id();
    auto state = fs_process2.get();

    EXPECT_TRUE(state == iteration_result::halted);
    EXPECT_EQ(fs->getState(), federate_state::HELICS_FINISHED);

    // Return to created state
    fs->reset();

    // Test CMD_FED_ACK message when no error
    cmd.setAction(helics::CMD_FED_ACK);
    global_federate_id fed22(22);
    cmd.dest_id = fed22;
    cmd.name = "fed_name";
    clearActionFlag(cmd, error_flag);
    fs_process = std::async(std::launch::async, [&]() { return fs->waitSetup(); });
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == iteration_result::next_step);
    EXPECT_EQ(fs->global_id.load(), fed22);

    // Test CMD_FED_ACK message with an error
    cmd.setAction(helics::CMD_FED_ACK);
    cmd.dest_id = global_federate_id(23);
    setActionFlag(cmd, error_flag);
    fs_process = std::async(std::launch::async, [&]() { return fs->waitSetup(); });
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == iteration_result::error);
    EXPECT_EQ(fs->global_id.load(), fed22);
    EXPECT_EQ(fs->getState(), federate_state::HELICS_ERROR);

    // Return to initializing state
    fs->reInit();

    // Test returning when an error occurs
    cmd.setAction(helics::CMD_ERROR);
    fs_process2 = std::async(std::launch::async, [&]() {
        return fs->enterExecutingMode(iteration_request::no_iterations);
    });
    auto st = fs->getState();
    EXPECT_TRUE((st == federate_state::HELICS_INITIALIZING) ||
                (st == federate_state::HELICS_EXECUTING));
    fs->addAction(cmd);
    auto res = fs_process2.get();
    if (res != iteration_result::error) {
        auto ittime = fs->requestTime(5.0, helics::iteration_request::no_iterations);
        res = ittime.state;
    }

    EXPECT_TRUE(res == iteration_result::error);
    EXPECT_EQ(fs->getState(), federate_state::HELICS_ERROR);

    fs->reset();

    // Test CMD_EXEC_REQUEST/CMD_EXEC_GRANT returns 0 if dependencies/dependents aren't done;
    // returns 1 if fs->iterating is true, 2 otherwise; if 1 ret false, if 2 ret true
    cmd.setAction(helics::CMD_EXEC_GRANT);

    // Test CMD_TIME_REQUEST/CMD_TIME_GRANT
    cmd.setAction(helics::CMD_TIME_GRANT);

    /* CMD_TIME_REQUEST and CMD_TIME_GRANT; manipulate time factors in ways to ensure behavior is
    correct for deciding event to allow or not; test based on desired functionality time_next and
    time_minDe compared against everything else Time Tallow(std::max(time_next, time_minDe)); if
    (time_event <= Tallow)
    {
        return 2;  //we can grant the time request
    }
    return (update) ? 1 : 0;
    */
}

TEST_F(federateStateTests, pubsub_test)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

TEST_F(federateStateTests, message_test)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

// Test create filters, publications, subscriptions, endpoints
// Test queue functions
// Test dependencies
// Test core object usage (if any) - create a dummy core for test purposes

/*
Core::local_federate_id fedID;
bool grant=false;
bool converged=false;
bool exec_requested = false;
Time Tnext=timeZero;  //!<next time computation
Time Te=timeZero;        //!< execution time computation
Time Tdemin=timeZero;    //!< min dependency event time

DependencyInfo() = default;
DependencyInfo(Core::local_federate_id id) :fedID(id) {};
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
    void addDependency(Core::local_federate_id);
    void addDependent(Core::local_federate_id);

    void setCoreObject(CommonCore *parent);

    std::pair<Time, bool> requestTime(Time nextTime, bool iterationRequested);
*/
