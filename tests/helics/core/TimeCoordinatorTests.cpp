/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/TimeCoordinator.hpp"
#include "helics/helics_enums.h"

#include "gtest/gtest.h"
#include <vector>

using namespace helics;

static constexpr GlobalFederateId fed2(2);
static constexpr GlobalFederateId fed3(3);
TEST(timeCoord_tests, dependency_tests)
{
    TimeCoordinator ftc;
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

TEST(timeCoord_tests, dependency_test_message)
{
    TimeCoordinator ftc;
    ActionMessage addDep(CMD_ADD_DEPENDENCY);
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

TEST(timeCoord_tests, dependent_tests)
{
    TimeCoordinator ftc;
    ftc.addDependent(fed2);
    ftc.addDependent(fed3);
    auto deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 2U);
    EXPECT_TRUE(deps[0] == fed2);
    EXPECT_TRUE(deps[1] == fed3);
    // test repeated inputs are dealt with correctly
    ftc.addDependent(fed3);
    // deps is a reference so it should change automatically
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

TEST(timeCoord_tests, dependent_test_message)
{
    TimeCoordinator ftc;

    ActionMessage addDep(CMD_ADD_DEPENDENT);
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

TEST(timeCoord_tests, max_time_request_after_peer_grant)
{
    constexpr GlobalFederateId source(1);
    std::vector<ActionMessage> requests;
    TimeCoordinator coordinator([&requests](const ActionMessage& message) {
        if (message.action() == CMD_TIME_REQUEST) {
            requests.push_back(message);
        }
    });
    coordinator.setSourceId(source);
    coordinator.setProperty(HELICS_PROPERTY_TIME_PERIOD, Time{60});
    coordinator.setProperty(HELICS_PROPERTY_TIME_OUTPUT_DELAY, Time{5});
    coordinator.addDependency(fed2);
    coordinator.addDependent(fed2);
    coordinator.addDependent(fed3);

    coordinator.enteringExecMode(IterationRequest::NO_ITERATIONS);
    coordinator.processTimeMessage(ActionMessage(CMD_EXEC_REQUEST, fed2, source));
    ASSERT_EQ(coordinator.checkExecEntry(), MessageProcessingResult::NEXT_STEP);
    coordinator.processTimeMessage(ActionMessage(CMD_EXEC_GRANT, fed2, source));

    coordinator.timeRequest(60, IterationRequest::NO_ITERATIONS, Time::maxVal(), Time::maxVal());
    ActionMessage peerGrant(CMD_TIME_GRANT, fed2, source);
    peerGrant.actionTime = 60;
    coordinator.processTimeMessage(peerGrant);
    ASSERT_EQ(coordinator.checkTimeGrant(), MessageProcessingResult::NEXT_STEP);
    ASSERT_EQ(coordinator.getGrantedTime(), Time{60});

    requests.clear();
    // The peer still advertises 60. The pending value can be granted at 120, so both
    // recipients should see 125 as the earliest time this federate can produce output.
    coordinator.timeRequest(Time::maxVal(), IterationRequest::NO_ITERATIONS, 120, Time::maxVal());
    ASSERT_EQ(requests.size(), 2U);
    EXPECT_NE(requests[0].dest_id, requests[1].dest_id);
    for (const auto& request : requests) {
        EXPECT_EQ(request.actionTime, Time{125});
        EXPECT_EQ(request.Te, Time{125});
        EXPECT_EQ(request.Tdemin, Time{125});
    }

    ActionMessage peerRequest(CMD_TIME_REQUEST, fed2, source);
    peerRequest.actionTime = 180;
    peerRequest.Te = 180;
    peerRequest.Tdemin = 180;
    coordinator.processTimeMessage(peerRequest);
    EXPECT_EQ(coordinator.checkTimeGrant(), MessageProcessingResult::NEXT_STEP);
    EXPECT_EQ(coordinator.getGrantedTime(), Time{120});
}

class TimeCoordinatorTester1: public ::testing::Test, helics::TimeCoordinator {
  public:
    void setup1()
    {
        info.timeDelta = 1.0;
        info.wait_for_current_time_updates = true;
        executionMode = true;
        time_granted = timeZero;
        ActionMessage addDep(CMD_ADD_DEPENDENT);
        addDep.source_id = fed2;

        processDependencyUpdateMessage(addDep);
    }

    helics::TimeCoordinator* getCoordinator()
    {
        return static_cast<helics::TimeCoordinator*>(this);
    }
};

TEST_F(TimeCoordinatorTester1, wait_for_current_time)
{
    setup1();
}
