/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"

#include <filesystem>
#include <fstream>
#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <vector>

/** these test cases test out user-directed logging functionality
 */

#define CORE_TYPE_TO_TEST helics::CoreType::TEST

TEST(profiling_tests, basic)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --profiler=log";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    auto cr = Fed->getCorePointer();
    cr->setLoggingCallback(helics::gLocalCoreId,
                           [&mlog](int level,
                                   std::string_view /*unused*/,
                                   std::string_view message) {
                               mlog.lock()->emplace_back(level, message);
                           });

    cr.reset();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, broker_basic)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString =
        "--brokerinitstring=\"--profiler=log --name=prbroker\" --autobroker --broker=prbroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    auto br = helics::BrokerFactory::findBroker("prbroker");
    EXPECT_TRUE(br);

    br->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    br.reset();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, broker_basic_no_flag)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString =
        "--brokerinitstring=\"--name=prbroker2\" --autobroker --broker=prbroker2";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    auto br = helics::BrokerFactory::findBroker("prbroker2");
    EXPECT_TRUE(br);

    br->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    br.reset();
    Fed->setFlagOption(helics::defs::PROFILING);
    Fed->enterExecutingMode();
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, fed_capture)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed->setFlagOption(helics::defs::LOCAL_PROFILING_CAPTURE);
    Fed->setFlagOption(helics::defs::PROFILING);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, fed_capture2)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker";
    fedInfo.setFlagOption(helics::defs::LOCAL_PROFILING_CAPTURE);
    fedInfo.setFlagOption(helics::defs::PROFILING);
    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, save_file)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --profiler=save_profile.txt";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    Fed->enterExecutingMode();
    Fed->finalize();
    helics::cleanupHelicsLibrary();
    if (!std::filesystem::exists("save_profile.txt")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        helics::cleanupHelicsLibrary();
    }
    ASSERT_TRUE(std::filesystem::exists("save_profile.txt"));

    std::vector<std::string> mlog;
    std::ifstream in("save_profile.txt");
    // Check if object is valid
    ASSERT_TRUE(in) << "Cannot open save_profile.txt";

    std::string str;
    // Read the next line from File until it reaches the end.
    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if (!str.empty()) {
            mlog.push_back(str);
        }
    }
    // Close The File
    in.close();

    ASSERT_TRUE(!mlog.empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog) {
        if (logM.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
    std::filesystem::remove("save_profile.txt");
}

TEST(profiling_tests, save_file_append)
{
    {
        std::ofstream out("save_profile_app.txt");
        out << "APPENDING_TO_FILE" << std::endl;
    }
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString = "--autobroker --profiler_append=save_profile_app.txt";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    Fed->enterExecutingMode();
    Fed->finalize();
    helics::cleanupHelicsLibrary();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    helics::cleanupHelicsLibrary();

    std::vector<std::string> mlog;
    std::ifstream in("save_profile_app.txt");
    // Check if object is valid
    ASSERT_TRUE(in) << "Cannot open save_profile_app.txt";

    std::string str;
    // Read the next line from File until it reaches the end.
    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if (!str.empty()) {
            mlog.push_back(str);
        }
    }
    // Close The File
    in.close();

    ASSERT_TRUE(!mlog.empty());
    bool hasMarker{false};
    EXPECT_NE(mlog.front().find("APPENDING_TO_FILE"), std::string::npos);
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog) {
        if (logM.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
    std::filesystem::remove("save_profile_app.txt");
}

TEST(profiling_tests, broker_file_save)
{
    helics::FederateInfo fedInfo(CORE_TYPE_TO_TEST);
    fedInfo.coreInitString =
        "--brokerinitstring=\"--profiler=save_profile2.txt --name=prbroker\" --autobroker --broker=prbroker";

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    Fed->enterExecutingMode();
    Fed->finalize();

    helics::cleanupHelicsLibrary();
    if (!std::filesystem::exists("save_profile2.txt")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        helics::cleanupHelicsLibrary();
    }
    ASSERT_TRUE(std::filesystem::exists("save_profile2.txt"));

    std::vector<std::string> mlog;
    std::ifstream in("save_profile2.txt");
    // Check if object is valid
    ASSERT_TRUE(in) << "Cannot open save_profile2.txt";

    std::string str;
    // Read the next line from File until it reaches the end.
    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if (!str.empty()) {
            mlog.push_back(str);
        }
    }
    // Close The File
    in.close();

    ASSERT_TRUE(!mlog.empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog) {
        if (logM.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
    std::filesystem::remove("save_profile2.txt");
}

TEST(profiling_tests, config)
{
    auto fedInfo = helics::loadFederateInfo(TEST_DIR "/../test_files/profiling_config.json");

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    auto mlog =
        std::make_shared<gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>>>();

    auto cr = Fed->getCorePointer();
    cr->setLoggingCallback(helics::gLocalCoreId,
                           [mlog](int level,
                                  std::string_view /*unused*/,
                                  std::string_view message) {
                               mlog->lock()->emplace_back(level, message);
                           });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    cr.reset();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog->lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog->lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}

TEST(profiling_tests, config2)
{
    auto fedInfo = helics::loadFederateInfo(TEST_DIR "/../test_files/profiling_config2.json");

    auto Fed = std::make_shared<helics::Federate>("test1", fedInfo);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->enterExecutingMode();
    Fed->setFlagOption(helics::defs::PROFILING_MARKER);
    Fed->finalize();

    ASSERT_TRUE(!mlog.lock()->empty());
    bool hasMarker{false};
    std::vector<std::int64_t> timeValues;
    for (const auto& logM : mlog.lock()) {
        if (logM.second.find("MARKER") != std::string::npos) {
            hasMarker = true;
        } else if (logM.second.find("<PROFILING>") != std::string::npos) {
            std::smatch ml;
            std::regex ptime("<([^|>]*)></PROFILING>");
            if (std::regex_search(logM.second, ml, ptime)) {
                timeValues.push_back(std::stoll(ml[1]));
            }
        }
    }
    EXPECT_TRUE(hasMarker);
    std::int64_t current = 0LL;
    bool increasing{true};
    for (auto st : timeValues) {
        if (st < current) {
            increasing = false;
        }
        current = st;
    }
    EXPECT_TRUE(increasing);
}
