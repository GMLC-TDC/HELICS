/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../common/JsonProcessingFunctions.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/container/flat_map.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace helics::apps {

class RestApiConnection {
  public:
    explicit RestApiConnection(std::string_view host = "localhost");

    ~RestApiConnection();

    bool connect(std::string_view server, std::string_view port);

    void disconnect();

    std::string sendGet(const std::string& target);

    std::string sendCommand(boost::beast::http::verb command,
                            const std::string& target,
                            const std::string& body);

  private:
    boost::asio::io_context ioc;

    std::unique_ptr<boost::beast::tcp_stream> stream;
    boost::beast::flat_buffer buffer;

    std::string hostName{"localHost"};
    nlohmann::json config;
    bool connected{false};
};
}  // namespace helics::apps
