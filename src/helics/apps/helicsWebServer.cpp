/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, asynchronous
//
//------------------------------------------------------------------------------

#include "helicsWebServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../utilities/timeStringOps.hpp"
#include "helics/external/CLI11/CLI11.hpp"
#include "indexPage.hpp"

#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/config.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>  // streaming operators etc.
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;  // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;  // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

namespace helics {
namespace apps {
    class IocWrapper {
      public:
        net::io_context ioc{1};
    };
}  // namespace apps
}  // namespace helics

static std::string generateIndexPage()
{
    std::string index = helics::webserver::indexPage1;
    index.append(helics::webserver::style);
    index.append(helics::webserver::indexPage2);
    index.append(helics::webserver::svg1);
    index.append(helics::webserver::svg2);
    index.append(helics::webserver::indexPage3);
    return index;
}
// decode a URI to clean up a string, convert character codes in a URI to the original character
static std::string uriDecode(beast::string_view str)
{
    std::string ret;
    size_t len = str.length();

    for (size_t ii = 0; ii < len; ii++) {
        if (str[ii] != '%') {
            if (str[ii] == '+') {
                ret.push_back(' ');
            } else {
                ret.push_back(str[ii]);
            }
        } else {
            const std::array<char, 3> exp{{str[ii + 1], str[ii + 2], '\0'}};
            char* loc{nullptr};
            unsigned int spchar = strtoul(exp.data(), &loc, 16);
            if (loc - exp.data() >= 2) {
                ret.push_back(static_cast<char>(spchar));
                ii = ii + 2;
            } else {
                ret.push_back(str[ii]);
            }
        }
    }
    return ret;
}

// function to extract the request parameters and clean up the target
static std::pair<beast::string_view, boost::container::flat_map<std::string, std::string>>
    processRequestParameters(beast::string_view target, beast::string_view body)
{
    std::pair<beast::string_view, boost::container::flat_map<std::string, std::string>> results;
    auto param_mark = target.find('?');
    if (param_mark != beast::string_view::npos) {
        results.first = target.substr(0, param_mark);
        target = target.substr(param_mark + 1);
    } else {
        results.first = target;
        target.clear();
    }
    if (results.first.starts_with('/')) {
        results.first.remove_prefix(1);
    }

    std::vector<beast::string_view> parameters;
    if (!target.empty()) {
        auto splitloc = target.find_first_of('&');
        while (splitloc != beast::string_view::npos) {
            parameters.push_back(target.substr(0, splitloc));
            target = target.substr(splitloc + 1);
            splitloc = target.find_first_of('&');
        }
        if (!target.empty()) {
            parameters.push_back(target);
        }
    }
    if (!body.empty()) {
        if (body.front() == '{') {
            Json::Value val = loadJsonStr(body.to_string());
            auto mnames = val.getMemberNames();
            for (auto& vb : mnames) {
                if (val[vb].isString()) {
                    results.second[vb] = val[vb].asString();
                } else {
                    results.second[vb] = generateJsonString(val[vb]);
                }
            }
        } else {
            auto splitloc = body.find_first_of('&');
            while (splitloc != beast::string_view::npos) {
                parameters.push_back(body.substr(0, splitloc));
                body = body.substr(splitloc + 1);
                splitloc = body.find_first_of('&');
            }
            if (!body.empty()) {
                parameters.push_back(body);
            }
        }
    }

    for (auto& param : parameters) {
        auto eq_loc = param.find_first_of('=');
        results.second[param.substr(0, eq_loc).to_string()] = uriDecode(param.substr(eq_loc + 1));
    }
    return results;
}

void partitionTarget(beast::string_view target,
                     std::string& brokerName,
                     std::string& query,
                     std::string& targetObj)
{
    if (target.back() == '/') {
        target.remove_suffix(1);
    }
    if (!target.empty() && target.front() == '/') {
        target.remove_prefix(1);
    }
    auto slashLoc = target.find('/');
    if (slashLoc == beast::string_view::npos) {
        brokerName = target.to_string();
        return;
    }
    brokerName = target.substr(0, slashLoc).to_string();
    auto tstr = target.substr(slashLoc + 1);
    slashLoc = tstr.find('/');
    if (slashLoc == beast::string_view::npos) {
        targetObj = tstr.to_string();
        return;
    }
    targetObj = tstr.substr(0, slashLoc).to_string();
    query = tstr.substr(slashLoc + 1).to_string();
}

