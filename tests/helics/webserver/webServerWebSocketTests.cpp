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
#include "gtest/gtest.h"

#include <algorithm>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
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
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

class webTest : public ::testing::Test {
protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    // Can be omitted if not needed.
    static void SetUpTestSuite() {
        webs = std::make_shared<helics::apps::WebServer>();
        webs->enableWebSocketServer(true);
        webs->startServer(nullptr);
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    // Can be omitted if not needed.
    static void TearDownTestSuite() {
        webs->stopServer();
    }

    // You can define per-test set-up logic as usual.
    virtual void SetUp() {}

    // You can define per-test tear-down logic as usual.
    virtual void TearDown() {}

    // Some expensive resource shared by all tests.
    static std::shared_ptr<helics::apps::WebServer> webs;
};

std::shared_ptr<helics::apps::WebServer> webTest::webs;

TEST_F(webTest, test1)
{

}
