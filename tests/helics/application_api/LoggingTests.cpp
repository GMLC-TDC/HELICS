/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
// #include "helics/core/CoreFactory.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/common/LogBuffer.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"

#include <filesystem>
#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

/** these test cases test out user-directed logging functionality
 */

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST(logging, basic_logging)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed->logMessage(3, "test log message");
    Fed->enterExecutingMode();
    Fed->finalize();

    EXPECT_TRUE(!mlog.lock()->empty());
}

TEST(logging, file_logging)
{
    const std::string lfilename = "logfile.txt";
    if (std::filesystem::exists(lfilename)) {
        std::error_code ec;
        bool res = std::filesystem::remove(lfilename, ec);
        int ii = 0;
        while (!res) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            res = std::filesystem::remove(lfilename, ec);
            ++ii;
            if (ii > 15) {
                break;
            }
        }
        EXPECT_TRUE(res);
    }
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --logfile logfile.txt --fileloglevel=timing";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    Fed->enterExecutingMode();
    Fed->finalize();
    auto cr{Fed->getCorePointer()};
    Fed.reset();

    EXPECT_TRUE(std::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
    helics::cleanupHelicsLibrary();
    std::error_code ec;
    std::filesystem::remove(lfilename, ec);
}

TEST(logging, file_logging_p2)
{
    const std::string lfilename = "logfile2.txt";
    if (std::filesystem::exists(lfilename)) {
        std::error_code ec;
        bool res = std::filesystem::remove(lfilename, ec);
        int ii = 0;
        while (!res) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            res = std::filesystem::remove(lfilename, ec);
            ++ii;
            if (ii > 15) {
                break;
            }
        }
        EXPECT_TRUE(res);
    }
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --fileloglevel=timing";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);
    auto cr = Fed->getCorePointer();

    cr->setLogFile(lfilename);
    Fed->enterExecutingMode();
    Fed->finalize();

    Fed.reset();
    EXPECT_TRUE(std::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
    helics::cleanupHelicsLibrary();
    std::error_code ec;
    std::filesystem::remove(lfilename, ec);
}

TEST(logging, check_log_message)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed->enterExecutingMode();
    Fed->logInfoMessage("test MEXAGE");
    Fed->requestNextStep();
    Fed->finalize();

    auto llock = mlog.lock();
    bool found = false;
    for (auto& m : llock) {
        if (m.second.find("MEXAGE") != std::string::npos) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(logging, check_log_message_command)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed->enterExecutingMode();
    Fed->sendCommand("test1", "log test MEXAGE");
    Fed->requestNextStep();
    Fed->finalize();

    auto llock = mlog.lock();
    bool found = false;
    for (auto& m : llock) {
        if (m.second.find("MEXAGE") != std::string::npos) {
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST(logging, check_log_message_functions)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TRACE);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed->enterExecutingMode();
    Fed->logErrorMessage("test error");
    Fed->logWarningMessage("test warning");
    Fed->logInfoMessage("test info");
    Fed->logDebugMessage("test debug");
    Fed->requestNextStep();
    Fed->finalize();

    auto llock = mlog.lock();
    int order = 0;
    for (auto& m : llock) {
        if (m.second.find("error") != std::string::npos) {
            EXPECT_EQ(m.first, HELICS_LOG_LEVEL_ERROR);
            EXPECT_EQ(order, 0);
            order = 1;
        }
        if (m.second.find("warning") != std::string::npos) {
            EXPECT_EQ(m.first, HELICS_LOG_LEVEL_WARNING);
            EXPECT_EQ(order, 1);
            order = 2;
        }
        if (m.second.find("info") != std::string::npos) {
            EXPECT_EQ(m.first, HELICS_LOG_LEVEL_SUMMARY);
            EXPECT_EQ(order, 2);
            order = 3;
        }
        if (m.second.find("debug") != std::string::npos) {
            EXPECT_GT(m.first, HELICS_LOG_LEVEL_SUMMARY);
            EXPECT_EQ(order, 3);
            order = 4;
        }
    }
}

TEST(logging, check_log_message_levels)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed->enterExecutingMode();
    Fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS, "test MEXAGE1");
    Fed->logMessage(HELICS_LOG_LEVEL_TRACE + 1, "test MEXAGE2");
    Fed->requestNextStep();
    Fed->finalize();

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
    EXPECT_TRUE(found_low && !found_high);
}

