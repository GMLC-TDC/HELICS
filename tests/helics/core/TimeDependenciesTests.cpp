/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/TimeDependencies.hpp"

#include "gtest/gtest.h"

using namespace helics;

TEST(timeDep_tests, dependency_tests)
{
    std::vector<DependencyInfo> deps;
    deps.resize(2);
    deps[0].connection = ConnectionType::child;
    deps[0].fedID = global_federate_id(131073);
    deps[0].time_state = time_state_t::time_requested;
    deps[0].dependent = true;
    deps[0].dependency = true;
    deps[0].forwarding = false;
    deps[0].next = 2.0;
    deps[0].Te = 2.0;
    deps[0].minDe = 2.0;

    deps[1].connection = ConnectionType::child;
    deps[1].fedID = global_federate_id(1879048192);
    deps[1].time_state = time_state_t::time_requested;
    deps[1].dependent = true;
    deps[1].dependency = true;
    deps[1].forwarding = true;
    deps[1].next = 1e-9;
    deps[1].Te = Time::maxVal();
    deps[1].minDe = Time::maxVal();

    TimeDependencies depTest;
    depTest.setDependencyVector(deps);
    auto total = generateMinTimeTotal(depTest, false, global_federate_id(1), global_federate_id());
    EXPECT_EQ(total.next, 2.0);
}
