/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "coreTypeOperations.hpp"

#include "../common/configFileHelpers.hpp"
#include "CoreTypes.hpp"
#include "core-exceptions.hpp"
#include "helics/helics-config.h"
#include "helicsCLI11JsonConfig.hpp"

#include <algorithm>
#include <cctype>
#include <frozen/set.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace frozen {
template<>
struct elsa<std::string_view> {
    constexpr std::size_t operator()(std::string_view value) const { return hash_string(value); }
    constexpr std::size_t operator()(std::string_view value, std::size_t seed) const
    {
        return hash_string(value, seed);
    }
};
}  // namespace frozen

namespace helics::core {
std::string to_string(CoreType type)
{
    switch (type) {
        case CoreType::MPI:
            return "mpi_";
        case CoreType::TEST:
            return "test_";
        case CoreType::ZMQ:
            return "zmq_";
        case CoreType::ZMQ_SS:
            return "zmqss_";
        case CoreType::INTERPROCESS:
        case CoreType::IPC:
            return "ipc_";
        case CoreType::TCP:
            return "tcp_";
        case CoreType::TCP_SS:
            return "tcpss_";
        case CoreType::HTTP:
            return "http_";
        case CoreType::UDP:
            return "udp_";
        case CoreType::NNG:
            return "nng_";
        case CoreType::INPROC:
            return "inproc_";
        case CoreType::WEBSOCKET:
            return "websocket_";
        case CoreType::NULLCORE:
            return "null_";
        case CoreType::EMPTY:
            return "empty_";
        default:
            return {};
    }
}
static constexpr frozen::unordered_map<std::string_view, CoreType, 57> coreTypes{
    {"default", CoreType::DEFAULT},
    {"def", CoreType::DEFAULT},
    {"mpi", CoreType::MPI},
    {"message_passing_interface", CoreType::MPI},
    {"MPI", CoreType::MPI},
    {"ZMQ", CoreType::ZMQ},
    {"0mq", CoreType::ZMQ},
    {"zmq", CoreType::ZMQ},
    {"zeromq", CoreType::ZMQ},
    {"zeromq_ss", CoreType::ZMQ_SS},
    {"zmqss", CoreType::ZMQ_SS},
    {"ZMQSS", CoreType::ZMQ_SS},
    {"zmq_ss", CoreType::ZMQ_SS},
    {"ZMQ_SS", CoreType::ZMQ_SS},
    {"0mq_ss", CoreType::ZMQ_SS},
    {"0mqss", CoreType::ZMQ_SS},
    {"zeromq2", CoreType::ZMQ_SS},
    {"zmq2", CoreType::ZMQ_SS},
    {"ZMQ2", CoreType::ZMQ_SS},
    {"interprocess", CoreType::INTERPROCESS},
    {"ZeroMQ", CoreType::ZMQ},
    {"ZeroMQ2", CoreType::ZMQ_SS},
    {"ipc", CoreType::INTERPROCESS},
    {"interproc", CoreType::INTERPROCESS},
    {"IPC", CoreType::INTERPROCESS},
    {"tcp", CoreType::TCP},
    {"tcpip", CoreType::TCP},
    {"TCP", CoreType::TCP},
    {"TCPIP", CoreType::TCP},
    {"tcpss", CoreType::TCP_SS},
    {"tcpipss", CoreType::TCP_SS},
    {"TCPSS", CoreType::TCP_SS},
    {"TCPIPSS", CoreType::TCP_SS},
    {"tcp_ss", CoreType::TCP_SS},
    {"tcpip_ss", CoreType::TCP_SS},
    {"TCP_SS", CoreType::TCP_SS},
    {"TCPIP_SS", CoreType::TCP_SS},
    {"single_socket", CoreType::TCP_SS},
    {"single socket", CoreType::TCP_SS},
    {"ss", CoreType::TCP_SS},
    {"udp", CoreType::UDP},
    {"test", CoreType::TEST},
    {"UDP", CoreType::UDP},
    {"local", CoreType::TEST},
    {"inprocess", CoreType::INPROC},
    {"websocket", CoreType::WEBSOCKET},
    {"web", CoreType::WEBSOCKET},
    {"inproc", CoreType::INPROC},
    {"nng", CoreType::NNG},
    {"null", CoreType::NULLCORE},
    {"nullcore", CoreType::NULLCORE},
    {"none", CoreType::NULLCORE},
    {"empty", CoreType::EMPTY},
    {"http", CoreType::HTTP},
    {"HTTP", CoreType::HTTP},
    {"test1", CoreType::TEST},
    {"multi", CoreType::MULTI}};

CoreType coreTypeFromString(std::string_view type) noexcept
{
    if (type.empty()) {
        return CoreType::DEFAULT;
    }
    // in case it came from the to_string(CoreType)
    if (type.back() == '_') {
        type.remove_suffix(1);
    }
    const auto* fnd = coreTypes.find(type);
    if (fnd != coreTypes.end()) {
        return fnd->second;
    }
    std::string type2{type};
    std::transform(type2.cbegin(), type2.cend(), type2.begin(), ::tolower);
    fnd = coreTypes.find(type2);
    if (fnd != coreTypes.end()) {
        return fnd->second;
    }
    if ((type2.front() == '=') || (type2.front() == '-')) {
        return coreTypeFromString(type2.substr(1));
    }
    if (type.compare(0, 5, "zmqss") == 0) {
        return CoreType::ZMQ_SS;
    }
    if (type.compare(0, 6, "zmq_ss") == 0) {
        return CoreType::ZMQ_SS;
    }
    if (type.compare(0, 4, "zmq2") == 0) {
        return CoreType::ZMQ_SS;
    }
    if (type.compare(0, 3, "zmq") == 0) {
        return CoreType::ZMQ;
    }
    if (type.compare(0, 3, "ipc") == 0) {
        return CoreType::INTERPROCESS;
    }
    if (type.compare(0, 4, "test") == 0) {
        return CoreType::TEST;
    }
    if (type.compare(0, 5, "tcpss") == 0) {
        return CoreType::TCP_SS;
    }
    if (type.compare(0, 3, "tcp") == 0) {
        return CoreType::TCP;
    }
    if (type.compare(0, 3, "udp") == 0) {
        return CoreType::UDP;
    }
    if (type.compare(0, 4, "http") == 0) {
        return CoreType::HTTP;
    }
    if (type.compare(0, 3, "mpi") == 0) {
        return CoreType::MPI;
    }
    if (type.compare(0, 6, "inproc") == 0) {
        return CoreType::INPROC;
    }
    if (type.compare(0, 3, "web") == 0) {
        return CoreType::WEBSOCKET;
    }
    if (type.compare(0, 4, "null") == 0) {
        return CoreType::NULLCORE;
    }

    return CoreType::UNRECOGNIZED;
}

#ifndef HELICS_ENABLE_ZMQ_CORE
static bool constexpr zmq_availability{false};
#else
static bool constexpr zmq_availability{true};
#endif

#ifndef HELICS_ENABLE_MPI_CORE
static bool constexpr mpi_availability{false};
#else
static bool constexpr mpi_availability{true};
#endif

#ifndef HELICS_ENABLE_TCP_CORE
static bool constexpr tcp_availability{false};
#else
static bool constexpr tcp_availability{true};
#endif

#ifndef HELICS_ENABLE_UDP_CORE
static bool constexpr udp_availability{false};
#else
static bool constexpr udp_availability{true};
#endif

#ifndef HELICS_ENABLE_IPC_CORE
static bool constexpr ipc_availability{false};
#else
static bool constexpr ipc_availability{true};
#endif

#ifndef HELICS_ENABLE_TEST_CORE
static bool constexpr test_availability{false};
#else
static bool constexpr test_availability{true};
#endif

#ifndef HELICS_ENABLE_INPROC_CORE
static bool constexpr inproc_availability{false};
#else
static bool constexpr inproc_availability{true};
#endif

bool isCoreTypeAvailable(CoreType type) noexcept
{
    bool available{false};

    switch (type) {
        case CoreType::ZMQ:
        case CoreType::ZMQ_SS:
            available = zmq_availability;
            break;
        case CoreType::MPI:
            available = mpi_availability;
            break;
        case CoreType::TEST:
            available = test_availability;
            break;
        case CoreType::INTERPROCESS:
        case CoreType::IPC:
            available = ipc_availability;
            break;
        case CoreType::UDP:
            available = udp_availability;
            break;
        case CoreType::TCP:
        case CoreType::TCP_SS:
            available = tcp_availability;
            break;
        case CoreType::DEFAULT:  // default and empty should always be available
        case CoreType::EMPTY:
            available = true;
            break;
        case CoreType::INPROC:
            available = inproc_availability;
            break;
        case CoreType::HTTP:
        case CoreType::WEBSOCKET:
            return false;  // these are not yet built
            break;
        case CoreType::NULLCORE:
            available = false;  // nullcore is never available
            break;
        default:
            break;
    }

    return available;
}

static constexpr frozen::set<std::string_view, 5> global_match_strings{"any",
                                                                       "all",
                                                                       "data",
                                                                       "string",
                                                                       "block"};

bool matchingTypes(std::string_view type1, std::string_view type2)
{
    if (type1 == type2) {
        return true;
    }
    if ((type1.empty()) || (type2.empty())) {
        return true;
    }
    if ((type1.compare(0, 3, "def") == 0) || (type2.compare(0, 3, "def") == 0)) {
        return true;
    }
    const auto* res = global_match_strings.find(type1);
    if (res != global_match_strings.end()) {
        return true;
    }
    res = global_match_strings.find(type2);
    return (res != global_match_strings.end());
}

namespace {
    std::unique_ptr<helicsCLI11App> makeCLITypeApp(std::string& corestring, std::string& namestring)
    {
        auto app = std::make_unique<helicsCLI11App>("type extraction app");
        app->remove_helics_specifics();
        app->option_defaults()->ignore_case()->ignore_underscore();
        app->allow_config_extras(CLI::config_extras_mode::ignore_all);
        app->allow_extras();
        app->set_config("--config-file,--config", "");
        app->add_option("--name,--identifier", namestring);
        auto* fmtr = addJsonConfig(app.get());
        fmtr->maxLayers(0);
        fmtr->promoteSection("helics");
        auto* networking = app->add_option_group("network type")->immediate_callback();
        networking->add_option("--core", corestring);
        networking->add_option("--coretype,-t", corestring)->envname("HELICS_CORE_TYPE");
        app->add_subcommand("broker")->fallthrough();
        app->add_subcommand("core")->fallthrough();
        return app;
    }
}  // namespace

std::pair<CoreType, std::string> extractCoreType(const std::string& configureString)
{
    std::string corestring;
    std::string namestring;
    auto app = makeCLITypeApp(corestring, namestring);
    auto type = fileops::getConfigType(configureString);
    try {
        switch (type) {
            case fileops::ConfigType::JSON_FILE: {
                std::ifstream file{configureString};
                app->parse_from_stream(file);
                break;
            }
            case fileops::ConfigType::JSON_STRING: {
                std::istringstream jstring{configureString};
                app->parse_from_stream(jstring);
                break;
            }
            case fileops::ConfigType::TOML_FILE: {
                app->allow_extras();
                auto dptr =
                    std::static_pointer_cast<HelicsConfigJSON>(app->get_config_formatter_base());
                if (dptr) {
                    dptr->skipJson(true);
                }
                std::ifstream file{configureString};
                app->parse_from_stream(file);
                break;
            }
            case fileops::ConfigType::TOML_STRING: {
                app->allow_extras();
                auto dptr =
                    std::static_pointer_cast<HelicsConfigJSON>(app->get_config_formatter_base());
                if (dptr) {
                    dptr->skipJson(true);
                }
                std::istringstream tstring{configureString};
                app->parse_from_stream(tstring);
                break;
            }
            case fileops::ConfigType::CMD_LINE:
                app->helics_parse(configureString);
                break;
            case fileops::ConfigType::NONE:
                break;
        }
    }
    catch (const std::exception&) {
        corestring.clear();
    }
    if (corestring.empty()) {
        return {CoreType::DEFAULT, namestring};
    }
    return {coreTypeFromString(corestring), namestring};
}

std::pair<CoreType, std::string> extractCoreType(const std::vector<std::string>& args)
{
    std::string corestring;
    std::string namestring;
    auto app = makeCLITypeApp(corestring, namestring);
    auto vec = args;
    app->helics_parse(vec);
    if (corestring.empty()) {
        return {CoreType::DEFAULT, namestring};
    }
    return {coreTypeFromString(corestring), namestring};
}

std::pair<CoreType, std::string> extractCoreType(int argc, char* argv[])
{
    std::string corestring;
    std::string namestring;
    auto app = makeCLITypeApp(corestring, namestring);
    app->helics_parse(argc, argv);
    if (corestring.empty()) {
        return {CoreType::DEFAULT, namestring};
    }
    return {coreTypeFromString(corestring), namestring};
}

}  // namespace helics::core

namespace helics {

std::string_view interfaceTypeName(InterfaceType type) noexcept
{
    static constexpr std::string_view pubType{"Publication"};
    static constexpr std::string_view inpType{"Input"};
    static constexpr std::string_view endType{"Endpoint"};
    static constexpr std::string_view transType{"Translator"};
    static constexpr std::string_view filtType{"Filter"};
    static constexpr std::string_view otherType{"Interface"};

    switch (type) {
        case InterfaceType::PUBLICATION:
            return pubType;
        case InterfaceType::ENDPOINT:
            return endType;
        case InterfaceType::INPUT:
            return inpType;
        case InterfaceType::FILTER:
            return filtType;
        case InterfaceType::TRANSLATOR:
            return transType;
        default:
            return otherType;
    }
}
}  // namespace helics
