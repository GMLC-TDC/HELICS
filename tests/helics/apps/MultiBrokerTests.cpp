/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gtest/gtest.h"

#ifdef _MSC_VER
#    pragma warning(push, 0)
#    include "helics/external/filesystem.hpp"
#    pragma warning(pop)
#else
#    include "helics/external/filesystem.hpp"
#endif

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/apps/MultiBroker.hpp"

#include <cstdio>
#include <future>

static const bool amb = helics::allowMultiBroker();

TEST(MultiBroker, constructor1)
{
    helics::BrokerApp App(helics::core_type::MULTI, "brk1", "--type test");

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk1");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(MultiBroker, connect1)
{
    helics::BrokerApp b(helics::core_type::MULTI, "brk2", "--type test");
    EXPECT_TRUE(b.connect());
    helics::CoreApp c1(helics::core_type::TEST, "--broker=brkt2 --name=core1b");
    EXPECT_TRUE(c1.connect());

    helics::Federate fedb("fedb", c1);
    fedb.enterExecutingMode();

    fedb.finalize();
    c1->disconnect();
    b.forceTerminate();
    EXPECT_TRUE(b.waitForDisconnect(std::chrono::milliseconds(500)));
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}
