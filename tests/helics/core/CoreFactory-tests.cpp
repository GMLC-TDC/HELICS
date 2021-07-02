/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/coreTypeOperations.hpp"
#include "helics/helics-config.h"
#include "helics/network/loadCores.hpp"

#include "gtest/gtest.h"
#include <thread>

static const bool ld = helics::loadCores();

#ifdef HELICS_ENABLE_ZMQ_CORE
TEST(CoreFactory_tests, ZmqCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::ZMQ), true);

    auto core = helics::CoreFactory::create(helics::CoreType::ZMQ, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#else  // HELICS_ENABLE_ZMQ_CORE
TEST(CoreFactory_tests, ZmqCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::ZMQ), false);
}
#endif  // HELICS_ENABLE_ZMQ_CORE

/*
#ifdef HELICS_ENABLE_MPI_CORE

TEST(CoreFactory_tests,MpiCore_test)
{
    EXPECT_EQ (helics::core::isCoreTypeAvailable (helics::CoreType::MPI), true);
    auto core = helics::CoreFactory::create (helics::CoreType::MPI, "");
    ASSERT_TRUE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
}
#else
TEST(CoreFactory_tests,MpiCore_test)
{
    EXPECT_EQ (helics::isCoreTypeAvailable (helics::CoreType::MPI), false);
}
#endif  // HELICS_ENABLE_MPI_CORE
*/

TEST(CoreFactory_tests, TestCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::TEST), true);

    auto core = helics::CoreFactory::create(helics::CoreType::TEST, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#ifdef HELICS_ENABLE_IPC_CORE
TEST(CoreFactory_tests, InterprocessCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::INTERPROCESS), true);
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::IPC), true);

    auto core = helics::CoreFactory::create(helics::CoreType::INTERPROCESS, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
    // make sure the OS has the chance to clean up a file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto core2 = helics::CoreFactory::create(helics::CoreType::IPC, "");
    ASSERT_TRUE(core2);
    core2->disconnect();
    core2 = nullptr;
}
#else
TEST(CoreFactory_tests, InterprocessCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::INTERPROCESS), false);
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::IPC), false);
}
#endif
#ifdef HELICS_ENABLE_TCP_CORE
TEST(CoreFactory_tests, tcpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::TCP), true);

    auto core = helics::CoreFactory::create(helics::CoreType::TCP, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
TEST(CoreFactory_tests, tcpSSCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::TCP_SS), true);

    auto core = helics::CoreFactory::create(helics::CoreType::TCP_SS, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#else
TEST(CoreFactory_tests, tcpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::TCP), false);
}
TEST(CoreFactory_tests, tcpSSCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::TCP_SS), false);
}
#endif

#ifdef HELICS_ENABLE_UDP_CORE
TEST(CoreFactory_tests, udpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::UDP), true);

    auto core = helics::CoreFactory::create(helics::CoreType::UDP, "");
    ASSERT_TRUE(core != nullptr);
    core->disconnect();
    core = nullptr;

    auto core2 = helics::CoreFactory::create(helics::CoreType::UDP, "");
    ASSERT_TRUE(core2 != nullptr);
    core2->disconnect();
    core2 = nullptr;
}
#else
TEST(CoreFactory_tests, udpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::CoreType::UDP), false);
}
#endif
