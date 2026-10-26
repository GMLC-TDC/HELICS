/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../ThirdParty/concurrency/gmlc/libguarded/guarded.hpp"

#include <algorithm>
#include <complex>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iterator>
#include <string>
#include <thread>
/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"
#include "helics/helics.h"

using logblocktype = gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>>;

namespace {
// The logging setters are queued. An ordered local query waits for them without shutting down
// the library. "logs" is deliberately used because simple queries such as "name" bypass the queue.
void synchronizeCoreLogging(HelicsCore core, HelicsError* err)
{
    auto query = helicsCreateQuery("core", "logs");
    helicsQuerySetOrdering(query, HELICS_SEQUENCING_MODE_ORDERED, err);
    helicsQueryCoreExecute(query, core, err);
    helicsQueryFree(query);
}

void synchronizeBrokerLogging(HelicsBroker broker, HelicsError* err)
{
    auto query = helicsCreateQuery("broker", "logs");
    helicsQuerySetOrdering(query, HELICS_SEQUENCING_MODE_ORDERED, err);
    helicsQueryBrokerExecute(query, broker, err);
    helicsQueryFree(query);
}

bool hasLogMessage(logblocktype& messages, int level, const std::string& message)
{
    const auto logs = messages.lock();
    return std::any_of(logs->begin(), logs->end(), [&](const auto& entry) {
        return entry.first == level && entry.second == message;
    });
}

std::string readLogFile(const std::string& filename)
{
    std::ifstream stream(filename);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
}  // namespace

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
    helicsCoreSendOrderedCommand(core, "core", "log core logging test message", &err);
    EXPECT_EQ(err.error_code, 0);
    // Detach the callback before the stack-owned message storage can be destroyed.
    helicsErrorClear(&err);
    helicsCoreSetLoggingCallback(core, nullptr, nullptr, &err);
    synchronizeCoreLogging(core, &err);
    helicsCoreDestroy(core);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_TRUE(hasLogMessage(mlog, HELICS_LOG_LEVEL_SUMMARY, "core logging test message"));
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
    helicsBrokerSendOrderedCommand(broker, "broker", "log broker logging test message", &err);
    synchronizeBrokerLogging(broker, &err);
    helicsBrokerDisconnect(broker, nullptr);
    // Retain coverage of a logging callback through full library shutdown.
    helicsCloseLibrary();
    EXPECT_EQ(err.error_code, 0);
    EXPECT_TRUE(hasLogMessage(mlog, HELICS_LOG_LEVEL_SUMMARY, "broker logging test message"));
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
    helicsBrokerSendOrderedCommand(broker, "broker", "log broker file test message", &err);
    // Clearing the sink flushes and closes the file before it is read or removed on Windows.
    helicsBrokerSetLogFile(broker, "", &err);
    synchronizeBrokerLogging(broker, &err);
    const auto contents = readLogFile(lfile);
    helicsBrokerDestroy(broker);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_NE(contents.find("broker file test message"), std::string::npos);
    std::filesystem::remove(lfile);
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
    helicsCoreSendOrderedCommand(core, "core", "log core file test message", &err);
    helicsCoreSetLogFile(core, "", &err);
    synchronizeCoreLogging(core, &err);
    const auto contents = readLogFile(lfile);
    helicsCoreDestroy(core);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_NE(contents.find("core file test message"), std::string::npos);
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
    helicsFederateInfoSetCoreType(fedInfo, HELICS_CORE_TYPE_INPROC, &err);
    helicsFederateInfoSetCoreName(fedInfo, "clogf", &err);
    helicsFederateInfoSetIntegerProperty(
        fedInfo, HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY, &err);
    auto fed = helicsCreateValueFederate("f1", fedInfo, &err);
    helicsFederateSetLogFile(fed, lfile.c_str(), &err);
    helicsFederateEnterExecutingMode(fed, &err);
    helicsFederateLogInfoMessage(fed, "federate file test message", &err);
    // A completed step drains the federate's logging message before the core closes its sink.
    helicsFederateRequestNextStep(fed, &err);
    helicsFederateSetLogFile(fed, "", &err);
    synchronizeCoreLogging(core, &err);
    const auto contents = readLogFile(lfile);
    helicsFederateFinalize(fed, &err);
    helicsFederateFree(fed);
    helicsFederateInfoFree(fedInfo);
    helicsCoreDestroy(core);
    EXPECT_EQ(err.error_code, 0);
    EXPECT_NE(contents.find("federate file test message"), std::string::npos);
    std::filesystem::remove(lfile);
}
