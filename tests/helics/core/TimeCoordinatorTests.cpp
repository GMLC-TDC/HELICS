/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/ActionMessage.hpp"
#include "helics/core/TimeCoordinator.hpp"

#include "gtest/gtest.h"

using namespace helics;

static constexpr global_federate_id fed2(2);
static constexpr global_federate_id fed3(3);
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
    remDep.source_id = global_federate_id(10);
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
    remDep.source_id = global_federate_id(10);
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependents();
    EXPECT_EQ(deps.size(), 1U);
    EXPECT_TRUE(deps[0] == fed3);
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