TEST(logging, check_log_message_levels_high)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TRACE + 2);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    Fed->enterExecutingMode();
    Fed->logMessage(HELICS_LOG_LEVEL_SUMMARY, "test MEXAGE1");
    Fed->logMessage(HELICS_LOG_LEVEL_TRACE + 1, "test MEXAGE2");
    Fed->requestNextStep();
    Fed->finalize();

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
}

TEST(logging, dumplog)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);
    auto cr = Fed->getCorePointer();
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    cr->setLoggingCallback(helics::gLocalCoreId,
                           [&mlog](int level,
                                   std::string_view /*unused*/,
                                   std::string_view message) {
                               mlog.lock()->emplace_back(level, message);
                           });

    Fed->enterExecutingMode();
    /** We are setting the flag then clearing it
    this will generate 1 and at most 2 messages in the log callback
    Thus the check for this is that there is a least 2 and at most 3 messages
    in the log block, to indicate that the set and clear was successful*/
    Fed->setFlagOption(HELICS_FLAG_DUMPLOG);
    Fed->setFlagOption(HELICS_FLAG_DUMPLOG, false);

    Fed->finalize();
    cr->waitForDisconnect();
    cr.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 2U);
    EXPECT_LE(llock->size(), 3U);
    // this is to check that it has the correct level
    EXPECT_EQ(llock->back().first, HELICS_LOG_LEVEL_DUMPLOG);
}

TEST(logging, timeMonitorFederate1)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST,
                                                "--name=monbroker1 --timemonitor=monitor");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "monbroker1";

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);

    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed->finalize();
    broker.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    bool exec{false};
    bool grant{false};
    bool disconnect{false};
    int timeCount{0};
    for (const auto& message : llock) {
        if (message.second.find("TIME:") != std::string::npos) {
            ++timeCount;
            if (message.second.find("exec granted") != std::string::npos) {
                exec = true;
                continue;
            }
            if (message.second.find("granted time") != std::string::npos) {
                grant = true;
                continue;
            }
            if (message.second.find("disconnected") != std::string::npos) {
                disconnect = true;
                continue;
            }
        }
    }
    EXPECT_EQ(timeCount, 3);
    EXPECT_TRUE(disconnect);
    EXPECT_TRUE(exec);
    EXPECT_TRUE(grant);
}

