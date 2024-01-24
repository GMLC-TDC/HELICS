/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <gtest/gtest.h>

class command_tests: public ::testing::Test, public FederateTestFixture {};

/** just a check that in the simple case we do actually get the time back we requested*/

TEST_F(command_tests, federate_federate_command)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    helicsFederateRegisterGlobalPublication(vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsFederateSendCommand(vFed1, helicsFederateGetName(vFed2), "test", nullptr);
    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);

    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);

    const auto* cmd = helicsFederateGetCommand(vFed2, nullptr);

    EXPECT_STREQ(cmd, "test");

    const auto* cmdsrc = helicsFederateGetCommandSource(vFed2, nullptr);
    EXPECT_STREQ(cmdsrc, helicsFederateGetName(vFed1));
    helicsFederateFinalize(vFed1, nullptr);
    helicsFederateFinalize(vFed2, nullptr);
}

TEST_F(command_tests, core_federate_command)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    helicsFederateRegisterGlobalPublication(vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    auto core = helicsFederateGetCore(vFed1, nullptr);
    helicsCoreSendCommand(core, helicsFederateGetName(vFed2), "test", nullptr);
    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);

    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);

    const auto* cmd = helicsFederateGetCommand(vFed2, nullptr);

    EXPECT_STREQ(cmd, "test");

    const auto* cmdsrc = helicsFederateGetCommandSource(vFed2, nullptr);
    EXPECT_STREQ(cmdsrc, helicsCoreGetIdentifier(core));
    helicsCoreFree(core);
    helicsFederateFinalize(vFed1, nullptr);
    helicsFederateFinalize(vFed2, nullptr);
}

TEST_F(command_tests, broker_federate_command)
{
    SetupTest(helicsCreateValueFederate, "test", 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    // register the publications
    helicsFederateRegisterGlobalPublication(vFed1, "pub1", HELICS_DATA_TYPE_DOUBLE, "", nullptr);
    helicsFederateRegisterSubscription(vFed2, "pub1", "", nullptr);

    helicsBrokerSendCommand(brokers[0], helicsFederateGetName(vFed2), "test", nullptr);
    helicsFederateEnterExecutingModeAsync(vFed1, nullptr);
    helicsFederateEnterExecutingMode(vFed2, nullptr);

    helicsFederateEnterExecutingModeComplete(vFed1, nullptr);

    const auto* cmd = helicsFederateGetCommand(vFed2, nullptr);

    EXPECT_STREQ(cmd, "test");

    const auto* cmdsrc = helicsFederateGetCommandSource(vFed2, nullptr);
    EXPECT_STREQ(cmdsrc, helicsBrokerGetIdentifier(brokers[0]));
    helicsFederateFinalize(vFed1, nullptr);
    helicsFederateFinalize(vFed2, nullptr);
}