std::string getBrokerList()
{
    auto brks = helics::BrokerFactory::getAllBrokers();
    Json::Value base;
    base["brokers"] = Json::arrayValue;
    for (auto& brk : brks) {
        Json::Value brokerBlock;

        brokerBlock["name"] = brk->getIdentifier();
        brokerBlock["address"] = brk->getAddress();
        brokerBlock["isConnected"] = brk->isConnected();
        brokerBlock["isOpen"] = brk->isOpenToNewFederates();
        brokerBlock["isRoot"] = brk->isRoot();
        base["brokers"].append(brokerBlock);
    }
    return generateJsonString(base);
}

enum class return_val : std::int32_t {
    ok = 0,
    bad_request = static_cast<std::int32_t>(http::status::bad_request),
    not_found = static_cast<std::int32_t>(http::status::not_found),
    not_implemented = static_cast<std::int32_t>(http::status::not_implemented)
};

enum class cmd { query, create, remove, barrier, clear_barrier, unknown };

std::pair<return_val, std::string>
    generateResults(cmd command,
                    std::string brokerName,
                    beast::string_view target,
                    beast::string_view query,
                    const boost::container::flat_map<std::string, std::string>& fields)
{
    static const std::string emptyString;
    if (command == cmd::unknown) {
        if (fields.find("command") != fields.end()) {
            auto cmdstr = fields.at("command");
            if (cmdstr == "query" || cmdstr == "search" || cmdstr == "get") {
                command = cmd::query;
            }
            if (cmdstr == "status") {
                command = cmd::query;
                query = "status";
            }
            if (cmdstr == "create") {
                command = cmd::create;
            }

            if (cmdstr == "barrier") {
                command = cmd::barrier;
            }
            if (cmdstr == "clearbarrier") {
                command = cmd::clear_barrier;
            }
            if (cmdstr == "delete" || cmdstr == "remove") {
                command = cmd::remove;
            }
        }
    }
    if (command == cmd::unknown) {
        return {return_val::not_implemented, "command not recognized"};
    }
    if (command == cmd::create && brokerName == "create") {
        brokerName.clear();
    }
    if (command == cmd::create && brokerName == "barrier") {
        brokerName.clear();
        command = cmd::barrier;
    }
    if (command == cmd::create && target == "barrier") {
        command = cmd::barrier;
    }
    if (command == cmd::remove && (brokerName == "delete" || brokerName == "remove")) {
        brokerName.clear();
    }
    if (command == cmd::remove && brokerName == "barrier") {
        brokerName.clear();
        command = cmd::clear_barrier;
    }
    if (command == cmd::remove && target == "barrier") {
        command = cmd::clear_barrier;
    }
    if (command == cmd::query && (brokerName == "query" || brokerName == "search")) {
        brokerName.clear();
    }
    if (query.empty()) {
        if (fields.find("query") != fields.end()) {
            query = fields.at("query");
        }
    }
    if (target.empty()) {
        if (fields.find("target") != fields.end()) {
            target = fields.at("target");
        }
    }
    if (brokerName.empty()) {
        if (fields.find("broker") != fields.end()) {
            brokerName = fields.at("broker");
        } else if (fields.find("broker_uuid") != fields.end()) {
            brokerName = fields.at("broker_uuid");
        } else if (fields.find("uuid") != fields.end()) {
            brokerName = fields.at("uuid");
        }
    }
    if (brokerName.empty() && target == "brokers") {
        brokerName = "brokers";
    }
    if (brokerName == "brokers" || (brokerName.empty() && query == "brokers")) {
        return {return_val::ok, getBrokerList()};
    }

    std::shared_ptr<helics::Broker> brkr;
    if (brokerName.empty()) {
        if (!target.empty()) {
            brkr = helics::BrokerFactory::findBroker(target.to_string());
        }
    } else {
        brkr = helics::BrokerFactory::findBroker(brokerName);
    }

    if (!brkr) {
        if (fields.find("broker") != fields.end()) {
            if (query.empty()) {
                query = target;
                target = brokerName;
            } else if (target.empty()) {
                target = brokerName;
            }
            brkr = helics::BrokerFactory::findBroker(fields.at("broker"));
        }
    }
    switch (command) {
        case cmd::create: {
            if (brkr) {
                return {return_val::bad_request, brokerName + " already exists"};
            }
            std::string start_args;
            std::string type;
            if (fields.find("args") != fields.end()) {
                start_args = fields.at("args");
            }
            if (fields.find("type") != fields.end()) {
                type = fields.at("type");
            } else if (fields.find("core_type") != fields.end()) {
                type = fields.at("core_type");
            }
            helics::core_type ctype{helics::core_type::DEFAULT};
            if (!type.empty()) {
                ctype = helics::core::coreTypeFromString(type);
                if (!helics::core::isCoreTypeAvailable(ctype)) {
                    // return send(bad_request(type + " is not available"));
                    return {return_val::bad_request, type + " is not available"};
                }
            }
            if (fields.find("num_feds") != fields.end()) {
                start_args += " -f " + fields.at("num_feds");
            }
            if (fields.find("num_brokers") != fields.end()) {
                start_args += " --minbrokers=" + fields.at("num_brokers");
            }
            bool useUuid{false};
            if (brokerName.empty()) {
                boost::uuids::random_generator generator;

                boost::uuids::uuid uuid1 = generator();
                std::ostringstream ss1;
                ss1 << uuid1;
                brokerName = ss1.str();
                useUuid = true;
            }
            brkr = helics::BrokerFactory::create(ctype, brokerName, start_args);
            if (!brkr) {
                return {return_val::bad_request, "unable to create broker"};
                // return send(bad_request("unable to create broker"));
            }
            if (useUuid) {
                return {return_val::ok, std::string(R"({"broker_uuid":")") + brokerName + "\"}"};
            }
            return {return_val::ok, emptyString};
        }
        case cmd::remove:
            if (!brkr) {
                return {return_val::not_found, brokerName + " not found"};
            }
            brkr->disconnect();
            return {return_val::ok, emptyString};
        case cmd::barrier: {
            if (!brkr) {
                brkr = helics::BrokerFactory::getConnectedBroker();
                if (!brkr) {
                    return {return_val::bad_request, "unable to locate broker"};
                }
            }
            if (fields.find("time") == fields.end()) {
                brkr->clearTimeBarrier();
                return {return_val::ok, emptyString};
            }
            auto bTime = gmlc::utilities::loadTimeFromString<helics::Time>(fields.at("time"));
            if (bTime >= helics::timeZero) {
                brkr->setTimeBarrier(bTime);
            } else {
                brkr->clearTimeBarrier();
            }
            return {return_val::ok, emptyString};
        }
        case cmd::clear_barrier:
            if (!brkr) {
                brkr = helics::BrokerFactory::getConnectedBroker();
                if (!brkr) {
                    return {return_val::bad_request, "unable to locate broker"};
                }
            }
            brkr->clearTimeBarrier();
            return {return_val::ok, emptyString};
        default:
            break;
    }  // end switch

    bool autoquery{false};
    if (!brkr) {
        brkr = helics::BrokerFactory::getConnectedBroker();
        if (!brkr) {
            return {return_val::not_found, brokerName + " not found and no valid brokers"};
        }
        if (target.empty()) {
            query = brokerName;
        } else {
            query = target;
            target = brokerName;
        }

    } else if (query.empty() && !target.empty()) {
        query = target;
        autoquery = true;
        target = "root";
    }
    if (target.empty()) {
        target = "root";
    }
    if (query.empty()) {
        query = "current_state";
    }
    auto res = brkr->query(target.to_string(), query.to_string());
    if (res != "#invalid") {
        return {return_val::ok, res};
    }

    if (autoquery) {
        res = brkr->query(query.to_string(), "current_state");
        if (res == "#invalid") {
            return {return_val::not_found, "target not found"};
        }
        return {return_val::ok, res};
    }
    return {return_val::not_found, "query not recognized"};
}

