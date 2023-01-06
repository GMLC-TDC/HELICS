/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../ThirdParty/concurrency/gmlc/libguarded/guarded.hpp"

#include <complex>
#include <filesystem>
#include <gtest/gtest.h>
#include <thread>

/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"
#include "helics/helics.h"

using logblocktype = gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>>;
TEST(logging_tests, check_log_message)
{
    auto fi = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fi, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreInitString(fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty(fi,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TIMING,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fi, &err);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* mp = reinterpret_cast<logblocktype*>(udata);
        mp->lock()->emplace_back(level, message);
    };

    helicsFederateSetLoggingCallback(fed, logg, &mlog, &err);

    EXPECT_EQ(err.error_code, 0);
    auto loglevel = helicsFederateGetIntegerProperty(fed, HELICS_PROPERTY_INT_LOG_LEVEL, &err);
    EXPECT_EQ(loglevel, HELICS_LOG_LEVEL_TIMING);
    helicsFederateEnterExecutingMode(fed, &err);
    helicsFederateLogInfoMessage(fed, "test MEXAGE", &err);
    helicsFederateRequestNextStep(fed, &err);
    helicsFederateFinalize(fed, &err);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(err.error_code, 0);
    auto llock = mlog.lock();
    bool found = false;
    for (auto& m : llock) {
        if (m.second.find("MEXAGE") != std::string::npos) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
    if (!found) {
        for (auto& m : llock) {
            std::cout << "message (" << m.first << ") ::" << m.second << std::endl;
        }
    }
    helicsFederateFree(fed);
    helicsFederateInfoFree(fi);
}

TEST(logging_tests, check_log_message_levels)
{
    auto fi = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fi, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreInitString(fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty(fi,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TIMING,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fi, &err);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* mp = reinterpret_cast<logblocktype*>(udata);
        mp->lock()->emplace_back(level, message);
    };
    helicsFederateSetLoggingCallback(fed, nullptr, &mlog, &err);

    helicsFederateSetLoggingCallback(fed, logg, &mlog, &err);

    EXPECT_EQ(err.error_code, 0);

    helicsFederateEnterExecutingMode(fed, &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_TIMING, "test MEXAGE1", &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_TRACE + 3, "test MEXAGE2", &err);
    helicsFederateRequestNextStep(fed, &err);
    helicsFederateFinalize(fed, &err);
    EXPECT_EQ(err.error_code, 0);

    auto llock = mlog.lock();
    bool found_low = false;
    bool found_high = false;
    for (auto& m : llock) {
        if (m.second.find("MEXAGE1") != std::string::npos) {
            found_low = true;
        }
        if (m.second.find("MEXAGE2") != std::string::npos) {
            found_high = true;
        }
    }
    EXPECT_TRUE(found_low);
    EXPECT_FALSE(found_high);

    helicsFederateFree(fed);
    helicsFederateInfoFree(fi);
}

TEST(logging_tests, check_log_message_levels_high)
{
    auto fi = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fi, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreInitString(fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty(fi,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TRACE + 6,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fi, &err);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* mp = reinterpret_cast<logblocktype*>(udata);
        mp->lock()->emplace_back(level, message);
    };

    helicsFederateSetLoggingCallback(fed, logg, &mlog, &err);

    EXPECT_EQ(err.error_code, 0);

    helicsFederateEnterExecutingMode(fed, &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_CONNECTIONS, "test MEXAGE1", &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_TRACE + 3, "test MEXAGE2", &err);
    helicsFederateRequestNextStep(fed, &err);
    helicsFederateFinalize(fed, &err);
    EXPECT_EQ(err.error_code, 0);

    auto llock = mlog.lock();
    bool found_low = false;
    bool found_high = false;
    for (auto& m : llock) {
        if (m.second.find("MEXAGE1") != std::string::npos) {
            found_low = true;
        }
        if (m.second.find("MEXAGE2") != std::string::npos) {
            found_high = true;
        }
    }
    EXPECT_TRUE(found_low && found_high);
    helicsFederateFree(fed);
    helicsFederateInfoFree(fi);
}

TEST(logging_tests, core_logging)
{
    auto core = helicsCreateCore("inproc", "ctype", "--autobroker --log_level=trace", nullptr);

    helicsCoreSetLoggingCallback(core, nullptr, nullptr, nullptr);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* mp = reinterpret_cast<logblocktype*>(udata);
        mp->lock()->emplace_back(level, message);
    };
    auto err = helicsErrorInitialize();
    helicsCoreSetLoggingCallback(core, logg, &mlog, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsCoreDisconnect(core, nullptr);
    helicsCloseLibrary();
    EXPECT_FALSE(mlog.lock()->empty());
}

TEST(logging_tests, broker_logging)
{
    auto broker = helicsCreateBroker("inproc", "btype", "--log_level=trace", nullptr);

    helicsBrokerSetLoggingCallback(broker, nullptr, nullptr, nullptr);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* mp = reinterpret_cast<logblocktype*>(udata);
        mp->lock()->emplace_back(level, message);
    };
    auto err = helicsErrorInitialize();
    helicsBrokerSetLoggingCallback(broker, logg, &mlog, &err);
    EXPECT_EQ(err.error_code, 0);
    helicsBrokerDisconnect(broker, nullptr);
    helicsCloseLibrary();
    EXPECT_FALSE(mlog.lock()->empty());
}

TEST(logging_tests, broker_logging_file)
{
    const std::string lfile{"logb.txt"};

    if (std::filesystem::exists(lfile)) {
        std::filesystem::remove(lfile);
    }
    auto err = helicsErrorInitialize();
    auto broker = helicsCreateBroker("inproc", "blog", "--log_level=trace", &err);
    helicsBrokerSetLogFile(broker, lfile.c_str(), &err);
    helicsBrokerDisconnect(broker, &err);
    helicsCloseLibrary();
    ASSERT_TRUE(std::filesystem::exists(lfile));
    std::filesystem::remove(lfile);
    EXPECT_EQ(err.error_code, 0);
}

TEST(logging_tests, core_logging_file)
{
    const std::string lfile{"logc.txt"};
    if (std::filesystem::exists(lfile)) {
        std::filesystem::remove(lfile);
    }
    auto core = helicsCreateCore("inproc", "clog", "--autobroker --log_level=trace", nullptr);

    auto err = helicsErrorInitialize();
    helicsCoreSetLogFile(core, lfile.c_str(), &err);
    helicsCoreDisconnect(core, &err);
    helicsCloseLibrary();
    ASSERT_TRUE(std::filesystem::exists(lfile));
    std::filesystem::remove(lfile);
}

TEST(logging_tests, fed_logging_file)
{
    const std::string lfile{"logf.txt"};
    if (std::filesystem::exists(lfile)) {
        std::filesystem::remove(lfile);
    }
    auto core = helicsCreateCore("inproc", "clogf", "--autobroker --log_level=trace", nullptr);

    auto err = helicsErrorInitialize();
    auto fi = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreName(fi, "clogf", nullptr);
    auto fed = helicsCreateValueFederate("f1", fi, nullptr);
    helicsFederateSetLogFile(fed, lfile.c_str(), nullptr);

    helicsCoreSetLogFile(core, lfile.c_str(), &err);
    helicsCoreDisconnect(core, &err);
    helicsFederateFinalize(fed, &err);

    helicsFederateSetLogFile(fed, "emptyfile.txt", nullptr);
    helicsFederateInfoFree(fi);
    helicsCloseLibrary();
    ASSERT_TRUE(std::filesystem::exists(lfile));
    std::filesystem::remove(lfile);
}
