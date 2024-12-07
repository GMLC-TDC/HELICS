/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/apps/RestApiConnection.hpp"
#include "helics/apps/helicsWebServer.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/coreTypeOperations.hpp"
#include "helics/network/loadCores.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>

using namespace helics::fileops;
namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

const constexpr char localhost[] = "localhost";

class httpTest: public ::testing::Test {
  protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite()
    {
        helics::loadCores();
        webs = std::make_shared<helics::apps::WebServer>();
        webs->enableHttpServer(true);
        config["http"] = nlohmann::json::object();
        config["http"]["port"] = 26242;

        webs->startServer(&config, webs);

        connection.connect(localhost, "26242");
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite()
    {
        connection.disconnect();
        webs->stopServer();
        helics::BrokerFactory::terminateAllBrokers();
    }

    // You can define per-test set-up logic as usual.
    void SetUp() final {}

    static std::string sendGet(const std::string& target) { return connection.sendGet(target); }

    static std::string sendCommand(boost::beast::http::verb command,
                                   const std::string& target,
                                   const std::string& body)
    {
        return connection.sendCommand(command, target, body);
    }

    static std::shared_ptr<helics::Broker> addBroker(helics::CoreType ctype,
                                                     const std::string& init)
    {
        auto brk = helics::BrokerFactory::create(ctype, init);
        if (brk) {
            brks.push_back(brk);
        }
        return brk;
    }

    static std::shared_ptr<helics::Core> addCore(helics::CoreType ctype, const std::string& init)
    {
        auto cr = helics::CoreFactory::create(ctype, init);
        if (cr) {
            cores.push_back(cr);
        }
        return cr;
    }
    static void clearBrokers()
    {
        for (auto& brk : brks) {
            if (brk) {
                brk->disconnect();
            }
        }
        brks.clear();
        helics::BrokerFactory::cleanUpBrokers();
    }
    // You can define per-test tear-down logic as usual.
    void TearDown() final {}

  private:
    // Some expensive resource shared by all tests.
    static std::shared_ptr<helics::apps::WebServer> webs;
    static helics::apps::RestApiConnection connection;

    static std::vector<std::shared_ptr<helics::Broker>> brks;
    static std::vector<std::shared_ptr<helics::Core>> cores;

    static nlohmann::json config;
};

std::shared_ptr<helics::apps::WebServer> httpTest::webs;
std::vector<std::shared_ptr<helics::Broker>> httpTest::brks;
std::vector<std::shared_ptr<helics::Core>> httpTest::cores;
helics::apps::RestApiConnection httpTest::connection(localhost);
nlohmann::json httpTest::config;

TEST_F(httpTest, test1)
{
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 0U);
}

#ifdef HELICS_ENABLE_ZMQ_CORE
constexpr helics::CoreType tCore = helics::CoreType::ZMQ;
#    define CORE1 "zmq"
#    ifdef HELICS_ENABLE_TCP_CORE
#        define CORE2 "TCP"
#    else
#        define CORE2 "ZMQ"
#    endif
#elif defined(HELICS_ENABLE_TCP_CORE)
constexpr helics::CoreType tCore = helics::CoreType::TCP;
#    define CORE1 "tcp"
#    ifdef HELICS_ENABLE_UDP_CORE
#        define CORE2 "UDP"
#    else
#        define CORE2 "TCP"
#    endif
#else
constexpr helics::CoreType tCore = helics::CoreType::TEST;
#    define CORE1 "test"
#    define CORE2 "TEST"
#endif

TEST_F(httpTest, single)
{
    addBroker(tCore, "--name=brk1");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 1U);
    EXPECT_EQ(val["brokers"][0]["name"].get<std::string>(), "brk1");
}

TEST_F(httpTest, pair)
{
    addBroker(helics::CoreType::TEST, "--name=brk2");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 2U);
    EXPECT_EQ(val["brokers"][0]["name"].get<std::string>(), "brk1");
    EXPECT_EQ(val["brokers"][1]["name"].get<std::string>(), "brk2");
}

TEST_F(httpTest, single_info)
{
    auto result = sendGet("brk1");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 0U);
    EXPECT_EQ(val["cores"].size(), 0U);
    EXPECT_EQ(val["federates"].size(), 0U);
    EXPECT_EQ(val["attributes"]["name"].get<std::string>(), "brk1");
    EXPECT_EQ(val["state"].get<std::string>(), "connected");
}

