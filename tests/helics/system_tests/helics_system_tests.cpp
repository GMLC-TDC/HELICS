/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/helics-config.h"

#include "gtest/gtest.h"
#include <helics/core/BrokerFactory.hpp>
#include <helics/core/CoreFactory.hpp>

#include <cstdlib>
#include <iostream>
#include <thread>

namespace {
bool debugCleanupEnabled()
{
    static const bool enabled = []() {
        const auto* env = std::getenv("HELICS_DEBUG_CLEANUP");
        return env != nullptr && env[0] != '\0' && env[0] != '0';
    }();
    return enabled;
}

void cleanupTrace(const char* stage)
{
    if (!debugCleanupEnabled()) {
        return;
    }
    std::cerr << "[helics-cleanup][system-tests] " << stage
              << " tid=" << std::this_thread::get_id() << '\n';
}
}  // namespace

struct globalTestConfig: public ::testing::Environment {
    virtual void TearDown() override
    {
        cleanupTrace("global teardown start");
        helics::CoreFactory::cleanUpCores();
        cleanupTrace("after core cleanup");
        helics::BrokerFactory::cleanUpBrokers();
        cleanupTrace("after broker cleanup");
    }
};

// register the global setup and teardown structure
::testing::Environment* const foo_env = ::testing::AddGlobalTestEnvironment(new globalTestConfig);
//____________________________________________________________________________//
