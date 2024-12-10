/*
Copyright (c) 2017-2024,
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
#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"
#include "helics/external/CLI11/CLI11.hpp"
#include "indexPage.hpp"

#include <algorithm>

// libc++ assumes aligned_alloc is available in the underlying c library,
// but it isn't in environments that use MSVCRT/UCRT; we're undefine it so that
// asio won't rely on the function
#if defined(_LIBCPP_HAS_ALIGNED_ALLOC) && (defined(_LIBCPP_MSVCRT) || defined(__MINGW32__))
#    undef _LIBCPP_HAS_ALIGNED_ALLOC
#endif
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
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;  // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;  // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

namespace helics::apps {
class IocWrapper {
  public:
    net::io_context ioc{1};
};
}  // namespace helics::apps

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
static std::string uriDecode(const std::string_view str)
{
    std::string ret;

    for (size_t ii = 0; ii < str.length(); ii++) {
        if (str[ii] != '%') {
            if (str[ii] == '+') {
                ret.push_back(' ');
            } else {
                ret.push_back(str[ii]);
            }
        } else {
            const std::array<char, 3> exp{{str[ii + 1], str[ii + 2], '\0'}};
            char* loc{nullptr};
            const unsigned int spchar = strtoul(exp.data(), &loc, 16);
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
static std::pair<std::string_view, boost::container::flat_map<std::string, std::string>>
    processRequestParameters(std::string_view target, std::string_view body)
{
    std::pair<std::string_view, boost::container::flat_map<std::string, std::string>> results;
    auto param_mark = target.find('?');
    if (param_mark != std::string_view::npos) {
        results.first = target.substr(0, param_mark);
        target = target.substr(param_mark + 1);
    } else {
        results.first = target;
        target = std::string_view{};
    }
    if (!results.first.empty() && results.first.front() == '/') {
        results.first.remove_prefix(1);
    }

    std::vector<std::string_view> parameters;
    if (!target.empty()) {
        auto splitloc = target.find_first_of('&');
        while (splitloc != std::string_view::npos) {
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
            nlohmann::json val = helics::fileops::loadJsonStr(body);
            for (const auto& value : val.items()) {
                if (value.value().is_string()) {
                    results.second[value.key()] = value.value().get<std::string>();
                } else {
                    results.second[value.key()] =
                        helics::fileops::generateJsonString(value.value());
                }
            }
        } else {
            auto splitloc = body.find_first_of('&');
            while (splitloc != std::string_view::npos) {
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
        results.second[std::string{param.substr(0, eq_loc)}] = uriDecode(param.substr(eq_loc + 1));
    }
    return results;
}

static void partitionTarget(std::string_view target,
                            std::string& brokerName,
                            std::string& query,
                            std::string& targetObj)
{
    if (!target.empty() && target.back() == '/') {
        target.remove_suffix(1);
    }
    if (!target.empty() && target.front() == '/') {
        target.remove_prefix(1);
    }
    auto slashLoc = target.find('/');
    if (slashLoc == std::string_view::npos) {
        brokerName = target;
        return;
    }
    brokerName = target.substr(0, slashLoc);
    auto tstr = target.substr(slashLoc + 1);
    slashLoc = tstr.find('/');
    if (slashLoc == std::string_view::npos) {
        targetObj = tstr;
        return;
    }
    targetObj = tstr.substr(0, slashLoc);
    query = tstr.substr(slashLoc + 1);
}

static std::string getBrokerList()
{
    auto brks = helics::BrokerFactory::getAllBrokers();
    nlohmann::json base;
    base["brokers"] = nlohmann::json::array();
    for (auto& brk : brks) {
        nlohmann::json brokerBlock;

        brokerBlock["name"] = brk->getIdentifier();
        brokerBlock["address"] = brk->getAddress();
        brokerBlock["isConnected"] = brk->isConnected();
        brokerBlock["isOpen"] = brk->isOpenToNewFederates();
        brokerBlock["isRoot"] = brk->isRoot();
        base["brokers"].push_back(brokerBlock);
    }
    return helics::fileops::generateJsonString(base);
}
/** create a broker from the fields*/
static std::tuple<std::shared_ptr<helics::Broker>, std::string, bool, std::string>
    createBroker(std::string& brokerName,
                 const boost::container::flat_map<std::string, std::string>& fields)
{
    std::string start_args;
    std::string type;
    if (fields.find("args") != fields.end()) {
        start_args = fields.at("args");
    }
    if (fields.find("type") != fields.end()) {
        type = fields.at("type");
    } else if (fields.find("CoreType") != fields.end()) {
        type = fields.at("CoreType");
    } else if (fields.find("core_type") != fields.end()) {
        type = fields.at("core_type");
    }
    helics::CoreType ctype{helics::CoreType::DEFAULT};
    if (!type.empty()) {
        ctype = helics::core::coreTypeFromString(type);
        if (!helics::core::isCoreTypeAvailable(ctype)) {
            // return send(bad_request(type + " is not available"));
            return {nullptr, type + " is not available", false, ""};
        }
    }
    if (fields.find("num_feds") != fields.end()) {
        start_args += " -f " + fields.at("num_feds");
    }
    if (fields.find("num_brokers") != fields.end()) {
        start_args += " --minbrokers=" + fields.at("num_brokers");
    }
    if (fields.find("port") != fields.end()) {
        start_args += " --port=" + fields.at("port");
    }
    if (fields.find("host") != fields.end()) {
        start_args += " --interface=" + fields.at("host");
    }
    if (fields.find("log_level") != fields.end()) {
        start_args += " --loglevel=" + fields.at("log_level");
    }
    bool useUuid{false};
    if (brokerName.empty()) {
        boost::uuids::random_generator generator;

        const boost::uuids::uuid uuid1 = generator();
        std::ostringstream ss1;
        ss1 << uuid1;
        brokerName = ss1.str();
        useUuid = true;
    }
    try {
        return {helics::BrokerFactory::create(ctype, brokerName, start_args), "", useUuid, type};
    }
    catch (const std::exception& exc) {
        return {nullptr, exc.what(), false, ""};
    }
}

