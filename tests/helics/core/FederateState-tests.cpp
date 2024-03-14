/*
Copyright (c) 2017-2024,
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

struct FederateStateTests: public ::testing::Test {
    FederateStateTests():
        fs(std::make_unique<helics::FederateState>("fed_name", helics::CoreFederateInfo()))
    {
    }
    std::unique_ptr<helics::FederateState> fs;
};

TEST_F(FederateStateTests, constructor)
{
    // Check setting of name, initial state, and info by the constructor
    EXPECT_EQ(fs->getIdentifier(), "fed_name");
    EXPECT_EQ(fs->getState(), helics::FederateStates::CREATED);

    EXPECT_EQ(fs->getTimeProperty(helics::defs::Properties::TIME_DELTA), helics::Time::epsilon());
    EXPECT_EQ(fs->getTimeProperty(helics::defs::Properties::OUTPUT_DELAY), helics::Time::zeroVal());
    EXPECT_EQ(fs->getTimeProperty(helics::defs::Properties::INPUT_DELAY), helics::Time::zeroVal());
    EXPECT_EQ(fs->getTimeProperty(HELICS_PROPERTY_TIME_PERIOD), helics::Time::zeroVal());
    EXPECT_EQ(fs->getOptionFlag(helics::defs::Flags::OBSERVER), false);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::Flags::UNINTERRUPTIBLE), false);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::Flags::INTERRUPTIBLE), true);
    EXPECT_EQ(fs->getOptionFlag(helics::defs::Flags::SOURCE_ONLY), false);

    // Check other default state values
    EXPECT_EQ(fs->getQueueSize(), 0U);
    EXPECT_EQ(fs->getEvents().size(), 0U);

    // EXPECT_EQ(fs->message_queue.size(), 0);
    // EXPECT_EQ(fs->dependencies.size(), 0);
    EXPECT_EQ(fs->getDependents().size(), 0U);
    EXPECT_TRUE(fs->local_id == helics::LocalFederateId{});
    EXPECT_TRUE(fs->global_id.load() == helics::GlobalFederateId{});
    EXPECT_EQ(fs->initRequested, false);

    EXPECT_EQ(fs->getCurrentIteration(), 0);
    EXPECT_EQ(fs->grantedTime(), helics::Time::minVal());
    // EXPECT_EQ(fs->time_requested, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_next, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_minDe, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_minTe, helics::Time::zeroVal());
    // EXPECT_EQ(fs->time_event, helics::Time::zeroVal());
    EXPECT_EQ(fs->getIntegerProperty(helics::defs::Properties::MAX_ITERATIONS), 50);
}

TEST_F(FederateStateTests, create_input)
{
    using namespace helics;
    fs->interfaces().createInput(InterfaceHandle(0), "first!", "type", "units", 0);
    fs->interfaces().createInput(InterfaceHandle(1), "second", "type", "units", 0);
    fs->interfaces().createInput(InterfaceHandle(3), "last", "type", "units", 0);
    fs->interfaces().createInput(InterfaceHandle(2), "cut-in-line", "type", "units", 0);

    helics::InputInfo* info;

    // Check first subscription
    info = fs->interfaces().getInput("first!");
    EXPECT_EQ(info->id.handle, InterfaceHandle(0));

    info = fs->interfaces().getInput(InterfaceHandle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second subscription
    info = fs->interfaces().getInput("second");
    EXPECT_EQ(info->id.handle, InterfaceHandle(1));

    info = fs->interfaces().getInput(InterfaceHandle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order subscription
    info = fs->interfaces().getInput("cut-in-line");
    EXPECT_EQ(info->id.handle, InterfaceHandle(2));

    info = fs->interfaces().getInput(InterfaceHandle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) subscription
    info = fs->interfaces().getInput("last");
    EXPECT_EQ(info->id.handle, InterfaceHandle(3));

    info = fs->interfaces().getInput(InterfaceHandle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(FederateStateTests, create_publication)
{
    fs->interfaces().createPublication(helics::InterfaceHandle(0), "first!", "type", "units", 0);
    fs->interfaces().createPublication(helics::InterfaceHandle(1), "second", "type", "units", 0);
    fs->interfaces().createPublication(helics::InterfaceHandle(3), "last", "type", "units", 0);
    fs->interfaces().createPublication(
        helics::InterfaceHandle(2), "cut-in-line", "type", "units", 0);

    helics::PublicationInfo* info;

    // Check first publication
    info = fs->interfaces().getPublication("first!");
    EXPECT_EQ(info->id.handle, helics::InterfaceHandle(0));

    info = fs->interfaces().getPublication(helics::InterfaceHandle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second publication
    info = fs->interfaces().getPublication("second");
    EXPECT_EQ(info->id.handle, helics::InterfaceHandle(1));

    info = fs->interfaces().getPublication(helics::InterfaceHandle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order publication
    info = fs->interfaces().getPublication("cut-in-line");
    EXPECT_EQ(info->id.handle, helics::InterfaceHandle(2));

    info = fs->interfaces().getPublication(helics::InterfaceHandle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) publication
    info = fs->interfaces().getPublication("last");
    EXPECT_EQ(info->id.handle, helics::InterfaceHandle(3));

    info = fs->interfaces().getPublication(helics::InterfaceHandle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(FederateStateTests, create_endpoint)
{
    using namespace helics;
    fs->interfaces().createEndpoint(InterfaceHandle(0), "first!", "type", 0);
    fs->interfaces().createEndpoint(InterfaceHandle(1), "second", "type", 0);
    fs->interfaces().createEndpoint(InterfaceHandle(3), "last", "type", 0);
    fs->interfaces().createEndpoint(InterfaceHandle(2), "cut-in-line", "type", 0);

    EndpointInfo* info;

    // Check first endpoint
    info = fs->interfaces().getEndpoint("first!");
    EXPECT_EQ(info->id.handle, InterfaceHandle(0));

    info = fs->interfaces().getEndpoint(InterfaceHandle(0));
    EXPECT_EQ(info->key, "first!");

    // Check second endpoint
    info = fs->interfaces().getEndpoint("second");
    EXPECT_EQ(info->id.handle, InterfaceHandle(1));

    info = fs->interfaces().getEndpoint(InterfaceHandle(1));
    EXPECT_EQ(info->key, "second");

    // Check the out of order endpoint
    info = fs->interfaces().getEndpoint("cut-in-line");
    EXPECT_EQ(info->id.handle, InterfaceHandle(2));

    info = fs->interfaces().getEndpoint(InterfaceHandle(2));
    EXPECT_EQ(info->key, "cut-in-line");

    // Check the displaced (last) endpoint
    info = fs->interfaces().getEndpoint("last");
    EXPECT_EQ(info->id.handle, InterfaceHandle(3));

    info = fs->interfaces().getEndpoint(InterfaceHandle(3));
    EXPECT_EQ(info->key, "last");
}

TEST_F(FederateStateTests, basic_processmessage)
{
    using namespace helics;
    ActionMessage cmd;

    // Test returning when the initialization state is entered
    cmd.setAction(helics::CMD_INIT_GRANT);
    auto fs_process = std::async(std::launch::async, [&]() {
        return fs->enterInitializingMode(IterationRequest::NO_ITERATIONS);
    });
    EXPECT_EQ(fs->getState(), FederateStates::CREATED);
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == IterationResult::NEXT_STEP);
    EXPECT_EQ(fs->getState(), FederateStates::INITIALIZING);

    // Test returning when the finished state is entered
    cmd.setAction(helics::CMD_STOP);
    fs->addAction(cmd);
    EXPECT_EQ(fs->getState(), FederateStates::INITIALIZING);
    auto fs_process2 = std::async(std::launch::async, [&]() {
        return fs->enterExecutingMode(IterationRequest::NO_ITERATIONS);
    });

    fs->global_id =
        GlobalFederateId(0);  // if it doesn't match the id in the command, this will hang
    fs_process2.wait();
    fs->global_id = helics::GlobalFederateId();
    auto state = fs_process2.get();

    EXPECT_TRUE(state.state == IterationResult::HALTED);
    EXPECT_EQ(fs->getState(), FederateStates::FINISHED);

    // Return to created state
    fs->reset(CoreFederateInfo());

    // Test CMD_FED_ACK message when no error
    cmd.setAction(helics::CMD_FED_ACK);
    GlobalFederateId fed22(22);
    cmd.dest_id = fed22;
    cmd.name("fed_name");
    clearActionFlag(cmd, error_flag);
    fs_process = std::async(std::launch::async, [&]() { return fs->waitSetup(); });
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == IterationResult::NEXT_STEP);
    EXPECT_EQ(fs->global_id.load(), fed22);

    // Test CMD_FED_ACK message with an error
    cmd.setAction(helics::CMD_FED_ACK);
    cmd.dest_id = GlobalFederateId(23);
    setActionFlag(cmd, error_flag);
    fs_process = std::async(std::launch::async, [&]() { return fs->waitSetup(); });
    fs->addAction(cmd);
    fs_process.wait();
    EXPECT_TRUE(fs_process.get() == IterationResult::ERROR_RESULT);
    EXPECT_EQ(fs->global_id.load(), fed22);
    EXPECT_EQ(fs->getState(), FederateStates::ERRORED);

    // Return to initializing state
    fs->reset(CoreFederateInfo());

    // Test returning when an error occurs
    cmd.setAction(helics::CMD_ERROR);
    fs_process2 = std::async(std::launch::async, [&]() {
        return fs->enterExecutingMode(IterationRequest::NO_ITERATIONS);
    });
    auto currentState = fs->getState();
    EXPECT_EQ(currentState, FederateStates::CREATED);
    fs->addAction(cmd);
    auto res = fs_process2.get();
    if (res.state != IterationResult::ERROR_RESULT) {
        auto ittime = fs->requestTime(5.0, helics::IterationRequest::NO_ITERATIONS);
        res.state = ittime.state;
    }

    EXPECT_TRUE(res.state == IterationResult::ERROR_RESULT);
    EXPECT_EQ(fs->getState(), FederateStates::ERRORED);

    fs->reset(CoreFederateInfo());

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

TEST_F(FederateStateTests, pubsub)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

TEST_F(FederateStateTests, message)
{
    // auto fs_process = std::async(std::launch::async, [&]() { return fs->processQueue(); });
}

// Test create filters, publications, subscriptions, endpoints
// Test queue functions
// Test dependencies
// Test core object usage (if any) - create a dummy core for test purposes

/*
Core::LocalFederateId fedID;
bool grant=false;
bool converged=false;
bool exec_requested = false;
Time Tnext=timeZero;  //!<next time computation
Time Te=timeZero;        //!< execution time computation
Time Tdemin=timeZero;    //!< min dependency event time

DependencyInfo() = default;
DependencyInfo(Core::LocalFederateId id) :fedID(id) {};
*/

/*
    uint64_t getQueueSize(InterfaceHandle id) const;
    uint64_t getQueueSize() const;
    uint64_t getFilterQueueSize() const;
    HelicsMessage *receive(InterfaceHandle id);
    std::pair<InterfaceHandle, HelicsMessage*> receive();
    std::pair<InterfaceHandle, HelicsMessage*> receiveForFilter();
    bool processQueue();
    void generateKnownDependencies();
    void addDependency(Core::LocalFederateId);
    void addDependent(Core::LocalFederateId);

    void setCoreObject(CommonCore *parent);

    std::pair<Time, bool> requestTime(Time nextTime, bool iterationRequested);
*/
