/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/ForwardingTimeCoordinator.hpp"

#include "gtest/gtest.h"

using namespace helics;

TEST(ftc_tests, dependency_tests)
{
    ForwardingTimeCoordinator ftc;
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    auto deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);
    // test repeated inputs are dealt with correctly
    ftc.addDependency(fed3);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);

    ftc.removeDependency(fed2);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
    // remove same one
    ftc.removeDependency(fed2);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, dependency_test_message)
{
    ForwardingTimeCoordinator ftc;
    ActionMessage addDep(CMD_ADD_DEPENDENCY);
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    addDep.source_id = fed2;

    ftc.processDependencyUpdateMessage(addDep);
    addDep.source_id = fed3;
    ftc.processDependencyUpdateMessage(addDep);
    auto deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);
    // test redundancy checking
    ftc.processDependencyUpdateMessage(addDep);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);

    ActionMessage remDep(CMD_REMOVE_DEPENDENCY);
    remDep.source_id = fed2;
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);

    // remove unrecognized one
    remDep.source_id = global_federate_id(10);
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, dependent_tests)
{
    ForwardingTimeCoordinator ftc;
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    ftc.addDependent(fed2);
    ftc.addDependent(fed3);
    auto deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);
    // test repeated inputs are dealt with correctly
    ftc.addDependent(fed3);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);

    ftc.removeDependent(fed2);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
    // remove same one
    ftc.removeDependent(fed2);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, dependent_test_message)
{
    ForwardingTimeCoordinator ftc;
    ActionMessage addDep(CMD_ADD_DEPENDENT);
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    addDep.source_id = fed2;

    ftc.processDependencyUpdateMessage(addDep);
    addDep.source_id = fed3;
    ftc.processDependencyUpdateMessage(addDep);
    auto deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);
    // test redundancy checking
    ftc.processDependencyUpdateMessage(addDep);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);

    ActionMessage remDep(CMD_REMOVE_DEPENDENT);
    remDep.source_id = fed2;
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);

    // remove unrecognized one
    remDep.source_id = global_federate_id(10);
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, execMode_entry)
{
    ForwardingTimeCoordinator ftc;
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    ftc.enteringExecMode();
    // message_processing_result
    auto ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == message_processing_result::continue_processing);

    ActionMessage execReady(CMD_EXEC_REQUEST);
    execReady.source_id = fed2;
    auto modified = ftc.processTimeMessage(execReady);
    EXPECT_TRUE(modified);
    ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == message_processing_result::continue_processing);
    execReady.source_id = fed3;
    modified = ftc.processTimeMessage(execReady);
    EXPECT_TRUE(modified);
    ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == message_processing_result::next_step);
}

void getFTCtoExecMode(ForwardingTimeCoordinator& ftc)
{
    ActionMessage execReady(CMD_EXEC_REQUEST);
    auto depList = ftc.getDependencies();
    for (auto dep : depList) {
        execReady.source_id = dep;
        ftc.processTimeMessage(execReady);
    }
    auto ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == message_processing_result::next_step);

    ActionMessage execGrant(CMD_EXEC_GRANT);
    for (auto dep : depList) {
        execReady.source_id = dep;
        ftc.processTimeMessage(execReady);
    }
    ftc.updateTimeFactors();
}

TEST(ftc_tests, timing_test1)
{
    ForwardingTimeCoordinator ftc;
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    getFTCtoExecMode(ftc);

    ftc.addDependent(global_federate_id(5));
    ActionMessage lastMessage(CMD_INVALID);
    ftc.source_id = global_federate_id(1);
    ftc.setMessageSender([&lastMessage](const helics::ActionMessage& mess) { lastMessage = mess; });

    ActionMessage timeUpdate(CMD_TIME_REQUEST, fed2, global_federate_id(1));
    timeUpdate.actionTime = 1.0;
    timeUpdate.Te = 1.0;
    timeUpdate.Tdemin = 1.0;

    auto modified = ftc.processTimeMessage(timeUpdate);
    EXPECT_TRUE(modified);
    ftc.updateTimeFactors();
    // there should be no update yet
    EXPECT_TRUE(lastMessage.action() == CMD_TIME_REQUEST);

    timeUpdate.source_id = fed3;
    timeUpdate.actionTime = 0.5;
    timeUpdate.Te = 1.0;
    timeUpdate.Tdemin = 0.5;
    modified = ftc.processTimeMessage(timeUpdate);
    EXPECT_TRUE(modified);
    ftc.updateTimeFactors();
    EXPECT_EQ(lastMessage.actionTime, 0.5);
    EXPECT_EQ(lastMessage.Te, 1.0);
    EXPECT_EQ(lastMessage.Tdemin, 0.5);
    EXPECT_TRUE(lastMessage.action() == CMD_TIME_REQUEST);
}

TEST(ftc_tests, timing_test2)
{
    ForwardingTimeCoordinator ftc;
    global_federate_id fed2(2);
    global_federate_id fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    getFTCtoExecMode(ftc);
}