TEST_F(httpTest, get_global_time)
{
    // test to make sure the default target to root is working
    auto result = sendGet("global_time");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 0U);
}

TEST_F(httpTest, singleNonJson)
{
    auto result = sendGet("brk1/isconnected");
    EXPECT_EQ(result, "true");
}

TEST_F(httpTest, garbage)
{
    auto result = sendGet("garbage");
    EXPECT_TRUE(result.find("not") != std::string::npos);
}

TEST_F(httpTest, bad_request)
{
    auto result = sendCommand(http::verb::copy, "/search/brk2", "");
    EXPECT_TRUE(result.find("Unknown") != std::string::npos);
}

TEST_F(httpTest, not_found)
{
    auto result = sendGet("/broker7");
    EXPECT_NE(result.find("not"), std::string::npos);

    result = sendGet("/brk2/ch5");
    EXPECT_NE(result.find("not found"), std::string::npos);
}

TEST_F(httpTest, core)
{
    auto cr = addCore(helics::CoreType::TEST, "--name=cr1 -f2");
    EXPECT_TRUE(cr->connect());

    auto result = sendGet("brk2");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    if (val["cores"].empty()) {
        // on occasion the core might not be registered with the broker in low CPU count systems
        // so we need to wait.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        result = sendGet("brk2");
        EXPECT_FALSE(result.empty());
        val = loadJson(result);
    }
    ASSERT_EQ(val["cores"].size(), 1U);
    EXPECT_EQ(val["cores"][0]["attributes"]["name"].get<std::string>(), "cr1");

    auto result2 = sendCommand(http::verb::search, "/search/brk2", "query=current_state");
    EXPECT_EQ(result, result2);

    result = sendGet("brk2/cr1");
    val = loadJson(result);
    EXPECT_EQ(val["attributes"]["name"].get<std::string>(), "cr1");

    result2 = sendGet("brk2/cr1/current_state");
    EXPECT_EQ(result, result2);

    result2 = sendGet("brk2/cr1?query=current_state");
    EXPECT_EQ(result, result2);

    result2 = sendGet("brk2?query=current_state&target=cr1");
    EXPECT_EQ(result, result2);
    result2 = sendGet("?broker=brk2&query=current_state&target=cr1");
    EXPECT_EQ(result, result2);

    result2 = sendGet("/cr1?broker=brk2&query=current_state");
    EXPECT_EQ(result, result2);

    result2 = sendGet("/cr1/current_state?broker=brk2");
    EXPECT_EQ(result, result2);
}

TEST_F(httpTest, coreBody)
{
    auto result = sendGet("brk2/cr1");

    auto result2 = sendCommand(http::verb::search, "brk2/cr1", "query=current_state");
    EXPECT_EQ(result, result2);
    result2 = sendCommand(http::verb::search, "/brk2/cr1", "query=current_state");
    EXPECT_EQ(result, result2);
    result2 = sendCommand(http::verb::search, "/brk2/cr1/", "query=current_state");
    EXPECT_EQ(result, result2);
    result2 = sendCommand(http::verb::search, "//brk2/cr1/", "query=current_state");
    EXPECT_EQ(result, result2);
    result2 = sendCommand(http::verb::search, "brk2", "query=current_state&target=cr1");
    EXPECT_EQ(result, result2);
    result2 = sendCommand(http::verb::search, "/", "broker=brk2&query=current_state&target=cr1");
    EXPECT_EQ(result, result2);

    result2 =
        sendCommand(http::verb::search, "query", "broker=brk2&query=current_state&target=cr1");
    EXPECT_EQ(result, result2);
    result2 =
        sendCommand(http::verb::search, "/query", "broker=brk2&query=current_state&target=cr1");
    EXPECT_EQ(result, result2);
    result2 =
        sendCommand(http::verb::search, "/query/", "broker=brk2&query=current_state&target=cr1");
    EXPECT_EQ(result, result2);

    result2 =
        sendCommand(http::verb::head, "/query/?broker=brk2&query=current_state&target=cr1", "");
    EXPECT_EQ(std::to_string(result.size()), result2);

    result2 = sendCommand(http::verb::search, "/cr1", "broker=brk2&query=current_state");
    EXPECT_EQ(result, result2);

    result2 = sendCommand(http::verb::search, "/cr1/current_state", "broker=brk2");
    EXPECT_EQ(result, result2);
}