TEST(logging, timeMonitorFederate2)
{
    auto broker = helics::BrokerFactory::create(
        helics::CoreType::TEST, "--name=monbroker2 --timemonitor=monitor --timemonitorperiod=2s");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "monbroker2";

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);

    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed->finalize();
    broker.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    int grantCount{0};
    for (const auto& message : llock) {
        if (message.second.find("granted time") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    EXPECT_EQ(grantCount, 2);
}

TEST(logging, timeMonitorFederate_command)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=monbroker3");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->sendCommand("root", "monitor monitor");
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "monbroker3";

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);

    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed->finalize();
    broker.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    int grantCount{0};
    for (const auto& message : llock) {
        if (message.second.find("granted time") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    EXPECT_EQ(grantCount, 4);
}

TEST(logging, timeMonitorFederate_command2)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=monbroker4");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->sendCommand("root", "monitor monitor 2 sec");
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "monbroker4";

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);

    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed->finalize();
    broker.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    int grantCount{0};
    for (const auto& message : llock) {
        if (message.second.find("granted time") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    EXPECT_EQ(grantCount, 2);
}

TEST(logging, timeMonitorFederate_swap)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=monbroker4");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view header, std::string_view message) {
            mlog.lock()->emplace_back(level, header, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->sendCommand("root", "monitor monitor 2 sec");
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "monbroker4";

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);
    Fed2->enterExecutingModeAsync();
    Fed->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed->finalize();
    // make sure all communications have gone through
    broker->sendCommand("root", "flush");
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    int grantCount{0};
    for (const auto& message : llock) {
        if (std::get<2>(message).find("granted time") != std::string::npos) {
            ++grantCount;
        }
    }
    EXPECT_EQ(grantCount, 2);
    llock.unlock();
    broker->sendCommand("root", "monitor monitor2");

    Fed2->processCommunication(std::chrono::milliseconds(100));

    auto deps = Fed2->query("dependents");
    int cnt{0};
    while (deps == "[]") {
        Fed2->processCommunication(std::chrono::milliseconds(100));

        deps = Fed2->query("dependents");
        ++cnt;
        if (cnt > 10) {
            break;
        }
    }
    EXPECT_LE(cnt, 10);
    rtime = Fed2->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed2->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed2->finalize();
    broker->waitForDisconnect();
    broker.reset();
    llock = mlog.lock();
    EXPECT_GE(llock->size(), 8U);

    grantCount = 0;
    for (const auto& message : llock) {
        if (std::get<1>(message).find("monitor2") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    // should be two grants and disconnect since we are looking at header but just need to make sure
    // there is at least 1 since test timing can vary here
    EXPECT_GE(grantCount, 1);
}

TEST(logging, log_buffer_broker)
{
    auto broker =
        helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker1 --logbuffer -f2");
    broker->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_TRACE);
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker1";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);

    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();
    auto str = broker->query("broker", "logs");
    Fed2->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_EQ(js["logs"].size(), 10U);
    broker->waitForDisconnect();

    str = broker->query("broker", "logs");
    broker.reset();
    js = helics::fileops::loadJsonStr(str);

    EXPECT_EQ(js["logs"].size(), 10U);
}

TEST(logging, log_buffer_broker2)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST,
                                                "--name=logbroker2 --logbuffer 7 -f2");
    broker->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_TRACE);
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker2";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);

    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();
    auto str = broker->query("broker", "logs");
    Fed2->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_EQ(js["logs"].size(), 7U);
    broker.reset();
}

TEST(logging, log_buffer_broker3)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker3 -f2");
    broker->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_TRACE);
    broker->sendCommand("broker", "logbuffer");
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker3";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);

    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();
    auto str = broker->query("broker", "logs");
    Fed2->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_EQ(js["logs"].size(), 10U);
    broker.reset();
}

TEST(logging, log_buffer_broker4)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker4 -f2");
    broker->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_TRACE);
    broker->sendCommand("broker", "logbuffer 7");
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker4";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);

    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();
    auto str = broker->query("broker", "logs");
    Fed2->finalize();
    broker.reset();

    auto js = helics::fileops::loadJsonStr(str);
    ASSERT_TRUE(js["logs"].is_array());
    EXPECT_EQ(js["logs"].size(), 7U);
}

TEST(logging, log_buffer_fed)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker5");

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker5";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    Fed1->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });

    broker->sendCommand("monitor1", "logbuffer");
    Fed1->enterExecutingMode();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);

    auto str = broker->query("monitor1", "logs");

    auto prop = Fed1->getIntegerProperty(helics::defs::LOG_BUFFER);
    EXPECT_EQ(prop, static_cast<int>(helics::LogBuffer::cDefaultBufferSize));
    EXPECT_TRUE(Fed1->getFlagOption(helics::defs::LOG_BUFFER));
    Fed1->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_GE(js["logs"].size(), 1U);
    broker.reset();
}

TEST(logging, log_buffer_fed2)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker6");

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker6";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    fedInfo.setProperty(helics::defs::Properties::LOG_BUFFER, 3);
    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    Fed1->setLoggingCallback(
        [](int /*level*/, std::string_view /*unused*/, std::string_view /*message*/) {
            // this is mainly so it doesn't overload the console output
        });

    Fed1->enterExecutingMode();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);

    auto str = Fed1->query("logs");

    Fed1->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_GE(js["logs"].size(), 1U);
    EXPECT_LE(js["logs"].size(), 3U);
    broker.reset();
}

