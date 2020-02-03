/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/common/logger.h"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"
#include "helics/external/filesystem.hpp"

#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <gtest/gtest.h>

/** these test cases test out user-directed logging functionality
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST

TEST(logging_tests, basic_logging)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::log_level, 5);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback([&mlog](int level, const std::string&, const std::string& message) {
        mlog.lock()->emplace_back(level, message);
    });
    Fed->logMessage(3, "test log message");
    Fed->enterExecutingMode();
    Fed->finalize();

    EXPECT_TRUE(!mlog.lock()->empty());
}

TEST(logging_tests, file_logging)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --logfile logfile.txt --fileloglevel=5";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);
    const std::string lfilename = "logfile.txt";
    Fed->enterExecutingMode();
    Fed->finalize();
    auto cr = Fed->getCorePointer();
    Fed.reset();

    EXPECT_TRUE(ghc::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
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

TEST(logging_tests, file_logging_p2)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --fileloglevel=5";

    auto Fed = std::make_shared<helics::Federate>("test1", fi);
    auto cr = Fed->getCorePointer();
    const std::string lfilename = "logfile2.txt";
    cr->setLogFile(lfilename);
    Fed->enterExecutingMode();
    Fed->finalize();

    Fed.reset();
    EXPECT_TRUE(ghc::filesystem::exists(lfilename));
    cr->waitForDisconnect();
    cr.reset();
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

TEST(logging_tests, check_log_message)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::log_level, 5);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback([&mlog](int level, const std::string&, const std::string& message) {
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

TEST(logging_tests, check_log_message_functions)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::log_level, helics_log_level_trace);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback([&mlog](int level, const std::string&, const std::string& message) {
        mlog.lock()->emplace_back(level, message);
    });

    Fed->enterExecutingMode();
    Fed->logErrorMessage("test ERROR");
    Fed->logWarningMessage("test WARNING");
    Fed->logInfoMessage("test INFO");
    Fed->logDebugMessage("test DEBUG");
    Fed->requestNextStep();
    Fed->finalize();

    auto llock = mlog.lock();
    int order = 0;
    for (auto& m : llock) {
        if (m.second.find("ERROR") != std::string::npos) {
            EXPECT_EQ(m.first, helics_log_level_error);
            EXPECT_EQ(order, 0);
            order = 1;
        }
        if (m.second.find("WARNING") != std::string::npos) {
            EXPECT_EQ(m.first, helics_log_level_warning);
            EXPECT_EQ(order, 1);
            order = 2;
        }
        if (m.second.find("INFO") != std::string::npos) {
            EXPECT_EQ(m.first, helics_log_level_summary);
            EXPECT_EQ(order, 2);
            order = 3;
        }
        if (m.second.find("DEBUG") != std::string::npos) {
            EXPECT_GT(m.first, helics_log_level_summary);
            EXPECT_EQ(order, 3);
            order = 4;
        }
    }
}

TEST(logging_tests, check_log_message_levels)
{
    helics::FederateInfo fi(CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty(helics::defs::log_level, 5);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback([&mlog](int level, const std::string&, const std::string& message) {
        mlog.lock()->emplace_back(level, message);
    });

    Fed->enterExecutingMode();
    Fed->logMessage(3, "test MEXAGE1");
    Fed->logMessage(8, "test MEXAGE2");
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
    fi.setProperty(helics::defs::log_level, 9);

    auto Fed = std::make_shared<helics::Federate>("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback([&mlog](int level, const std::string&, const std::string& message) {
        mlog.lock()->emplace_back(level, message);
    });

    Fed->enterExecutingMode();
    Fed->logMessage(3, "test MEXAGE1");
    Fed->logMessage(8, "test MEXAGE2");
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
