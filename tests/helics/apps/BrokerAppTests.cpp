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

TEST(BrokerAppTests, constructor1)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk1", std::vector<std::string>{});

    // Brokers connect automatically
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk1");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor2)
{
    helics::BrokerApp App(helics::core_type::TEST, std::vector<std::string>{"brk2", "--name"});

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk2");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor3)
{
    helics::BrokerApp App(std::vector<std::string>{"brk3", "--name", "test", "--type"});

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk3");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor4)
{
    std::vector<std::string> args{"constructor4", "--name", "brk4"};
    char* argv[3];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);

    helics::BrokerApp App(helics::core_type::TEST, 3, argv);

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk4");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor5)
{
    std::vector<std::string> args{"constructor4", "--name", "brk5", "--type", "test"};
    char* argv[5];
    argv[0] = &(args[0][0]);
    argv[1] = &(args[1][0]);
    argv[2] = &(args[2][0]);
    argv[3] = &(args[3][0]);
    argv[4] = &(args[4][0]);

    helics::BrokerApp App(5, argv);

    EXPECT_TRUE(App.connect());
    EXPECT_TRUE(App.isConnected());
    EXPECT_TRUE(App.isOpenToNewFederates());
    EXPECT_EQ(App.getIdentifier(), "brk5");
    App.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor6)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk6", std::vector<std::string>{});

    EXPECT_TRUE(App.connect());
    EXPECT_EQ(App.getIdentifier(), "brk6");

    helics::BrokerApp App2("brk6");
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofBrokerPointer().get(), App.getCopyofBrokerPointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor8)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk8", std::vector<std::string>{});

    EXPECT_TRUE(App.connect());
    EXPECT_EQ(App.getIdentifier(), "brk8");

    helics::BrokerApp App2(App);
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofBrokerPointer().get(), App.getCopyofBrokerPointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor9)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk9", std::vector<std::string>{});

    helics::BrokerApp App2(std::move(App));

    EXPECT_TRUE(App2.isConnected());
    EXPECT_TRUE(App2.isOpenToNewFederates());
    EXPECT_EQ(App2.getIdentifier(), "brk9");
    App2.forceTerminate();
    EXPECT_FALSE(App2.isConnected());
}

TEST(BrokerAppTests, constructor10)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk10", std::vector<std::string>{});

    helics::BrokerApp App2(App.getCopyofBrokerPointer());
    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofBrokerPointer().get(), App.getCopyofBrokerPointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor11)
{
    helics::BrokerApp App(helics::core_type::TEST, "brk11", std::vector<std::string>{});

    helics::BrokerApp App2(helics::core_type::TEST, "brk11", std::vector<std::string>{});

    EXPECT_TRUE(App2.isConnected());
    // App2 should point to the same core
    EXPECT_EQ(App2.getCopyofBrokerPointer().get(), App.getCopyofBrokerPointer().get());
    App2.forceTerminate();
    EXPECT_FALSE(App.isConnected());
}

TEST(BrokerAppTests, constructor12)
{
    EXPECT_THROW(helics::BrokerApp(helics::core_type::NULLCORE,
                                   "brk12",
                                   std::vector<std::string>{}),
                 helics::HelicsException);
#ifdef ENABLE_ZMQ_CORE
    EXPECT_THROW(helics::BrokerApp(helics::core_type::ZMQ,
                                   "brk12",
                                   std::vector<std::string>{"10.7.5.5", "--interface"}),
                 helics::ConnectionFailure);
#endif
}

TEST(BrokerAppTests, null)
{
    helics::BrokerApp app;
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

TEST(BrokerAppTests, help)
{
    helics::BrokerApp app("--type=test --help");
    EXPECT_FALSE(app.isOpenToNewFederates());
    EXPECT_FALSE(app.isConnected());
    EXPECT_FALSE(app.connect());
    EXPECT_NO_THROW(app.forceTerminate());
    EXPECT_TRUE(app.getAddress().empty());
    EXPECT_TRUE(app.getIdentifier().empty());
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(BrokerAppTests, file_logging_p2)
{
    helics::BrokerApp app("--name=loggerBrk1 --type=test");
    app.setLoggingLevel(5);
    const std::string lfilename = "logfile3.txt";
    app.setLogFile("logfile3.txt");
    app.connect();

    helics::FederateInfo fi;
    fi.broker = "loggerBrk1";
    fi.coreType = helics::core_type::TEST;
    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}
