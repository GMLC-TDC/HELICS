/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
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
    helics::CoreApp App(helics::core_type::TEST,
                        std::vector<std::string>{"--autobroker", "core2", "--name"});

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
    helics::CoreApp App(
        std::vector<std::string>{"--autobroker", "core3", "--name", "test", "--type"});

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
    std::vector<std::string> args{"constructor4", "--autobroker", "--name", "core4"};
    char* argv[4];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);

    helics::CoreApp App(helics::core_type::TEST, 4, argv);

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
    std::vector<std::string> args{
        "constructor4", "--autobroker", "--name", "core5", "--type", "test"};
    char* argv[6];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);
    argv[4] = &(args[4][0]);
    argv[5] = &(args[5][0]);

    helics::CoreApp App(6, argv);

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
    App.forceTerminate();
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

TEST(CoreAppTests, constructor10)
{
    helics::CoreApp App(helics::core_type::TEST,
                        "core10",
                        std::vector<std::string>{"--autobroker"});

    helics::CoreApp App2(App.getCopyofCorePointer());

    EXPECT_FALSE(App2.isConnected());
    App.connect();
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofCorePointer().get(), App.getCopyofCorePointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(CoreAppTests, null)
{
    helics::CoreApp app;
    EXPECT_FALSE(app.isOpenToNewFederates());
    EXPECT_FALSE(app.isConnected());
    EXPECT_FALSE(app.connect());
    EXPECT_NO_THROW(app.forceTerminate());
    EXPECT_TRUE(app.getAddress().empty());
    EXPECT_TRUE(app.getIdentifier().empty());
    EXPECT_TRUE(app.waitForDisconnect());
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(CoreAppTests, help)
{
    helics::CoreApp app("--type=test --help");
    EXPECT_FALSE(app.isOpenToNewFederates());
    EXPECT_FALSE(app.isConnected());
    EXPECT_FALSE(app.connect());
    EXPECT_NO_THROW(app.forceTerminate());
    EXPECT_TRUE(app.getAddress().empty());
    EXPECT_TRUE(app.getIdentifier().empty());
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(CoreAppTests, file_logging_p2)
{
    helics::CoreApp app("--autobroker --name=loggerCore1 --type=test");
    app.setLoggingLevel(5);
    const std::string lfilename = "logfile2.txt";
    app.setLogFile("logfile2.txt");

    auto Fed = std::make_shared<helics::Federate>("test1", app, helics::FederateInfo{});

    Fed->enterExecutingMode();
    Fed->finalize();

    Fed.reset();
    EXPECT_TRUE(ghc::filesystem::exists(lfilename));
    app.waitForDisconnect();
    app.reset();
    helics::cleanupHelicsLibrary();
    std::error_code ec;
    bool res = ghc::filesystem::remove(lfilename, ec);
    int ii = 0;
    while (!res) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        res = ghc::filesystem::remove(lfilename, ec);
        ++ii;
        if (ii > 15) {
            break;
        }
    }
    EXPECT_TRUE(res);
}

TEST(CoreAppTests, core_global_file_ci_skip)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::BrokerApp brk(helics::core_type::TEST, "be1", "-f2");
    brk.connect();
    brk.setGlobal("GlobalB", "excited");
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_globale3";
    fi.coreInitString = "-f 1";

    auto Fed1 = std::make_shared<helics::Federate>("fed1", fi);
    fi.coreName = "core_globale4";
    auto Fed2 = std::make_shared<helics::Federate>("fed2", fi);

    helics::CoreApp app(Fed1->getCorePointer());
    auto testFile = std::string(GLOBAL_TEST_DIR) + "example_globals1.json";
    app.makeConnections(testFile);
    app.setGlobal("GlobalC", "excited_too");
    Fed1->enterInitializingModeAsync();
    Fed2->enterInitializingMode();

    Fed1->enterInitializingModeComplete();

    auto str1 = Fed1->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = Fed2->query("global", "global1");
    EXPECT_EQ(str1, "this is a global1 value");
    str1 = app.query("global", "GlobalB");
    EXPECT_EQ(str1, "excited");
    str1 = brk.query("global", "GlobalC");
    EXPECT_EQ(str1, "excited_too");

    str1 = Fed1->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = Fed2->query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = app.query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");
    str1 = brk.query("global", "global2");
    EXPECT_EQ(str1, "this is another global value");

    auto str3 = Fed1->query("global", "all");
    EXPECT_NE(str3, "#invalid");
    Fed1->finalize();
    Fed2->finalize();
    app.reset();
    brk.waitForDisconnect();
}

TEST(CoreAppTests, readyToInit)
{
    helics::BrokerApp b(helics::core_type::TEST, "brkt1", "-f1");
    EXPECT_TRUE(b.connect());
    helics::CoreApp c1(helics::core_type::TEST, "--broker=brkt1 --name=core1b");
    EXPECT_TRUE(c1.connect());
    c1.haltInit();

    helics::Federate fedb("fedb", c1);
    fedb.enterExecutingModeAsync();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(fedb.isAsyncOperationCompleted());
    c1.setReadyToInit();
    fedb.enterExecutingModeComplete();
    fedb.finalize();
    c1->disconnect();
    b.forceTerminate();
    EXPECT_TRUE(b.waitForDisconnect(std::chrono::milliseconds(500)));
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}