// LCOV_EXCL_START
// Report a failure
static void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// LCOV_EXCL_STOP

// Echoes back all received WebSocket messages
class WebSocketsession: public std::enable_shared_from_this<WebSocketsession> {
    websocket::stream<beast::tcp_stream> ws;
    beast::flat_buffer buffer;

  public:
    // Take ownership of the socket
    explicit WebSocketsession(tcp::socket&& socket): ws(std::move(socket)) {}

    // Get on the correct executor
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws.get_executor(),
                      beast::bind_front_handler(&WebSocketsession::on_run, shared_from_this()));
    }

    // Start the asynchronous operation
    void on_run()
    {
        // Set suggested timeout settings for the websocket
        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
            res.set(http::field::server, std::string("HELICS_WEB_SERVER" HELICS_VERSION_STRING));
        }));
        // Accept the websocket handshake
        ws.async_accept(
            beast::bind_front_handler(&WebSocketsession::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec)
    {
        if (ec) {
            return fail(ec, "helics websocket accept");
        }

        // Read a message
        do_read();
    }

    void do_read()
    {
        // Read a message into our buffer
        ws.async_read(buffer,
                      beast::bind_front_handler(&WebSocketsession::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (ec == websocket::error::closed) {
            return;
        }

        if (ec) {
            return fail(ec, "helics web server read");
        }

        beast::string_view result{boost::asio::buffer_cast<const char*>(buffer.data()),
                                  buffer.size()};
        // Echo the message
        auto reqpr = processRequestParameters("", result);

        cmd command{cmd::unknown};

        auto res = generateResults(command, {}, "", "", reqpr.second);
        // Clear the buffer
        buffer.consume(buffer.size());

        ws.text(true);
        if (res.first == return_val::ok && !res.second.empty() && res.second.front() == '{') {
            boost::beast::ostream(buffer) << res.second;  // NOLINT
            ws.async_write(buffer.data(),
                           beast::bind_front_handler(&WebSocketsession::on_write,
                                                     shared_from_this()));
            return;
        }
        Json::Value response;
        switch (res.first) {
            case return_val::bad_request:
                response["status"] = static_cast<int>(http::status::bad_request);
                response["error"] = res.second;
                break;
            case return_val::not_found:
                response["status"] = static_cast<int>(http::status::not_found);
                response["error"] = res.second;
                break;
            default:
                response["status"] = static_cast<int>(res.first);
                response["error"] = res.second;
                break;
            case return_val::ok:
                response["status"] = 0;
                response["value"] = res.second;
                break;
        }

        boost::beast::ostream(buffer) << generateJsonString(response);  // NOLINT
        ws.async_write(buffer.data(),
                       beast::bind_front_handler(&WebSocketsession::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            return fail(ec, "helics socket write");
        }

        // Clear the buffer
        buffer.consume(buffer.size());

        // Do another read
        do_read();
    }
};

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
{
    static const std::string index_page = generateIndexPage();
    // Returns a bad request response
    auto const bad_request = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER" HELICS_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a bad request response
    auto const not_found = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER " HELICS_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // generate a conversion response
    auto const response_ok = [&req](const std::string& resp, beast::string_view content_type) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER " HELICS_VERSION_STRING);
        res.set(http::field::content_type, content_type);
        res.keep_alive(req.keep_alive());
        if (req.method() != http::verb::head) {
            res.body() = resp;
            res.prepare_payload();
        } else {
            res.set(http::field::content_length, std::to_string(resp.size()));
        }
        return res;
    };

    cmd command{cmd::query};

    auto method = req.method();
    switch (method) {
        case http::verb::head:
        case http::verb::search:
        case http::verb::get:
            break;
        case http::verb::post:
        case http::verb::put:
            command = cmd::create;
            break;
        case http::verb::delete_:
            command = cmd::remove;
            break;
        default:
            return send(bad_request("Unknown HTTP-method"));
    }

    beast::string_view target(req.target());
    auto psize = req.payload_size();
    if (target == "/index.html" || (target == "/" && (!psize || *psize < 4))) {
        return send(response_ok(index_page, "text/html"));
    }

    auto reqpr = processRequestParameters(target, req.body());
    std::string brokerName;
    std::string query;
    std::string targetObj;

    partitionTarget(reqpr.first, brokerName, query, targetObj);

    if (method == http::verb::post) {
        if (brokerName == "delete" || brokerName == "remove") {
            command = cmd::remove;
            brokerName.clear();
        } else if (brokerName == "create") {
            command = cmd::create;
            brokerName.clear();
        } else if (brokerName == "query" || brokerName == "search") {
            command = cmd::query;
            brokerName.clear();
        }
    }
    auto res = generateResults(command, brokerName, targetObj, query, reqpr.second);
    switch (res.first) {
        case return_val::bad_request:
            return send(bad_request(res.second));
        case return_val::not_found:
            return send(not_found(res.second));
        case return_val::ok:
        default:
            if (res.second.empty()) {
                return send(response_ok(index_page, "text/html"));
            }
            if (res.second.front() == '{') {
                return send(response_ok(res.second, "application/json"));
            }
            return send(response_ok(res.second, "text/plain"));
    }
}

//------------------------------------------------------------------------------

// Handles an HTTP server connection
class HttpSession: public std::enable_shared_from_this<HttpSession> {
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda {
        HttpSession& self_ref;

        explicit send_lambda(HttpSession& self): self_ref(self) {}

        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_ref.res = sp;

            // Write the response
            http::async_write(self_ref.stream,
                              *sp,
                              beast::bind_front_handler(&HttpSession::on_write,
                                                        self_ref.shared_from_this(),
                                                        sp->need_eof()));
        }
    };

    beast::tcp_stream stream;
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    std::shared_ptr<void> res;
    send_lambda lambda;

  public:
    // Take ownership of the stream
    explicit HttpSession(tcp::socket&& socket): stream(std::move(socket)), lambda(*this) {}

    // Start the asynchronous operation
    void run() { do_read(); }

    void do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req = {};

        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream,
                         buffer,
                         req,
                         beast::bind_front_handler(&HttpSession::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == http::error::end_of_stream) {
            return do_close();
        }

        if (ec) {
            if (beast::error::timeout != ec) {
                fail(ec, "helics web server read");
            }
            return;
        }

        // Send the response
        handle_request(std::move(req), lambda);
    }

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            return fail(ec, "helics web server write");
        }

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res = nullptr;

        // Read another request
        do_read();
    }

    void do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class Listener: public std::enable_shared_from_this<Listener> {
    net::io_context& ioc;
    tcp::acceptor acceptor;
    bool websocket{false};

  public:
    Listener(net::io_context& context, const tcp::endpoint& endpoint, bool webs = false):
        ioc(context), acceptor(net::make_strand(ioc)), websocket{webs}
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor.open(endpoint.protocol(), ec);
        if (ec) {
            fail(ec, "helics acceptor open");
            return;
        }

        // Allow address reuse
        acceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            fail(ec, "helics acceptor set_option");
            return;
        }

        // Bind to the server address
        acceptor.bind(endpoint, ec);
        if (ec) {
            fail(ec, "helics acceptor bind");
            return;
        }

        // Start listening for connections
        acceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            fail(ec, "helics acceptor listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run() { do_accept(); }

  private:
    void do_accept()
    {
        // The new connection gets its own strand
        acceptor.async_accept(net::make_strand(ioc),
                              beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if (ec) {
            return fail(ec, "helics accept connections");
        } else {
            if (websocket) {
                // Create the session and run it
                std::make_shared<WebSocketsession>(std::move(socket))->run();
            } else {
                // Create the session and run it
                std::make_shared<HttpSession>(std::move(socket))->run();
            }
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

static const Json::Value null;

namespace helics {
namespace apps {

    void WebServer::processArgs(const std::string& args)

    {
        CLI::App parser("http web server parser");
        parser.allow_extras();
        parser.add_option("--http_port", httpPort_, "specify the http port to use")
            ->envname("HELICS_HTTP_PORT");
        parser.add_option("--http_interface",
                          httpAddress_,
                          "specify the interface to use for connecting an http server");

        parser.add_option("--websocket_port", websocketPort_, "specify the websocket port to use")
            ->envname("HELICS_WEBSOCKET_PORT");
        parser.add_option("--websocket_interface",
                          websocketAddress_,
                          "specify the interface to use for connecting a web server");

        try {
            parser.parse(args);
        }
        catch (const CLI::Error& ce) {
            logMessage(std::string("error processing command line arguments for web server :") +
                       ce.what());
        }
    }

    void WebServer::startServer(const Json::Value* val)
    {
        logMessage("starting broker web server");
        config = (val != nullptr) ? val : &null;
        bool exp{false};
        if (running.compare_exchange_strong(exp, true)) {
            // The io_context is required for all I/O
            context = std::make_shared<IocWrapper>();

            std::lock_guard<std::mutex> tlock(threadGuard);

            mainLoopThread = std::thread([this]() { mainLoop(); });
            mainLoopThread.detach();
            std::this_thread::yield();
            while (!executing) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }

    /** stop the server*/
    void WebServer::stopServer()
    {
        bool exp{true};
        if (running.compare_exchange_strong(exp, false)) {
            logMessage("stopping broker web server");
            std::lock_guard<std::mutex> tlock(threadGuard);
            context->ioc.stop();
        }
    }

    void WebServer::mainLoop()
    {
        if (http_enabled_) {
            if (config->isMember("http")) {
                auto V = (*config)["http"];
                replaceIfMember(V, "interface", httpAddress_);
                replaceIfMember(V, "port", httpPort_);
            }
            auto const address = net::ip::make_address(httpAddress_);
            // Create and launch a listening port
            std::make_shared<Listener>(context->ioc,
                                       tcp::endpoint{address,
                                                     static_cast<std::uint16_t>(httpPort_)})
                ->run();
        }

        if (websocket_enabled_) {
            if (config->isMember("websocket")) {
                auto V = (*config)["websocket"];
                replaceIfMember(V, "interface", websocketAddress_);
                replaceIfMember(V, "port", websocketPort_);
            }
            auto const address = net::ip::make_address(websocketAddress_);
            // Create and launch a listening port
            std::make_shared<Listener>(context->ioc,
                                       tcp::endpoint{address,
                                                     static_cast<std::uint16_t>(websocketPort_)},
                                       true)
                ->run();
        }
        executing.store(true);
        // Run the I/O service
        if (running.load()) {
            context->ioc.run();
        }
        executing.store(false);
    }

}  // namespace apps
}  // namespace helics