TEST(logging, log_buffer_core)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker7");

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker7";
    fedInfo.coreName = "logcore1";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    Fed1->getCorePointer()->setLoggingCallback(helics::gLocalCoreId,
                                               [](int /*level*/,
                                                  std::string_view /*unused*/,
                                                  std::string_view /*message*/) {
                                                   // this is mainly so it doesn't overload the
                                                   // console output
                                               });

    broker->sendCommand("logcore1", "logbuffer");
    Fed1->enterExecutingMode();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);

    auto str = broker->query("logcore1", "logs");

    Fed1->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_GE(js["logs"].size(), 1U);
    broker.reset();
}

TEST(logging, log_buffer_core2)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=logbroker8");

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "logbroker8";
    fedInfo.coreName = "logcore2";
    fedInfo.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_DEBUG);
    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto cr = Fed1->getCorePointer();
    cr->setLoggingCallback(helics::gLocalCoreId,
                           [](int /*level*/,
                              std::string_view /*unused*/,
                              std::string_view /*message*/) {
                               // this is mainly so it doesn't overload the
                               // console output
                           });

    cr->setIntegerProperty(helics::gLocalCoreId, helics::defs::Properties::LOG_BUFFER, 3);
    Fed1->enterExecutingMode();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    auto sz = cr->getIntegerProperty(helics::gLocalCoreId, helics::defs::Properties::LOG_BUFFER);
    EXPECT_EQ(sz, 3);
    auto str = cr->query("core", "logs", HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED);

    Fed1->finalize();
    auto js = helics::fileops::loadJsonStr(str);

    EXPECT_GE(js["logs"].size(), 1U);
    EXPECT_LE(js["logs"].size(), 3U);
    broker.reset();
    cr->waitForDisconnect();
    // test to make sure this works after disconnect
    auto str2 = cr->query("core", "logs", HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED);
    auto js2 = helics::fileops::loadJsonStr(str);

    EXPECT_GE(js2["logs"].size(), 1U);
    EXPECT_LE(js2["logs"].size(), 3U);
    cr.reset();
    helics::cleanupHelicsLibrary();
}

TEST(logging, remote_log_broker)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=broker_rlog");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view source, std::string_view message) {
            mlog.lock()->emplace_back(level, source, message);
        });
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "broker_rlog";
    fedInfo.forceNewCore = true;

    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);
    std::cout << "send command" << std::endl;
    broker->sendCommand("monitor", "remotelog timing");
    std::cout << "send flush query" << std::endl;
    broker->query("root", "global_flush");
    std::cout << "enter exec" << std::endl;
    Fed->enterExecutingMode();
    std::cout << "request time" << std::endl;
    auto rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    std::cout << "finalize" << std::endl;
    Fed->finalize();
    std::cout << "wait for disconnect" << std::endl;
    broker->waitForDisconnect();
    auto llock = mlog.lock();
    int remote_cnt{0};
    for (const auto& lg : llock) {
        if (std::get<1>(lg).find("monitor") != std::string::npos) {
            ++remote_cnt;
        }
    }
    llock.unlock();
    broker.reset();
    EXPECT_GT(remote_cnt, 0);
    helics::cleanupHelicsLibrary();
}

TEST(logging, remote_log_fed)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=broker9");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "broker9";
    fedInfo.forceNewCore = true;
    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);
    Fed->setLoggingCallback([&mlog](int level, std::string_view source, std::string_view message) {
        mlog.lock()->emplace_back(level, source, message);
    });

    Fed->sendCommand("broker9", "remotelog debug");
    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed->query("root", "global_flush");
    Fed->finalize();

    broker.reset();
    auto llock = mlog.lock();
    int remote_cnt{0};
    for (const auto& lg : llock) {
        if (std::get<1>(lg).find("broker9") != std::string::npos ||
            std::get<1>(lg).find("root") != std::string::npos) {
            ++remote_cnt;
        }
    }
    llock.unlock();
    EXPECT_GT(remote_cnt, 0);
}

