/*
Copyright (c) 2017-2021,
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

TEST(logging_tests, basic_logging)
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

TEST(logging_tests, file_logging)
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

TEST(logging_tests, file_logging_p2)
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

TEST(logging_tests, check_log_message)
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

TEST(logging_tests, check_log_message_command)
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

TEST(logging_tests, check_log_message_functions)
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

TEST(logging_tests, check_log_message_levels)
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

TEST(logging_tests, check_log_message_levels_high)
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

TEST(logging_tests, dumplog)
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

TEST(logging_tests, grant_timeout)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --name gtcore1";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.coreInitString.clear();
    fi.coreName = "gtcore1";
    fi.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.3);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    auto llock = mlog.lock();
    while (llock->empty()) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }
    
    EXPECT_NE((*llock)[0].second.find("grant timeout"), std::string::npos);
    EXPECT_NE((*llock)[0].second.find("131072"), std::string::npos);

    Fed1->requestTime(3.0);
    auto res=Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(logging_tests, grant_timeout_phase2)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --name gtcore1";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.coreInitString.clear();
    fi.coreName = "gtcore1";
    fi.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.2);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto llock = mlog.lock();
    while (llock->size()<2) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[0].second.find("grant timeout"), std::string::npos);
    EXPECT_NE((*llock)[0].second.find("131072"), std::string::npos);
    EXPECT_NE((*llock)[1].second.find("stage 2"), std::string::npos);
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}

TEST(logging_tests, grant_timeout_phase3)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --name gtcore1";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.coreInitString.clear();
    fi.coreName = "gtcore1";
    fi.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.1);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    auto llock = mlog.lock();
    while (llock->size() < 4) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[2].second.find("stage 3"), std::string::npos);
    EXPECT_NE((*llock)[3].second.find("TIME DEBUGGING"), std::string::npos);
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}


TEST(logging_tests, grant_timeout_phase4)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --name gtcore1";
    fi.setProperty(helics::defs::Properties::LOG_LEVEL, HELICS_LOG_LEVEL_SUMMARY);

    auto Fed1 = std::make_shared<helics::ValueFederate>("test1", fi);
    fi.coreInitString.clear();
    fi.coreName = "gtcore1";
    fi.setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 0.1);
    auto Fed2 = std::make_shared<helics::ValueFederate>("test2", fi);
    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed2->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed1->registerGlobalPublication<double>("pub1");
    Fed2->registerSubscription("pub1");

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed2->requestTimeAsync(2.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    auto llock = mlog.lock();
    while (llock->size() < 5) {
        llock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        llock = mlog.lock();
    }

    EXPECT_NE((*llock)[4].second.find("stage 4"), std::string::npos);
    Fed1->requestTime(3.0);
    auto res = Fed2->requestTimeComplete();
    EXPECT_EQ(res, 2.0);
    Fed1->finalize();
    Fed2->finalize();
}
