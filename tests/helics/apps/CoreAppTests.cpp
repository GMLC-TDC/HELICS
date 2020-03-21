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

#include "helics/application_api/CoreApp.hpp"
#include "helics/core/core-exceptions.hpp"
#include <cstdio>
#include <future>

TEST(CoreAppTests, constructor1)
{
    helics::CoreApp App(helics::core_type::TEST, "core1", std::vector<std::string>{"--autobroker"});

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "core1");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, constructor2)
{
    helics::CoreApp App(helics::core_type::TEST, std::vector<std::string>{"--autobroker","core2", "--name"});

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "core2");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, constructor3)
{
    helics::CoreApp App(std::vector<std::string>{"--autobroker", "core3", "--name", "test","--type"});

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "core3");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, constructor4)
{
    constexpr char *name = "constructor4";
    std::vector<std::string> args{ "--autobroker", "--name", "core4" };
    char *argv[4];
    argv[0] = name;
    argv[1] = &(args[0][0]);
    argv[2] = &(args[1][0]);
    argv[3] = &(args[2][0]);


    helics::CoreApp App(helics::core_type::TEST, 4,argv);

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "core4");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, constructor5)
{
    constexpr char *name = "constructor4";
    std::vector<std::string> args{ "--autobroker", "--name", "core5","--type","test" };
    char *argv[6];
    argv[0] = name;
    argv[1] = &(args[0][0]);
    argv[2] = &(args[1][0]);
    argv[3] = &(args[2][0]);
    argv[4] = &(args[3][0]);
    argv[5] = &(args[4][0]);

    helics::CoreApp App( 6, argv);

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "core5");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}


TEST(CoreAppTests, constructor6)
{
    helics::CoreApp App(helics::core_type::TEST, "core6", std::vector<std::string>{"--autobroker"});

    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_EQ(App.getIdentifier(), "core6");

    helics::CoreApp App2("core6");
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofCorePointer().get(), App.getCopyofCorePointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, constructor7)
{
    helics::CoreApp App(helics::core_type::TEST, "core7", std::vector<std::string>{"--autobroker"});

    EXPECT_THROW(helics::CoreApp App2("core9"), helics::ConnectionFailure);

}

TEST(CoreAppTests, constructor8)
{
    helics::CoreApp App(helics::core_type::TEST, "core8", std::vector<std::string>{"--autobroker"});

    
    EXPECT_FALSE(App.isConnected());

    EXPECT_TRUE(App.connect());
    EXPECT_EQ(App.getIdentifier(), "core8");

    helics::CoreApp App2(App);
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofCorePointer().get(), App.getCopyofCorePointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());

}

TEST(CoreAppTests, constructor9)
{
    helics::CoreApp App(helics::core_type::TEST, "core9", std::vector<std::string>{"--autobroker"});

    helics::CoreApp App2(std::move(App));

    EXPECT_FALSE(App2.isConnected());

    EXPECT_TRUE(App2.connect());
    EXPECT_TRUE(App2.isConnected());
    EXPECT_TRUE(App2.isOpenToNewFederates());
    EXPECT_EQ(App2.getIdentifier(), "core9");
    App2.forceTerminate();
    EXPECT_FALSE(App2.isConnected());
}
