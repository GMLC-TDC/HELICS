/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"
#include "helics/external/filesystem.hpp"

#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <gtest/gtest.h>
#include <thread>

/** these test cases test out user-directed logging functionality
 */

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST(logging, basic_logging)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    if (ghc::filesystem::exists(lfilename)) {
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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --logfile logfile.txt --fileloglevel=timing";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    Fed->enterExecutingMode();
    Fed->finalize();
    auto cr{Fed->getCorePointer()};
    Fed.reset();

    EXPECT_TRUE(ghc::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
    helics::cleanupHelicsLibrary();
    std::error_code ec;
    ghc::filesystem::remove(lfilename, ec);
}

TEST(logging, file_logging_p2)
{
    const std::string lfilename = "logfile2.txt";
    if (ghc::filesystem::exists(lfilename)) {
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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --fileloglevel=timing";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);
    auto cr = Fed->getCorePointer();

    cr->setLogFile(lfilename);
    Fed->enterExecutingMode();
    Fed->finalize();

    Fed.reset();
    EXPECT_TRUE(ghc::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
    helics::cleanupHelicsLibrary();
    std::error_code ec;
    ghc::filesystem::remove(lfilename, ec);
}

TEST(logging, check_log_message)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TRACE);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TIMING);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_TRACE + 2);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

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
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_NO_PRINT);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);
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
    EXPECT_EQ(llock->back().first, -10);  // the -10 should have a level enum value at some point in
                                          // the future as part of the debugging improvements
}


TEST(logging, timeMonitorFederate1)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST, "--name=monbroker1 --timemonitor=monitor");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.broker = "monbroker1";
    
    auto Fed = std::make_shared<helics::Federate>("monitor", fi);

    

    Fed->enterExecutingMode();

    auto rtime=Fed->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    Fed->finalize();
    broker.reset();
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    bool exec{false};
    bool grant{false};
    bool disconnect{false};
    int timeCount{0};
    for (const auto &message:llock) {
        if (message.second.find("TIME:")!=std::string::npos) {
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
    auto broker = helics::BrokerFactory::create(helics::CoreType::TEST,
                                                "--name=monbroker2 --timemonitor=monitor --timemonitorperiod=2s");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.broker = "monbroker2";

    auto Fed = std::make_shared<helics::Federate>("monitor", fi);

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
    auto broker = helics::BrokerFactory::create(
        helics::CoreType::TEST, "--name=monbroker3");
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->sendCommand("root", "monitor monitor");
    broker->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.broker = "monbroker3";

    auto Fed = std::make_shared<helics::Federate>("monitor", fi);

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

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.broker = "monbroker4";

    auto Fed = std::make_shared<helics::Federate>("monitor", fi);

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
    gmlc::libguarded::guarded<std::vector<std::tuple<int,std::string, std::string>>> mlog;
    broker->setLoggingCallback(
        [&mlog](int level, std::string_view header, std::string_view message) {
            mlog.lock()->emplace_back(level,header, message);
        });
    broker->setLoggingLevel(HELICS_LOG_LEVEL_SUMMARY);
    broker->sendCommand("root", "monitor monitor 2 sec");
    broker->connect();

    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.broker = "monbroker4";

    auto Fed = std::make_shared<helics::Federate>("monitor", fi);
    auto Fed2 = std::make_shared<helics::Federate>("monitor2", fi);
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
    
    auto llock = mlog.lock();
    EXPECT_GE(llock->size(), 4U);

    int grantCount{0};
    for (const auto& message : llock) {
        if (std::get<2>(message).find("granted time") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    EXPECT_EQ(grantCount, 2);
    llock.unlock();
    broker->sendCommand("root", "monitor monitor2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    rtime = Fed2->requestTime(1.0);
    EXPECT_EQ(rtime, 1.0);
    rtime = Fed2->requestTime(2.0);
    EXPECT_EQ(rtime, 2.0);
    rtime = Fed2->requestTime(3.0);
    EXPECT_EQ(rtime, 3.0);
    rtime = Fed2->requestTime(4.0);
    EXPECT_EQ(rtime, 4.0);
    Fed2->finalize();

    llock = mlog.lock();
    EXPECT_GE(llock->size(), 8U);

    grantCount=0;
    for (const auto& message : llock) {
        if (std::get<1>(message).find("monitor2") != std::string::npos) {
            ++grantCount;
            continue;
        }
    }
    // should be two grants and disconnect since we are looking at header
    EXPECT_EQ(grantCount, 3);
}