enum class RequestReturnVal : std::int32_t {
    OK = 0,
    BAD_REQUEST = static_cast<std::int32_t>(http::status::bad_request),
    NOT_FOUND = static_cast<std::int32_t>(http::status::not_found),
    NOT_IMPLEMENTED = static_cast<std::int32_t>(http::status::not_implemented)
};

// set of possible commands that the web server can implement
enum class RestCommand { QUERY, CREATE, REMOVE, BARRIER, CLEAR_BARRIER, COMMAND, UNKNOWN };

static const std::unordered_map<std::string_view, RestCommand> commandMap{
    {"query", RestCommand::QUERY},
    {"search", RestCommand::QUERY},
    {"get", RestCommand::QUERY},
    {"status", RestCommand::QUERY},
    {"create", RestCommand::CREATE},
    {"barrier", RestCommand::BARRIER},
    {"command", RestCommand::COMMAND},
    {"clearbarrier", RestCommand::CLEAR_BARRIER},
    {"clear_barrier", RestCommand::CLEAR_BARRIER},
    {"delete", RestCommand::REMOVE},
    {"clear", RestCommand::REMOVE},
};

static std::pair<RequestReturnVal, std::string>
    generateResults(RestCommand command,
                    std::string brokerName,
                    std::string_view target,
                    std::string_view query,
                    const boost::container::flat_map<std::string, std::string>& fields)
{
    static const std::string emptyString;
    if (command == RestCommand::UNKNOWN) {
        if (fields.find("command") != fields.end()) {
            const auto& cmdstr = fields.at("command");
            if (commandMap.find(cmdstr) != commandMap.end()) {
                command = commandMap.at(cmdstr);
                if (cmdstr == "status") {
                    query = "status";
                }
            }
        }
    }
    switch (command) {
        case RestCommand::UNKNOWN:
            return {RequestReturnVal::NOT_IMPLEMENTED, "command not recognized"};
        case RestCommand::CREATE:
            if (brokerName == "create") {
                brokerName.clear();
                break;
            }
            if (brokerName == "barrier") {
                brokerName.clear();
                command = RestCommand::BARRIER;
                break;
            }
            if (target == "barrier") {
                command = RestCommand::BARRIER;
            }
            break;
        case RestCommand::REMOVE:
            if (brokerName == "delete" || brokerName == "remove") {
                brokerName.clear();
                break;
            }
            if (brokerName == "barrier") {
                brokerName.clear();
                command = RestCommand::CLEAR_BARRIER;
                break;
            }
            if (target == "barrier") {
                command = RestCommand::CLEAR_BARRIER;
            }
            break;
        case RestCommand::QUERY:
            if (brokerName == "query" || brokerName == "search") {
                brokerName.clear();
            }
            break;
        default:
            break;
    }

    if (query.empty() && fields.find("query") != fields.end()) {
        query = fields.at("query");
    }
    if (target.empty() && fields.find("target") != fields.end()) {
        target = fields.at("target");
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
    if ((brokerName.empty() && (target == "brokers" || query == "brokers")) ||
        brokerName == "brokers") {
        return {RequestReturnVal::OK, getBrokerList()};
    }

    std::shared_ptr<helics::Broker> brkr;
    if (brokerName.empty()) {
        if (!target.empty()) {
            brkr = helics::BrokerFactory::findBroker(target);
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
        case RestCommand::CREATE: {
            if (brkr) {
                return {RequestReturnVal::BAD_REQUEST, brokerName + " already exists"};
            }
            std::string errorMessage;
            bool useUuid{false};
            std::string type;
            std::tie(brkr, errorMessage, useUuid, type) = createBroker(brokerName, fields);

            if (!brkr) {
                return {RequestReturnVal::BAD_REQUEST, errorMessage};
            }
            nlohmann::json retJson;
            retJson["broker"] = brokerName;
            if (useUuid) {
                retJson["broker_uuid"] = brokerName;
            }
            retJson["type"] = type;
            return {RequestReturnVal::OK, helics::fileops::generateJsonString(retJson)};
        }
        case RestCommand::REMOVE:
            if (!brkr) {
                return {RequestReturnVal::NOT_FOUND, brokerName + " not found"};
            }
            brkr->disconnect();
            return {RequestReturnVal::OK, emptyString};
        case RestCommand::BARRIER: {
            if (!brkr) {
                brkr = helics::BrokerFactory::getConnectedBroker();
                if (!brkr) {
                    return {RequestReturnVal::NOT_FOUND, "unable to locate broker"};
                }
            }
            if (fields.find("time") == fields.end()) {
                brkr->clearTimeBarrier();
                return {RequestReturnVal::OK, emptyString};
            }
            auto bTime = gmlc::utilities::loadTimeFromString<helics::Time>(fields.at("time"));
            if (bTime >= helics::timeZero) {
                brkr->setTimeBarrier(bTime);
            } else {
                brkr->clearTimeBarrier();
            }
            return {RequestReturnVal::OK, emptyString};
        }
        case RestCommand::COMMAND:
            if (!brkr) {
                brkr = helics::BrokerFactory::getConnectedBroker();
                if (!brkr) {
                    return {RequestReturnVal::NOT_FOUND, "unable to locate broker"};
                }
            }
            if (fields.find("command_str") != fields.end()) {
                brkr->sendCommand(target, fields.at("command_str"));
            } else if (!query.empty()) {
                brkr->sendCommand(target, query);
            } else {
                return {RequestReturnVal::BAD_REQUEST, "no valid command string"};
            }
            return {RequestReturnVal::OK, emptyString};
        case RestCommand::CLEAR_BARRIER:
            if (!brkr) {
                brkr = helics::BrokerFactory::getConnectedBroker();
                if (!brkr) {
                    return {RequestReturnVal::NOT_FOUND, "unable to locate broker"};
                }
            }

            brkr->clearTimeBarrier();
            return {RequestReturnVal::OK, emptyString};
        default:
            break;
    }  // end switch

    bool autoquery{false};
    if (!brkr) {
        brkr = helics::BrokerFactory::getConnectedBroker();
        if (!brkr) {
            return {RequestReturnVal::NOT_FOUND, brokerName + " not found and no valid brokers"};
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
    auto res = brkr->query(target, query);
    if (res.find("\"error\"") == std::string::npos) {
        return {RequestReturnVal::OK, res};
    }

    if (autoquery) {
        res = brkr->query(query, "current_state");
        if (res.find("\"error\"") != std::string::npos) {
            return {RequestReturnVal::NOT_FOUND, "target not found"};
        }
        return {RequestReturnVal::OK, res};
    }
    return {RequestReturnVal::NOT_FOUND, "query not recognized"};
}

// LCOV_EXCL_START
// Report a failure
static void fail(beast::error_code eCode, char const* what)
{
    std::cerr << what << ": " << eCode.message() << "\n";
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

    void on_accept(beast::error_code eCode)
    {
        if (eCode) {
            return fail(eCode, "helics websocket accept");
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

    void on_read(beast::error_code eCode, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (eCode == websocket::error::closed) {
            return;
        }

        if (eCode) {
            return fail(eCode, "helics web server read");
        }

        const std::string_view result{boost::asio::buffer_cast<const char*>(buffer.data()),
                                      buffer.size()};
        // Echo the message
        auto reqpr = processRequestParameters("", result);

        const RestCommand command{RestCommand::UNKNOWN};

        auto res = generateResults(command, {}, "", "", reqpr.second);
        // Clear the buffer
        buffer.consume(buffer.size());

        ws.text(true);
        if (res.first == RequestReturnVal::OK && !res.second.empty() && res.second.front() == '{') {
            boost::beast::ostream(buffer) << res.second;  // NOLINT
            ws.async_write(buffer.data(),
                           beast::bind_front_handler(&WebSocketsession::on_write,
                                                     shared_from_this()));
            return;
        }
        nlohmann::json response;
        switch (res.first) {
            case RequestReturnVal::BAD_REQUEST:
                response["status"] = static_cast<int>(http::status::bad_request);
                response["error"] = res.second;
                break;
            case RequestReturnVal::NOT_FOUND:
                response["status"] = static_cast<int>(http::status::not_found);
                response["error"] = res.second;
                break;
            default:
                response["status"] = static_cast<int>(res.first);
                response["error"] = res.second;
                break;
            case RequestReturnVal::OK:
                response["status"] = 0;
                response["value"] = res.second;
                break;
        }

        boost::beast::ostream(buffer) << helics::fileops::generateJsonString(response);  // NOLINT
        ws.async_write(buffer.data(),
                       beast::bind_front_handler(&WebSocketsession::on_write, shared_from_this()));
    }

    void on_write(beast::error_code eCode, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (eCode) {
            return fail(eCode, "helics socket write");
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
    auto const bad_request = [&req](std::string_view why) {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER " HELICS_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());

        res.set(http::field::access_control_allow_origin, "*");

        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a bad request response
    auto const not_found = [&req](std::string_view why) {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER " HELICS_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());

        res.set(http::field::access_control_allow_origin, "*");

        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // generate a conversion response
    auto const response_ok = [&req](const std::string& resp, std::string_view content_type) {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "HELICS_WEB_SERVER " HELICS_VERSION_STRING);
        res.set(http::field::content_type, content_type);
        res.keep_alive(req.keep_alive());

        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, "*");
        res.set(http::field::access_control_allow_headers, "*");

        if (req.method() != http::verb::head) {
            res.body() = resp;
            res.prepare_payload();
        } else {
            res.set(http::field::content_length, std::to_string(resp.size()));
        }
        return res;
    };

    RestCommand command{RestCommand::QUERY};

    auto method = req.method();
    switch (method) {
        case http::verb::head:
        case http::verb::search:
        case http::verb::get:
            break;
        case http::verb::post:
        case http::verb::put:
            command = RestCommand::CREATE;
            break;
        case http::verb::delete_:
            command = RestCommand::REMOVE;
            break;
        case http::verb::options:
            return send(response_ok("{\"success\":true}", "application/json"));
        default:
            return send(bad_request("Unknown HTTP-method"));
    }

    const std::string_view target(req.target());
    auto psize = req.payload_size();
    if (target == "/index.html" || target == "index.html" ||
        (target == "/" && (!psize || *psize < 4))) {
        return send(response_ok(index_page, "text/html"));
    }

    if (target == "/healthcheck" || target == "healthcheck") {
        return send(response_ok("{\"success\":true}", "application/json"));
    }

    auto reqpr = processRequestParameters(target, req.body());
    std::string brokerName;
    std::string query;
    std::string targetObj;

    partitionTarget(reqpr.first, brokerName, query, targetObj);

    if (method == http::verb::post) {
        if (brokerName == "delete" || brokerName == "remove") {
            command = RestCommand::REMOVE;
            brokerName.clear();
        } else if (brokerName == "create") {
            command = RestCommand::CREATE;
            brokerName.clear();
        } else if (brokerName == "query" || brokerName == "search") {
            command = RestCommand::QUERY;
            brokerName.clear();
        } else if (brokerName == "command") {
            command = RestCommand::COMMAND;
            brokerName.clear();
        }
    }
    auto res = generateResults(command, brokerName, targetObj, query, reqpr.second);
    switch (res.first) {
        case RequestReturnVal::BAD_REQUEST:
            return send(bad_request(res.second));
        case RequestReturnVal::NOT_FOUND:
            return send(not_found(res.second));
        case RequestReturnVal::OK:
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
            auto message = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_ref.res = message;

            // Write the response
            http::async_write(self_ref.stream,
                              *message,
                              beast::bind_front_handler(&HttpSession::on_write,
                                                        self_ref.shared_from_this(),
                                                        message->need_eof()));
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

    void on_read(beast::error_code eCode, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (eCode == http::error::end_of_stream) {
            return do_close();
        }

        if (eCode) {
            if (beast::error::timeout != eCode) {
                fail(eCode, "helics web server read");
            }
            return;
        }

        // Send the response
        handle_request(std::move(req), lambda);
    }

    void on_write(bool close, beast::error_code eCode, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (eCode) {
            return fail(eCode, "helics web server write");
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
        beast::error_code eCode;
        eCode = stream.socket().shutdown(tcp::socket::shutdown_send, eCode);

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
        beast::error_code eCode;

        // Open the acceptor
        eCode = acceptor.open(endpoint.protocol(), eCode);
        if (eCode) {
            fail(eCode, "helics acceptor open");
            return;
        }

        // Allow address reuse
        (void)(acceptor.set_option(net::socket_base::reuse_address(true), eCode));
        if (eCode) {
            fail(eCode, "helics acceptor set_option");
            return;
        }

        // Bind to the server address
        (void)(acceptor.bind(endpoint, eCode));
        if (eCode) {
            fail(eCode, "helics acceptor bind");
            return;
        }

        // Start listening for connections
        (void)(acceptor.listen(net::socket_base::max_listen_connections, eCode));
        if (eCode) {
            fail(eCode, "helics acceptor listen");
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

    void on_accept(beast::error_code eCode, tcp::socket socket)
    {
        if (eCode) {
            return fail(eCode, "helics accept connections");
        }
        if (websocket) {
            // Create the session and run it
            std::make_shared<WebSocketsession>(std::move(socket))->run();
        } else {
            // Create the session and run it
            std::make_shared<HttpSession>(std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

static const nlohmann::json null;

namespace helics::apps {

void WebServer::processArgs(std::string_view args)

{
    CLI::App parser("http web server parser");
    parser.allow_extras();
    parser.add_option("--http_port", mHttpPort, "specify the http port to use")
        ->envname("HELICS_HTTP_PORT");
    parser
        .add_option("--http_interface",
                    mHttpAddress,
                    "specify the interface for the http server to listen on for connections")
        ->envname("HELICS_HTTP_ADDRESS");
    auto* httpsub = parser.add_subcommand("http")->fallthrough();
    httpsub->add_option("--port", mHttpPort, "specify the http port to use");
    httpsub->add_option("--interface",
                        mHttpAddress,
                        "specify the interface for the http server to listen on for connections");

    parser.add_option("--websocket_port", mWebsocketPort, "specify the websocket port to use")
        ->envname("HELICS_WEBSOCKET_PORT");
    parser
        .add_option("--websocket_interface",
                    mWebsocketAddress,
                    "specify the interface for the websocket server to listen on for connections")
        ->envname("HELICS_WEBSOCKET_ADDRESS");

    auto* websub = parser.add_subcommand("websocket");
    websub->add_option("--port", mWebsocketPort, "specify the websocket port to use");
    websub->add_option(
        "--interface",
        mWebsocketAddress,
        "specify the interface for the websocket server to listen on for connections");
    auto* niflag = parser
                       .add_flag("--local{0},--ipv4{4},--ipv6{6},--all{10},--external{10}",
                                 mInterfaceNetwork,
                                 "specify external interface to use, default is --local")
                       ->disable_flag_override()
                       ->envname("HELICS_WEBSERVER_INTERFACE");

    parser
        .add_option("--network_connectivity",
                    mInterfaceNetwork,
                    "specify the connectivity of the network interface")
        ->transform(CLI::CheckedTransformer(
            {{"local", "0"}, {"ipv4", "4"}, {"ipv6", "6"}, {"external", "10"}, {"all", "10"}}))
        ->excludes(niflag);

    try {
        parser.parse(std::string(args));
    }
    catch (const CLI::Error& ce) {
        logMessage(std::string("error processing command line arguments for web server :") +
                   ce.what());
    }
}

void WebServer::startServer(const nlohmann::json* val,
                            const std::shared_ptr<TypedBrokerServer>& ptr)
{
    logMessage("starting broker web server");
    config = (val != nullptr) ? val : &null;
    bool exp{false};
    if (running.compare_exchange_strong(exp, true)) {
        // The io_context is required for all I/O
        context = std::make_shared<IocWrapper>();

        const std::lock_guard<std::mutex> tlock(threadGuard);

        auto webptr = std::dynamic_pointer_cast<WebServer>(ptr);
        if (!webptr) {
            throw(std::invalid_argument("pointer to a webserver required for operation"));
        }
        mainLoopThread = std::thread([this, webptr = std::move(webptr)]() { mainLoop(webptr); });
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
        const std::lock_guard<std::mutex> tlock(threadGuard);
        context->ioc.stop();
    }
}

void WebServer::mainLoop(std::shared_ptr<WebServer> keepAlive)
{
    if (mHttpEnabled) {
        auto httpInterfaceNetwork = mInterfaceNetwork;
        if (config->contains("http")) {
            const auto& httpConfig = (*config)["http"];
            helics::fileops::replaceIfMember(httpConfig, "interface", mHttpAddress);
            helics::fileops::replaceIfMember(httpConfig, "port", mHttpPort);
            const bool ipv4 = helics::fileops::getOrDefault(httpConfig, "ipv4", false);

            if (ipv4) {
                httpInterfaceNetwork = static_cast<int>(gmlc::networking::InterfaceNetworks::IPV4);
            }
            const bool ipv6 = helics::fileops::getOrDefault(httpConfig, "ipv6", false);
            if (ipv6) {
                httpInterfaceNetwork =
                    static_cast<int>((ipv4) ? gmlc::networking::InterfaceNetworks::ALL :
                                              gmlc::networking::InterfaceNetworks::IPV6);
            }

            bool external = helics::fileops::getOrDefault(httpConfig, "external", false);
            helics::fileops::replaceIfMember(httpConfig, "all", external);
            if (external) {
                httpInterfaceNetwork = static_cast<int>(gmlc::networking::InterfaceNetworks::ALL);
            }
        }
        mHttpAddress = gmlc::networking::generateMatchingInterfaceAddress(
            mHttpAddress, static_cast<gmlc::networking::InterfaceNetworks>(httpInterfaceNetwork));
        gmlc::networking::removeProtocol(mHttpAddress);
        if (mHttpAddress == "*") {
            mHttpAddress = "0.0.0.0";
        }
        auto const address = net::ip::make_address(mHttpAddress);
        // Create and launch a listening port
        std::make_shared<Listener>(context->ioc,
                                   tcp::endpoint{address, static_cast<std::uint16_t>(mHttpPort)})
            ->run();
    }

    if (mWebsocketEnabled) {
        auto websocketInterfaceNetwork = mInterfaceNetwork;
        if (config->contains("websocket")) {
            const auto& webConfig = (*config)["websocket"];
            helics::fileops::replaceIfMember(webConfig, "interface", mWebsocketAddress);
            helics::fileops::replaceIfMember(webConfig, "port", mWebsocketPort);

            const bool ipv4 = helics::fileops::getOrDefault(webConfig, "ipv4", false);

            if (ipv4) {
                websocketInterfaceNetwork =
                    static_cast<int>(gmlc::networking::InterfaceNetworks::IPV4);
            }
            const bool ipv6 = helics::fileops::getOrDefault(webConfig, "ipv6", false);
            if (ipv6) {
                websocketInterfaceNetwork =
                    static_cast<int>((ipv4) ? gmlc::networking::InterfaceNetworks::ALL :
                                              gmlc::networking::InterfaceNetworks::IPV6);
            }

            bool external = helics::fileops::getOrDefault(webConfig, "external", false);
            helics::fileops::replaceIfMember(webConfig, "all", external);
            if (external) {
                websocketInterfaceNetwork =
                    static_cast<int>(gmlc::networking::InterfaceNetworks::ALL);
            }
        }
        mWebsocketAddress = gmlc::networking::generateMatchingInterfaceAddress(
            mWebsocketAddress,
            static_cast<gmlc::networking::InterfaceNetworks>(websocketInterfaceNetwork));
        gmlc::networking::removeProtocol(mWebsocketAddress);
        if (mWebsocketAddress == "*") {
            mWebsocketAddress = "0.0.0.0";
        }
        auto const address = net::ip::make_address(mWebsocketAddress);
        // Create and launch a listening port
        std::make_shared<Listener>(context->ioc,
                                   tcp::endpoint{address,
                                                 static_cast<std::uint16_t>(mWebsocketPort)},
                                   true)
            ->run();
    }
    executing.store(true);
    // Run the I/O service
    if (running.load()) {
        context->ioc.run();
    }
    executing.store(false);
    keepAlive.reset();
}

}  // namespace helics::apps
