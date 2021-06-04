/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/apps/helicsWebServer.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/coreTypeOperations.hpp"
#include "helics/network/loadCores.hpp"

#include "gtest/gtest.h"
#include <algorithm>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/container/flat_map.hpp>
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

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;  // from <boost/beast/http.hpp>
namespace net = boost::asio;  // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

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
        config["http"] = Json::objectValue;
        config["http"]["port"] = 26242;

        webs->startServer(&config);

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        stream = std::make_unique<beast::tcp_stream>(ioc);  // NOLINT

        // Look up the domain name
        auto const results = resolver.resolve(localhost, "26242");

        // Make the connection on the IP address we get from a lookup
        stream->connect(results);
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite()
    {
        beast::error_code ec;
        stream->socket().shutdown(tcp::socket::shutdown_both, ec);
        webs->stopServer();
        helics::BrokerFactory::terminateAllBrokers();
        stream.reset();
    }

    // You can define per-test set-up logic as usual.
    void SetUp() final {}

    static std::string sendGet(const std::string& target)
    {
        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, localhost);
        req.set(http::field::user_agent, "HELICS_HTTP_TEST");

        // Send the HTTP request to the remote host
        http::write(*stream, req);

        // Declare a container to hold the response
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(*stream, buffer, res);
        return res.body();
    }

    static std::string
        sendCommand(http::verb command, const std::string& target, const std::string& body)
    {
        // Set up an HTTP command message
        http::request<http::string_body> req{command, target, 11};
        req.set(http::field::host, localhost);
        req.set(http::field::user_agent, "HELICS_HTTP_TEST");
        if (!body.empty()) {
            if (body.front() == '{') {
                req.set(http::field::content_type, "application/json");
            } else {
                req.set(http::field::content_type, "text/plain");
            }
            req.body() = body;
            req.prepare_payload();
        }
        // Send the HTTP request to the remote host
        http::write(*stream, req);
        if (command == http::verb::head) {
            // Declare a container to hold the response
            http::response_parser<http::empty_body> res;
            res.skip(true);
            // Receive the HTTP response
            http::read(*stream, buffer, res);
            return std::to_string(*res.content_length());
        }
        // Declare a container to hold the response
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(*stream, buffer, res);
        if (res.result() == http::status::not_found) {
            return "#invalid";
        }
        return res.body();
    }

    static std::shared_ptr<helics::Broker> addBroker(helics::core_type ctype,
                                                     const std::string& init)
    {
        auto brk = helics::BrokerFactory::create(ctype, init);
        if (brk) {
            brks.push_back(brk);
        }
        return brk;
    }

    static std::shared_ptr<helics::Core> addCore(helics::core_type ctype, const std::string& init)
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
    static net::io_context ioc;

    // These objects perform our I/O

    static std::unique_ptr<beast::tcp_stream> stream;
    static beast::flat_buffer buffer;

    static std::vector<std::shared_ptr<helics::Broker>> brks;
    static std::vector<std::shared_ptr<helics::Core>> cores;

    static Json::Value config;
};

std::shared_ptr<helics::apps::WebServer> httpTest::webs;
std::vector<std::shared_ptr<helics::Broker>> httpTest::brks;
std::vector<std::shared_ptr<helics::Core>> httpTest::cores;
beast::flat_buffer httpTest::buffer;
std::unique_ptr<beast::tcp_stream> httpTest::stream;
net::io_context httpTest::ioc;
Json::Value httpTest::config;

TEST_F(httpTest, test1)
{
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 0U);
}

TEST_F(httpTest, single)
{
    addBroker(helics::core_type::ZMQ, "--name=brk1");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 1U);
    EXPECT_STREQ(val["brokers"][0]["name"].asCString(), "brk1");
}

TEST_F(httpTest, pair)
{
    addBroker(helics::core_type::TEST, "--name=brk2");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 2U);
    EXPECT_STREQ(val["brokers"][0]["name"].asCString(), "brk1");
    EXPECT_STREQ(val["brokers"][1]["name"].asCString(), "brk2");
}

TEST_F(httpTest, single_info)
{
    auto result = sendGet("brk1");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 0U);
    EXPECT_EQ(val["cores"].size(), 0U);
    EXPECT_EQ(val["federates"].size(), 0U);
    EXPECT_STREQ(val["name"].asCString(), "brk1");
    EXPECT_STREQ(val["state"].asCString(), "connected");
}

TEST_F(httpTest, get_global_time)
{
    // test to make sure the default target to root is working
    auto result = sendGet("global_time");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
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
    EXPECT_TRUE(result.find("not") != std::string::npos);

    result = sendGet("/brk2/ch5");
    EXPECT_TRUE(result.find("not found") != std::string::npos);
}

TEST_F(httpTest, core)
{
    auto cr = addCore(helics::core_type::TEST, "--name=cr1 -f2");
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
    EXPECT_STREQ(val["cores"][0]["name"].asCString(), "cr1");

    auto result2 = sendCommand(http::verb::search, "/search/brk2", "query=current_state");
    EXPECT_EQ(result, result2);

    result = sendGet("brk2/cr1");
    val = loadJson(result);
    EXPECT_STREQ(val["name"].asCString(), "cr1");

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
    sendCommand(http::verb::post, "brk3", "type=TCP");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 3U);
    EXPECT_STREQ(val["brokers"][2]["name"].asCString(), "brk3");
}

TEST_F(httpTest, deleteBroker)
{
    sendCommand(http::verb::delete_, "brk3", "");
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 2U);
    sendCommand(http::verb::delete_, "/delete", "broker=brk1");
    result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 1U);
}

TEST_F(httpTest, createBrokerUUID)
{
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    auto brksize = val["brokers"].size();

    result = sendCommand(http::verb::post, "/", "core_type=zmq&num_feds=2");
    val = loadJson(result);
    EXPECT_TRUE(val["broker_uuid"].isString());
    auto uuid = val["broker_uuid"].asString();
    result = sendGet(std::string("/?uuid=") + uuid);
    val = loadJson(result);
    EXPECT_TRUE(val["status"].asBool());

    sendCommand(http::verb::delete_, "/", std::string("broker_uuid=") + uuid);

    result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), brksize);
}

TEST_F(httpTest, coreJson)
{
    auto result = sendGet("brk2/cr1");

    Json::Value v1;
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
    Json::Value v1;
    v1["broker"] = "brk2";
    sendCommand(http::verb::post, "delete", generateJsonString(v1));
    auto result = sendGet("brokers");
    EXPECT_FALSE(result.empty());
    auto val = loadJson(result);
    EXPECT_TRUE(val["brokers"].isArray());
    EXPECT_EQ(val["brokers"].size(), 0U);
}

TEST_F(httpTest, timeBlock)
{
    sendCommand(http::verb::post, "brk_timer", "type=ZMQ");
    auto cr = addCore(helics::core_type::ZMQ, "--name=c_timer -f1 --broker=brk_timer");
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

    vFed.finalize();
    Json::Value v1;
    v1["broker"] = "brk_timer";
    sendCommand(http::verb::post, "delete", generateJsonString(v1));
}
