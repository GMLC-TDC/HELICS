/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "RestApiConnection.hpp"

#include <algorithm>
#include <boost/asio/connect.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>

// NOLINTNEXTLINE
namespace beast = boost::beast;  // from <boost/beast.hpp>
// NOLINTNEXTLINE
namespace http = beast::http;  // from <boost/beast/http.hpp>
// NOLINTNEXTLINE
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

namespace helics::apps {

RestApiConnection::RestApiConnection(std::string_view host): hostName{host} {}

RestApiConnection::~RestApiConnection() {}

bool RestApiConnection::connect(std::string_view server, std::string_view port)
{
    disconnect();
    // These objects perform our I/O
    tcp::resolver resolverObj(ioc);
    stream = std::make_unique<boost::beast::tcp_stream>(ioc);  // NOLINT

    // Look up the domain name
    auto const results = resolverObj.resolve(server, port);

    // Make the connection on the IP address we get from a lookup
    stream->connect(results);
    connected = true;
    return connected;
}

void RestApiConnection::disconnect()
{
    if (connected) {
        beast::error_code ec;
        stream->socket().shutdown(tcp::socket::shutdown_both, ec);
        stream.reset();
        connected = false;
    }
}

std::string RestApiConnection::sendGet(const std::string& target)
{
    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, hostName);
    req.set(http::field::user_agent, "HELICS_HTTP_RESTCONNECTION");

    // Send the HTTP request to the remote host
    http::write(*stream, req);

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(*stream, buffer, res);
    return res.body();
}

std::string RestApiConnection::sendCommand(http::verb command,
                                           const std::string& target,
                                           const std::string& body)
{
    // Set up an HTTP command message
    http::request<http::string_body> req{command, target, 11};
    req.set(http::field::host, hostName);
    req.set(http::field::user_agent, "HELICS_HTTP_RESTCONNECTION");
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
        return (res.content_length()) ? std::to_string(*res.content_length()) : std::string("0");
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

}  // namespace helics::apps
