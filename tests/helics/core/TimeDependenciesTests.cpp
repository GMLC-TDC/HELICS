/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/TimeDependencies.hpp"

#include "gtest/gtest.h"
#include <utility>
#include <vector>

using namespace helics;

TEST(timeDep_tests, dependency_tests)
{
    std::vector<DependencyInfo> deps;
    deps.resize(2);
    deps[0].connection = ConnectionType::CHILD;
    deps[0].fedID = GlobalFederateId{131073};
    deps[0].mTimeState = TimeState::time_requested;
    deps[0].dependent = true;
    deps[0].dependency = true;
    deps[0].forwarding = false;
    deps[0].next = 2.0;
    deps[0].Te = 2.0;
    deps[0].minDe = 2.0;

    deps[1].connection = ConnectionType::CHILD;
    deps[1].fedID = GlobalFederateId{1879048192};
    deps[1].mTimeState = TimeState::time_requested;
    deps[1].dependent = true;
    deps[1].dependency = true;
    deps[1].forwarding = true;
    deps[1].next = 1e-9;
    deps[1].Te = Time::maxVal();
    deps[1].minDe = Time::maxVal();

    TimeDependencies depTest;
    depTest.setDependencyVector(deps);
    auto total = generateMinTimeTotal(depTest, false, GlobalFederateId{1}, GlobalFederateId{}, 0);
    EXPECT_EQ(total.next, 2.0);
}

TEST(timeDep_tests, equal_time_iteration_state_precedence)
{
    auto generateState = [](TimeState firstState, TimeState secondState) {
        std::vector<DependencyInfo> deps;
        deps.reserve(2);
        for (const auto& [fedID, state] : {std::pair{GlobalFederateId{2}, firstState},
                                           std::pair{GlobalFederateId{3}, secondState}}) {
            DependencyInfo dep(fedID);
            dep.connection = ConnectionType::CHILD;
            dep.dependency = true;
            dep.next = 1.0;
            dep.Te = 1.0;
            dep.minDe = 1.0;
            dep.mTimeState = state;
            deps.push_back(dep);
        }

        TimeDependencies timeDependencies;
        timeDependencies.setDependencyVector(deps);
        return generateMinTimeUpstream(
                   timeDependencies, false, GlobalFederateId{1}, GlobalFederateId{}, 0)
            .mTimeState;
    };

    // Iteration requirements at the same time must not depend on federate ID ordering.
    EXPECT_EQ(generateState(TimeState::time_requested, TimeState::time_requested_iterative),
              TimeState::time_requested_iterative);
    EXPECT_EQ(generateState(TimeState::time_requested_iterative, TimeState::time_requested),
              TimeState::time_requested_iterative);

    // A required iteration has stronger precedence than an optional iteration.
    EXPECT_EQ(generateState(TimeState::time_requested_iterative,
                            TimeState::time_requested_require_iteration),
              TimeState::time_requested_require_iteration);
    EXPECT_EQ(generateState(TimeState::time_requested_require_iteration,
                            TimeState::time_requested_iterative),
              TimeState::time_requested_require_iteration);

    // A completed peer must not mask an equal-time request that still requires iteration.
    EXPECT_EQ(generateState(TimeState::time_granted, TimeState::time_requested_iterative),
              TimeState::time_requested_iterative);
    EXPECT_EQ(generateState(TimeState::time_requested_iterative, TimeState::time_granted),
              TimeState::time_requested_iterative);
    EXPECT_EQ(generateState(TimeState::time_granted,
                            TimeState::time_requested_require_iteration),
              TimeState::time_requested_require_iteration);
}