TEST_F(httpTest, post)
{
    std::string init = "type=" CORE2;
    auto result = sendCommand(http::verb::post, "brk3", init);
    auto val = loadJson(result);
    EXPECT_EQ(val["broker"], "brk3");
    EXPECT_EQ(val["type"], CORE2);
    result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 3U);
    EXPECT_EQ(val["brokers"][2]["name"].get<std::string>(), "brk3");
}

TEST_F(httpTest, deleteBroker)
{
    sendCommand(http::verb::delete_, "brk3", "");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 2U);
    sendCommand(http::verb::delete_, "/delete", "broker=brk1");
    result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 1U);
}

TEST_F(httpTest, createBrokerUUID)
{
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    auto brksize = val["brokers"].size();
    std::string init = "core_type=" CORE1 "&num_feds=2";
    result = sendCommand(http::verb::post, "/", init);
    val = loadJson(result);
    EXPECT_TRUE(val["broker_uuid"].is_string());
    auto uuid = val["broker_uuid"].get<std::string>();
    result = sendGet(std::string("/?uuid=") + uuid);
    val = loadJson(result);
    EXPECT_TRUE(val["status"].get<bool>());

    sendCommand(http::verb::delete_, "/", std::string("broker_uuid=") + uuid);

    result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), brksize);
}

TEST_F(httpTest, coreJson)
{
    auto result = sendGet("brk2/cr1");

    nlohmann::json v1;
    v1["query"] = "current_state";
    auto result2 = sendCommand(http::verb::search, "brk2/cr1", generateJsonString(v1));
    EXPECT_EQ(result, result2);

    v1["target"] = "cr1";

    result2 = sendCommand(http::verb::search, "brk2", generateJsonString(v1));
    EXPECT_EQ(result, result2);
    v1["broker"] = "brk2";
    result2 = sendCommand(http::verb::search, "/", generateJsonString(v1));
    EXPECT_EQ(result, result2);

    result2 = sendCommand(http::verb::search, "query", generateJsonString(v1));
    EXPECT_EQ(result, result2);

    result2 = sendCommand(http::verb::post, "query", generateJsonString(v1));
    EXPECT_EQ(result, result2);
}

TEST_F(httpTest, deleteJson)
{
    nlohmann::json v1;
    v1["broker"] = "brk2";
    sendCommand(http::verb::post, "delete", generateJsonString(v1));
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].is_array());
    EXPECT_EQ(val["brokers"].size(), 0U);
}

TEST_F(httpTest, timeBlock)
{
    const std::string init = std::string("type=") + std::string(CORE1) + "&log_level=timing";
    sendCommand(http::verb::post, "brk_timer", init);
    auto cr = addCore(tCore, "--name=c_timer -f1 --broker=brk_timer");
    EXPECT_TRUE(cr->connect());

    helics::ValueFederate vFed("fed1", cr);
    sendCommand(http::verb::post, "brk_timer/barrier", "time=2");

    vFed.enterExecutingMode();
    auto treq = vFed.requestTime(1.75);
    EXPECT_EQ(treq, 1.75);
    vFed.requestTimeAsync(3.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(vFed.isAsyncOperationCompleted());
    sendCommand(http::verb::post, "brk_timer/barrier", "time=5");
    auto rtime = vFed.requestTimeComplete();
    EXPECT_EQ(rtime, 3.0);

    vFed.requestTimeAsync(6.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(vFed.isAsyncOperationCompleted());
    sendCommand(http::verb::post, "brk_timer/barrier", "time=-1");
    rtime = vFed.requestTimeComplete();
    EXPECT_EQ(rtime, 6.0);

    auto brk = helics::BrokerFactory::findBroker("brk_timer");
    EXPECT_TRUE(brk);

    auto res = brk->query("brk_timer", "config");

    EXPECT_TRUE(res.find("\"timing\"") != std::string::npos);
    brk.reset();

    vFed.finalize();
    nlohmann::json v1;
    v1["broker"] = "brk_timer";
    sendCommand(http::verb::post, "delete", generateJsonString(v1));
}

TEST_F(httpTest, healthcheck)
{
    auto result = sendGet("healthcheck");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["success"].get<bool>());
}
