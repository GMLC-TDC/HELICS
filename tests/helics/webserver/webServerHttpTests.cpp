/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/apps/helicsWebServer.hpp"

#include "helics/common/JsonProcessingFunctions.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/coreTypeOperations.hpp"
#include "helics/network/loadCores.hpp"
#include "gtest/gtest.h"

#include <algorithm>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
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

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class httpTest : public ::testing::Test {
protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite() {
        helics::loadCores();
        webs = std::make_shared<helics::apps::WebServer>();
        webs->enableHttpServer(true);
        webs->startServer(nullptr);
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite() {
        webs->stopServer();
    }

    // You can define per-test set-up logic as usual.
    virtual void SetUp() {

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        stream=std::make_unique<beast::tcp_stream>(ioc);

        // Look up the domain name
        auto const results = resolver.resolve("localhost", "80");

        // Make the connection on the IP address we get from a lookup
        stream->connect(results);
    }

    std::string sendGet(const std::string &target)
    {
        // Set up an HTTP GET request message
        http::request<http::string_body> req{ http::verb::get, target, 11 };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(*stream, req);

        // Declare a container to hold the response
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(*stream, buffer, res);
        return res.body();

    }
    static std::shared_ptr<helics::Broker> addBroker(helics::core_type ctype, const std::string &init)
    {
        auto brk = helics::BrokerFactory::create(ctype, init);
        if (brk)
        {
            brks.push_back(brk);
        }
        return brk;
    }

    static void clearBrokers()
    {
        for (auto &brk : brks)
        {
            if (brk)
            {
                brk->disconnect();
           }
        }
        brks.clear();
        helics::BrokerFactory::cleanUpBrokers();

    }
    // You can define per-test tear-down logic as usual.
    virtual void TearDown() {
        beast::error_code ec;
        stream->socket().shutdown(tcp::socket::shutdown_both, ec);
    }

private:

    // Some expensive resource shared by all tests.
    static std::shared_ptr<helics::apps::WebServer> webs;
    net::io_context ioc;

    // These objects perform our I/O
   
    std::unique_ptr<beast::tcp_stream> stream;
    beast::flat_buffer buffer;
    const std::string host{ "localhost" };
    static std::vector<std::shared_ptr<helics::Broker>> brks;
    
};

std::shared_ptr<helics::apps::WebServer> httpTest::webs;
std::vector<std::shared_ptr<helics::Broker>> httpTest::brks;

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
    EXPECT_STREQ(val["brokers"][0]["name"].asCString(),"brk1");
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
    EXPECT_STREQ(val["brokers"][0]["name"].asCString(), "brk1");

}
