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

static const bool ld = helics::loadCores();

#ifdef ENABLE_ZMQ_CORE
TEST(CoreFactory_tests, ZmqCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::ZMQ), true);

    auto core = helics::CoreFactory::create(helics::core_type::ZMQ, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#else  // ENABLE_ZMQ_CORE
TEST(CoreFactory_tests, ZmqCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::ZMQ), false);
}
#endif  // ENABLE_ZMQ_CORE

/*
#ifdef ENABLE_MPI_CORE

TEST(CoreFactory_tests,MpiCore_test)
{
    EXPECT_EQ (helics::core::isCoreTypeAvailable (helics::core_type::MPI), true);
    auto core = helics::CoreFactory::create (helics::core_type::MPI, "");
    ASSERT_TRUE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
}
#else
TEST(CoreFactory_tests,MpiCore_test)
{
    EXPECT_EQ (helics::isCoreTypeAvailable (helics::core_type::MPI), false);
}
#endif  // ENABLE_MPI_CORE
*/

TEST(CoreFactory_tests, TestCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::TEST), true);

    auto core = helics::CoreFactory::create(helics::core_type::TEST, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#ifdef ENABLE_IPC_CORE
TEST(CoreFactory_tests, InterprocessCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::INTERPROCESS), true);
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::IPC), true);

    auto core = helics::CoreFactory::create(helics::core_type::INTERPROCESS, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
    // make sure the OS has the chance to clean up a file
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto core2 = helics::CoreFactory::create(helics::core_type::IPC, "");
    ASSERT_TRUE(core2);
    core2->disconnect();
    core2 = nullptr;
}
#else
TEST(CoreFactory_tests, InterprocessCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::INTERPROCESS), false);
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::IPC), false);
}
#endif
#ifdef ENABLE_TCP_CORE
TEST(CoreFactory_tests, tcpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::TCP), true);

    auto core = helics::CoreFactory::create(helics::core_type::TCP, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
TEST(CoreFactory_tests, tcpSSCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::TCP_SS), true);

    auto core = helics::CoreFactory::create(helics::core_type::TCP_SS, "");
    ASSERT_TRUE(core);
    core->disconnect();
    core = nullptr;
}
#else
TEST(CoreFactory_tests, tcpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::TCP), false);
}
TEST(CoreFactory_tests, tcpSSCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::TCP_SS), false);
}
#endif

#ifdef ENABLE_UDP_CORE
TEST(CoreFactory_tests, udpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::UDP), true);

    auto core = helics::CoreFactory::create(helics::core_type::UDP, "");
    ASSERT_TRUE(core != nullptr);
    core->disconnect();
    core = nullptr;

    auto core2 = helics::CoreFactory::create(helics::core_type::UDP, "");
    ASSERT_TRUE(core2 != nullptr);
    core2->disconnect();
    core2 = nullptr;
}
#else
TEST(CoreFactory_tests, udpCore_test)
{
    EXPECT_EQ(helics::core::isCoreTypeAvailable(helics::core_type::UDP), false);
}
#endif
