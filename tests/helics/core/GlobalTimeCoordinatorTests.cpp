/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/GlobalTimeCoordinator.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <vector>

using namespace helics;

TEST(global_time_coord_tests, execMode_mixed_iterative_and_non_iterative_dependencies)
{
    GlobalTimeCoordinator gtc;
    const GlobalFederateId coordinator(1);
    const GlobalFederateId iterativeFed(2);
    const GlobalFederateId nonIterativeFed(3);

    gtc.setSourceId(coordinator);
    gtc.addDependency(iterativeFed);
    gtc.addDependent(iterativeFed);
    gtc.setAsChild(iterativeFed);
    gtc.addDependency(nonIterativeFed);
    gtc.addDependent(nonIterativeFed);
    gtc.setAsChild(nonIterativeFed);
    gtc.enteringExecMode();

    ActionMessage iterativeRequest(CMD_EXEC_REQUEST, iterativeFed, coordinator);
    setIterationFlags(iterativeRequest, IterationRequest::ITERATE_IF_NEEDED);
    iterativeRequest.counter = 1;
    iterativeRequest.setExtraData(coordinator.baseValue());
    iterativeRequest.setExtraDestData(0);
    EXPECT_GE(gtc.processTimeMessage(iterativeRequest), TimeProcessingResult::PROCESSED);

    ActionMessage nonIterativeRequest(CMD_EXEC_REQUEST, nonIterativeFed, coordinator);
    EXPECT_GE(gtc.processTimeMessage(nonIterativeRequest), TimeProcessingResult::PROCESSED);

    gtc.updateTimeFactors();
    EXPECT_EQ(gtc.checkExecEntry(), MessageProcessingResult::NEXT_STEP);
}

TEST(global_time_coord_tests, iterative_runtime_states_start_time_update)
{
    for (const auto iterationRequest :
         {IterationRequest::ITERATE_IF_NEEDED, IterationRequest::FORCE_ITERATION}) {
        GlobalTimeCoordinator gtc;
        const GlobalFederateId coordinator(1);
        const GlobalFederateId federate(2);
        std::vector<ActionMessage> sentMessages;

        gtc.setSourceId(coordinator);
        gtc.setMessageSender(
            [&sentMessages](const ActionMessage& message) { sentMessages.push_back(message); });
        gtc.addDependency(federate);
        gtc.addDependent(federate);
        gtc.setAsChild(federate);
        gtc.enteringExecMode();

        ActionMessage execRequest(CMD_EXEC_REQUEST, federate, coordinator);
        EXPECT_GE(gtc.processTimeMessage(execRequest), TimeProcessingResult::PROCESSED);
        EXPECT_EQ(gtc.checkExecEntry(), MessageProcessingResult::NEXT_STEP);
        sentMessages.clear();

        // A delayed execution request must not move an executing coordinator back into an
        // initialization state and prevent the next time request from starting its handshake.
        EXPECT_GE(gtc.processTimeMessage(execRequest), TimeProcessingResult::PROCESSED);
        EXPECT_TRUE(gtc.updateTimeFactors());

        ActionMessage timeRequest(CMD_TIME_REQUEST, federate, coordinator);
        setIterationFlags(timeRequest, iterationRequest);
        timeRequest.actionTime = 1.0;
        timeRequest.Te = 1.0;
        timeRequest.Tdemin = 1.0;
        timeRequest.counter = 1;
        EXPECT_GE(gtc.processTimeMessage(timeRequest), TimeProcessingResult::PROCESSED);

        EXPECT_TRUE(gtc.updateTimeFactors());
        EXPECT_TRUE(std::ranges::any_of(sentMessages, [](const auto& message) {
            return message.action() == CMD_REQUEST_CURRENT_TIME;
        }));
    }
}
