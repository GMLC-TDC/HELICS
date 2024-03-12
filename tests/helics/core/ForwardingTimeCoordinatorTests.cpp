/*
Copyright (c) 2017-2024,
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
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
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
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
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
    remDep.source_id = GlobalFederateId(10);
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependencies();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, dependent_tests)
{
    ForwardingTimeCoordinator ftc;
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
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
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
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
    remDep.source_id = GlobalFederateId(10);
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
}

TEST(ftc_tests, execMode_entry)
{
    ForwardingTimeCoordinator ftc;
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    ftc.enteringExecMode();
    // MessageProcessingResult
    auto ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == MessageProcessingResult::CONTINUE_PROCESSING);

    ActionMessage execReady(CMD_EXEC_REQUEST);
    execReady.source_id = fed2;
    auto result = ftc.processTimeMessage(execReady);
    EXPECT_GE(result, helics::TimeProcessingResult::PROCESSED);
    ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == MessageProcessingResult::CONTINUE_PROCESSING);
    execReady.source_id = fed3;
    result = ftc.processTimeMessage(execReady);
    EXPECT_GE(result, helics::TimeProcessingResult::PROCESSED);
    ret = ftc.checkExecEntry();
    EXPECT_TRUE(ret == MessageProcessingResult::NEXT_STEP);
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
    EXPECT_TRUE(ret == MessageProcessingResult::NEXT_STEP);

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
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    getFTCtoExecMode(ftc);

    ftc.addDependent(GlobalFederateId(5));
    ActionMessage lastMessage(CMD_INVALID);
    ftc.setSourceId(GlobalFederateId(1));
    ftc.setMessageSender([&lastMessage](const helics::ActionMessage& mess) { lastMessage = mess; });

    ActionMessage timeUpdate(CMD_TIME_REQUEST, fed2, GlobalFederateId(1));
    timeUpdate.actionTime = 1.0;
    timeUpdate.Te = 1.0;
    timeUpdate.Tdemin = 1.0;

    auto result = ftc.processTimeMessage(timeUpdate);
    EXPECT_GE(result, helics::TimeProcessingResult::PROCESSED);
    ftc.updateTimeFactors();
    // there should still be the invalid as all federates are not updated past exec
    EXPECT_TRUE(lastMessage.action() == CMD_INVALID);

    timeUpdate.source_id = fed3;
    timeUpdate.actionTime = 0.5;
    timeUpdate.Te = 1.0;
    timeUpdate.Tdemin = 0.5;
    ftc.enteringExecMode();
    result = ftc.processTimeMessage(timeUpdate);
    EXPECT_GE(result, helics::TimeProcessingResult::PROCESSED);
    ftc.updateTimeFactors();
    EXPECT_EQ(lastMessage.actionTime, 0.5);
    EXPECT_EQ(lastMessage.Te, 1.0);
    EXPECT_EQ(lastMessage.Tdemin, 0.5);
    EXPECT_TRUE(lastMessage.action() == CMD_TIME_REQUEST);
}

TEST(ftc_tests, timing_test2)
{
    ForwardingTimeCoordinator ftc;
    GlobalFederateId fed2(2);
    GlobalFederateId fed3(3);
    ftc.addDependency(fed2);
    ftc.addDependency(fed3);
    getFTCtoExecMode(ftc);
}
