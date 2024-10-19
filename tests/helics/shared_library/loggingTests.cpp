/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../ThirdParty/concurrency/gmlc/libguarded/guarded.hpp"

#include <complex>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <thread>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"
#include "helics/helics.h"

using logblocktype = gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>>;

TEST(logging_tests, check_log_message)
{
    helicsCleanupLibrary();
    auto fedInfo = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreName(fedInfo, "clogcore", &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "--autobroker --broker=clogbroker", &err);
    helicsFederateInfoSetIntegerProperty(fedInfo,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TIMING,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fedInfo, &err);
    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* messageLock = reinterpret_cast<logblocktype*>(udata);
        messageLock->lock()->emplace_back(level, message);
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
    for (auto& message : *llock) {
        if (message.second.find("MEXAGE") != std::string::npos) {
            found = true;
        }
    }

    /*if (!found) {
        for (auto& message : *llock) {
            std::cout << "message (" << message.first << ") ::" << message.second << std::endl;
        }
    }
    */
    EXPECT_TRUE(found);
    llock.unlock();
    helicsFederateFree(fed);
    helicsFederateInfoFree(fedInfo);
}

TEST(logging_tests, check_log_message_levels)
{
    auto fedInfo = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "--autobroker --broker=clogbrokerlevel", &err);
    helicsFederateInfoSetCoreName(fedInfo, "clogcorelevels", &err);
    helicsFederateInfoSetIntegerProperty(fedInfo,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TIMING,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fedInfo, &err);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* messageLock = reinterpret_cast<logblocktype*>(udata);
        messageLock->lock()->emplace_back(level, message);
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
    for (auto& message : *llock) {
        if (message.second.find("MEXAGE1") != std::string::npos) {
            found_low = true;
        }
        if (message.second.find("MEXAGE2") != std::string::npos) {
            found_high = true;
        }
    }
    EXPECT_TRUE(found_low);
    EXPECT_FALSE(found_high);
    llock.unlock();
    helicsFederateFree(fed);
    helicsFederateInfoFree(fedInfo);
}

TEST(logging_tests, check_log_message_levels_high)
{
    auto fedInfo = helicsCreateFederateInfo();
    auto err = helicsErrorInitialize();
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_TEST, &err);
    helicsFederateInfoSetCoreInitString(fedInfo, "--autobroker --broker=clogbrokerhigh", &err);
    helicsFederateInfoSetCoreName(fedInfo, "clogcorehigh", &err);
    helicsFederateInfoSetIntegerProperty(fedInfo,
                                         HELICS_PROPERTY_INT_LOG_LEVEL,
                                         HELICS_LOG_LEVEL_TRACE + 6,
                                         &err);

    auto fed = helicsCreateValueFederate("test1", fedInfo, &err);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* messageLock = reinterpret_cast<logblocktype*>(udata);
        messageLock->lock()->emplace_back(level, message);
    };

    helicsFederateSetLoggingCallback(fed, logg, &mlog, &err);

    EXPECT_EQ(err.error_code, 0);

    helicsFederateEnterExecutingMode(fed, &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_CONNECTIONS, "test MEXAGE1", &err);
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_TRACE + 3, "test MEXAGE2", &err);
    helicsFederateRequestNextStep(fed, &err);
    helicsFederateFinalize(fed, &err);
    EXPECT_EQ(err.error_code, 0);
    bool found_low = false;
    bool found_high = false;
    auto llock = mlog.lock();
    for (auto& message : *llock) {
        if (message.second.find("MEXAGE1") != std::string::npos) {
            found_low = true;
        }
        if (message.second.find("MEXAGE2") != std::string::npos) {
            found_high = true;
        }
    }
    llock.unlock();
    EXPECT_TRUE(found_low && found_high);
    helicsFederateFree(fed);
    helicsFederateInfoFree(fedInfo);
}

TEST(logging_tests, core_logging)
{
    auto core = helicsCreateCore("inproc",
                                 "ctype",
                                 "--autobroker --log_level=trace --broker=ncorelogging",
                                 nullptr);

    helicsCoreSetLoggingCallback(core, nullptr, nullptr, nullptr);

    logblocktype mlog;

    auto logg = [](int level, const char* /*unused*/, const char* message, void* udata) {
        auto* messageLock = reinterpret_cast<logblocktype*>(udata);
        messageLock->lock()->emplace_back(level, message);
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
        auto* messageLock = reinterpret_cast<logblocktype*>(udata);
        messageLock->lock()->emplace_back(level, message);
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
    auto core = helicsCreateCore("inproc",
                                 "clog",
                                 "--autobroker --log_level=trace --broker=clogfile",
                                 nullptr);

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
    auto core = helicsCreateCore("inproc",
                                 "clogf",
                                 "--autobroker --log_level=trace --broker=clogfbroker",
                                 nullptr);

    auto err = helicsErrorInitialize();
    auto fedInfo = helicsCreateFederateInfo();
    helicsFederateInfoSetCoreName(fedInfo, "clogf", nullptr);
    auto fed = helicsCreateValueFederate("f1", fedInfo, nullptr);
    helicsFederateSetLogFile(fed, lfile.c_str(), nullptr);

    helicsCoreSetLogFile(core, lfile.c_str(), &err);
    helicsCoreDisconnect(core, &err);
    helicsFederateFinalize(fed, &err);

    helicsFederateSetLogFile(fed, "emptyfile.txt", nullptr);
    helicsFederateInfoFree(fedInfo);
    helicsCloseLibrary();
    ASSERT_TRUE(std::filesystem::exists(lfile));
    std::filesystem::remove(lfile);
}