TEST(logging, remote_log_core)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=broker10");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;

    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "broker10";
    fedInfo.coreName = "core10";
    auto Fed = std::make_shared<helics::Federate>("monitor", fedInfo);
    Fed->getCorePointer()->setLoggingCallback(helics::gLocalCoreId,
                                              [&mlog](int level,
                                                      std::string_view source,
                                                      std::string_view message) {
                                                  mlog.lock()->emplace_back(level, source, message);
                                              });

    Fed->getCorePointer()->sendCommand("broker10",
                                       "remotelog debug",
                                       "",
                                       HelicsSequencingModes::HELICS_SEQUENCING_MODE_DEFAULT);
    Fed->enterExecutingMode();

    auto rtime = Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed->query("root", "global_flush");
    Fed->finalize();

    broker.reset();
    int remote_cnt{0};
    auto llock = mlog.lock();
    for (const auto& lg : llock) {
        if (std::get<1>(lg).find("broker10") != std::string::npos ||
            std::get<1>(lg).find("root") != std::string::npos) {
            ++remote_cnt;
        }
    }
    llock.unlock();
    EXPECT_GT(remote_cnt, 0);
}

TEST(logging, remote_log_multifed)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "-f2 --name=broker12");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view source, std::string_view message) {
            mlog.lock()->emplace_back(level, source, message);
        });
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "broker12";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);
    broker->sendCommand("monitor1", "remotelog timing");
    broker->sendCommand("monitor2", "remotelog timing");

    broker->query("root", "global_flush");
    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();

    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed2->finalize();
    broker->waitForDisconnect();
    broker.reset();
    int remote_cnt1{0};
    int remote_cnt2{0};
    auto llock = mlog.lock();
    for (const auto& lg : llock) {
        if (std::get<1>(lg).find("monitor1") != std::string::npos) {
            ++remote_cnt1;
        }
        if (std::get<1>(lg).find("monitor2") != std::string::npos) {
            ++remote_cnt2;
        }
    }
    llock.unlock();
    EXPECT_GT(remote_cnt1, 0);
    EXPECT_GT(remote_cnt2, 0);
}

TEST(logging, remote_log_multiObjects)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "-f2 --name=broker13");
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view source, std::string_view message) {
            mlog.lock()->emplace_back(level, source, message);
        });
    broker->connect();

    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.broker = "broker13";

    auto Fed1 = std::make_shared<helics::Federate>("monitor1", fedInfo);
    gmlc::libguarded::guarded<std::vector<std::tuple<int, std::string, std::string>>> mlogFed;
    Fed1->setLoggingCallback(
        [&mlogFed](int level, std::string_view source, std::string_view message) {
            mlogFed.lock()->emplace_back(level, source, message);
        });

    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fedInfo);
    broker->sendCommand("monitor1", "remotelog timing");
    broker->sendCommand("monitor2", "remotelog timing");

    Fed1->sendCommand("broker13", "remotelog debug");
    Fed1->sendCommand("monitor2", "remotelog timing");

    broker->query("root", "global_flush");
    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingMode();
    Fed2->enterExecutingModeComplete();

    auto rtime = Fed1->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed1->finalize();

    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed2->finalize();
    broker->waitForDisconnect();
    broker.reset();
    int remote_cnt1{0};
    int remote_cnt2{0};
    auto llock = mlog.lock();
    for (const auto& lg : llock) {
        if (std::get<1>(lg).find("monitor1") != std::string::npos) {
            ++remote_cnt1;
        }
        if (std::get<1>(lg).find("monitor2") != std::string::npos) {
            ++remote_cnt2;
        }
    }
    llock.unlock();
    EXPECT_GT(remote_cnt1, 0);
    EXPECT_GT(remote_cnt2, 0);

    int remote_cntFed2{0};
    int remote_cntBroker{0};
    auto llock2 = mlogFed.lock();

    for (const auto& lg : llock2) {
        if (std::get<1>(lg).find("broker12") != std::string::npos ||
            std::get<1>(lg).find("root") != std::string::npos) {
            ++remote_cntBroker;
        }
        if (std::get<1>(lg).find("monitor2") != std::string::npos) {
            ++remote_cntFed2;
        }
    }
    llock2.unlock();
    EXPECT_GT(remote_cntBroker, 0);
    EXPECT_GT(remote_cntFed2, 0);
}
