/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Connector.hpp"

#include "../application_api/HelicsPrimaryTypes.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <deque>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace helics::apps {

/** dataclass for potential connections*/
struct PotentialConnections {
    std::string_view federate;
    std::string_view key;
    bool used{false};
};

/** data class for information from queries*/
struct ConnectionsList {
    std::unordered_multimap<std::string_view, std::string_view> aliases;
    std::vector<std::string_view> unconnectedPubs;
    std::vector<std::string_view> unconnectedInputs;
    std::vector<std::string_view> unconnectedTargetEndpoints;
    std::vector<std::string_view> unconnectedSourceEndpoints;
    std::unordered_set<std::string_view> pubs;
    std::unordered_set<std::string_view> inputs;
    std::unordered_set<std::string_view> endpoints;

    std::unordered_map<std::string_view, PotentialConnections> potentialPubs;
    std::unordered_map<std::string_view, PotentialConnections> potentialInputs;
    std::unordered_map<std::string_view, PotentialConnections> potentialEndpoints;
    std::deque<std::string> interfaces;
    std::deque<std::string> federatesWithPotentialInterfaces;
    std::vector<std::string> unknownPubs;
    std::vector<std::string> unknownInputs;
    std::vector<std::string> unknownEndpoints;
    std::vector<std::string> tagStrings;
    std::vector<std::size_t> tagCodes;
    bool hasPotentialInterfaces{false};
};

static void loadTags(ConnectionsList& connections, const Json::Value& tags)
{
    for (Json::Value::const_iterator itr = tags.begin(); itr != tags.end(); ++itr) {
        if (itr.key().asString() == "tags") {
            auto tagVect = gmlc::utilities::stringOps::splitlineQuotes(itr->asString());
            connections.tagStrings.insert(connections.tagStrings.end(),
                                          tagVect.begin(),
                                          tagVect.end());
        } else {
            if (!itr->isString() || isTrueString(itr->asCString())) {
                connections.tagStrings.emplace_back(itr.key().asString());
            }
        }
    }
}
static void coreConnectionList(ConnectionsList& connections, Json::Value& core)
{
    if (core.isMember("tags")) {
        loadTags(connections, core["tags"]);
    }
    if (core.isMember("federates")) {
        for (const auto& fed : core["federates"]) {
            try {
                if (fed.isMember("tags")) {
                    loadTags(connections, fed["tags"]);
                }
                if (fed.isMember("connected_inputs")) {
                    for (const auto& input : fed["connected_inputs"]) {
                        const std::string_view input1 =
                            connections.interfaces.emplace_back(input.asString());
                        connections.inputs.insert(input1);
                    }
                }
                if (fed.isMember("connected_publications")) {
                    for (const auto& pub : fed["connected_publications"]) {
                        const std::string_view pub1 =
                            connections.interfaces.emplace_back(pub.asString());
                        connections.pubs.insert(pub1);
                    }
                }
                if (fed.isMember("unconnected_inputs")) {
                    for (const auto& input : fed["unconnected_inputs"]) {
                        const std::string_view input1 =
                            connections.interfaces.emplace_back(input.asString());
                        connections.unconnectedInputs.push_back(input1);
                        connections.inputs.insert(input1);
                    }
                }
                if (fed.isMember("unconnected_publications")) {
                    for (const auto& pub : fed["unconnected_publications"]) {
                        const std::string_view pub1 =
                            connections.interfaces.emplace_back(pub.asString());
                        connections.unconnectedPubs.push_back(pub1);
                        connections.pubs.insert(pub1);
                    }
                }

                if (fed.isMember("unconnected_target_endpoints")) {
                    for (const auto& endpoint : fed["unconnected_target_endpoints"]) {
                        const std::string_view end1 =
                            connections.interfaces.emplace_back(endpoint.asString());
                        connections.unconnectedTargetEndpoints.push_back(end1);
                        connections.endpoints.insert(end1);
                    }
                }
                if (fed.isMember("unconnected_source_endpoints")) {
                    for (const auto& endpoint : fed["unconnected_source_endpoints"]) {
                        const std::string_view end1 =
                            connections.interfaces.emplace_back(endpoint.asString());
                        connections.unconnectedSourceEndpoints.push_back(end1);
                        connections.endpoints.insert(end1);
                    }
                }
                if (fed.isMember("connected_endpoints")) {
                    for (const auto& endpoint : fed["connected_endpoints"]) {
                        const std::string_view end1 =
                            connections.interfaces.emplace_back(endpoint.asString());
                        connections.endpoints.insert(end1);
                    }
                }
                if (fed.isMember("potential_interfaces")) {
                    connections.hasPotentialInterfaces = true;
                    const std::string_view federateName =
                        connections.federatesWithPotentialInterfaces.emplace_back(
                            fed["attributes"]["name"].asCString());
                    const auto& potInterfaces = fed["potential_interfaces"];
                    if (potInterfaces.isMember("inputs")) {
                        for (const auto& input : potInterfaces["inputs"]) {
                            if (input.isObject()) {
                                auto name = fileops::getName(input);
                                if (name.empty()) {
                                    continue;
                                }
                                const std::string_view input1 =
                                    connections.interfaces.emplace_back(name);
                                connections.potentialInputs.emplace(
                                    input1, PotentialConnections{federateName, input1, false});
                            } else if (input.isString()) {
                                const std::string_view input1 =
                                    connections.interfaces.emplace_back(input.asCString());
                                connections.potentialInputs.emplace(
                                    input1, PotentialConnections{federateName, input1, false});
                            }
                        }
                    }
                    if (potInterfaces.isMember("publications")) {
                        for (const auto& pub : potInterfaces["publications"]) {
                            if (pub.isObject()) {
                                auto name = fileops::getName(pub);
                                if (name.empty()) {
                                    continue;
                                }
                                const std::string_view pub1 =
                                    connections.interfaces.emplace_back(name);
                                connections.potentialPubs.emplace(
                                    pub1, PotentialConnections{federateName, pub1, false});
                            } else if (pub.isString()) {
                                const std::string_view pub1 =
                                    connections.interfaces.emplace_back(pub.asCString());
                                connections.potentialPubs.emplace(
                                    pub1, PotentialConnections{federateName, pub1, false});
                            }
                        }
                    }
                    if (potInterfaces.isMember("endpoints")) {
                        for (const auto& endpoint : potInterfaces["endpoints"]) {
                            if (endpoint.isObject()) {
                                auto name = fileops::getName(endpoint);
                                if (name.empty()) {
                                    continue;
                                }
                                const std::string_view endpoint1 =
                                    connections.interfaces.emplace_back(name);
                                connections.potentialEndpoints.emplace(
                                    endpoint1,
                                    PotentialConnections{federateName, endpoint1, false});
                            } else {
                                const std::string_view endpoint1 =
                                    connections.interfaces.emplace_back(endpoint.asCString());
                                connections.potentialEndpoints.emplace(
                                    endpoint1,
                                    PotentialConnections{federateName, endpoint1, false});
                            }
                        }
                    }
                }
            }
            catch (const Json::Exception& /*ev*/) {
                // TODO(PT): I think this is going to be almost impossible now, but someday might
                // want to create a response
                continue;
            }
        }
    }
}

static void brokerConnectionList(ConnectionsList& connections, Json::Value& broker)
{
    for (auto& subBroker : broker["brokers"]) {
        brokerConnectionList(connections, subBroker);
    }
    for (auto& core : broker["cores"]) {
        coreConnectionList(connections, core);
    }
}

static ConnectionsList generateConnectionsList(const std::string& connectionData)
{
    ConnectionsList connections;
    auto json = fileops::loadJsonStr(connectionData);
    if (json.isMember("aliases")) {
        for (auto& alias : json["aliases"]) {
            const std::string_view alias1 =
                connections.interfaces.emplace_back(alias[0].asString());
            const std::string_view alias2 =
                connections.interfaces.emplace_back(alias[1].asString());
            connections.aliases.emplace(alias1, alias2);
        }
    }
    if (json.isMember("tags")) {
        loadTags(connections, json["tags"]);
    }
    if (json.isMember("unknown_inputs")) {
        for (const auto& input : json["unknown_inputs"]) {
            connections.unknownInputs.push_back(input.asString());
        }
    }
    if (json.isMember("unknown_publications")) {
        for (const auto& pub : json["unknown_publications"]) {
            connections.unknownPubs.push_back(pub.asString());
        }
    }
    if (json.isMember("unknown_endpoints")) {
        for (const auto& ept : json["unknown_endpoints"]) {
            connections.unknownEndpoints.push_back(ept.asString());
        }
    }
    for (auto& broker : json["brokers"]) {
        brokerConnectionList(connections, broker);
    }
    for (auto& core : json["cores"]) {
        coreConnectionList(connections, core);
    }
    connections.tagStrings.push_back("default");
    connections.tagCodes.reserve(connections.tagStrings.size());
    std::transform(connections.tagStrings.begin(),
                   connections.tagStrings.end(),
                   std::back_inserter(connections.tagCodes),
                   std::hash<std::string>());
    return connections;
}

Connector::Connector(std::vector<std::string> args):
    App("connector", std::move(args)), core((fed) ? fed->getCorePointer() : nullptr)
{
    processArgs();
    initialSetup();
}

Connector::Connector(int argc, char* argv[]):
    App("connector", argc, argv), core((fed) ? fed->getCorePointer() : nullptr)
{
    processArgs();
    initialSetup();
}

void Connector::processArgs()
{
    auto app = generateParser();

    if (!deactivated) {
        app->helics_parse(remArgs);
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}

void Connector::initialSetup()
{
    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
        loadInputFiles();
    }
}

const std::unordered_map<std::string_view, InterfaceDirection> directionNames{
    {"from_to", InterfaceDirection::FROM_TO},
    {"1", InterfaceDirection::FROM_TO},
    {"FROM_TO", InterfaceDirection::FROM_TO},
    {"to_from", InterfaceDirection::TO_FROM},
    {"TO_FROM", InterfaceDirection::TO_FROM},
    {"-1", InterfaceDirection::TO_FROM},
    {"bidirectional", InterfaceDirection::BIDIRECTIONAL},
    {"BIDIRECTIONAL", InterfaceDirection::BIDIRECTIONAL},
    {"0", InterfaceDirection::BIDIRECTIONAL},
    {"bi", InterfaceDirection::BIDIRECTIONAL},
    {"BI", InterfaceDirection::BIDIRECTIONAL}};

std::optional<InterfaceDirection> getDirection(std::string_view direction)
{
    auto res = directionNames.find(direction);
    if (res != directionNames.end()) {
        return res->second;
    }
    return std::nullopt;
}

std::unique_ptr<helicsCLI11App> Connector::generateParser()
{
    auto app = std::make_unique<helicsCLI11App>("Command line options for the Connector App");
    app->add_option_function<std::vector<std::vector<std::string>>>(
           "--connection",
           [this](const std::vector<std::vector<std::string>>& args) {
               for (const auto& conn : args) {
                   addConnectionVector(conn);
               }
           },
           "specify connections to make in the cosimulation")
        ->expected(2, CLI::detail::expected_max_vector_size)
        ->type_name("[INTERFACE1,INTERFACE2,DIRECTIONALITY,TAGS...]");
    app->add_flag("--match_target_endpoints",
                  matchTargetEndpoints,
                  "set to true to enable connection of unconnected target endpoints")
        ->ignore_underscore();
    app->add_flag("--match_multiple",
                  matchMultiple,
                  "set to true to enable matching of multiple connections (default false)")
        ->ignore_underscore();
    app->add_flag("--always_check_regex",
                  alwaysCheckRegex,
                  "set to true to enable regex matching even if other matches are defined")
        ->ignore_underscore();

    return app;
}

Connector::Connector(std::string_view appName, const FederateInfo& fedInfo):
    App(appName, fedInfo), core((fed) ? fed->getCorePointer() : nullptr)
{
    initialSetup();
}

Connector::Connector(std::string_view appName,
                     const std::shared_ptr<Core>& core,
                     const FederateInfo& fedInfo):
    App(appName, core, fedInfo),
    core((fed) ? fed->getCorePointer() : nullptr)
{
    initialSetup();
}

Connector::Connector(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    App(appName, core, fedInfo), core((fed) ? fed->getCorePointer() : nullptr)
{
    initialSetup();
}

Connector::Connector(std::string_view appName, const std::string& configString):
    App(appName, configString), core((fed) ? fed->getCorePointer() : nullptr)
{
    initialSetup();
}

std::size_t Connector::addTag(std::string_view tagName)
{
    std::size_t hash = std::hash<std::string_view>()(tagName);
    tags.emplace(hash, tagName);
    return hash;
}

std::string_view Connector::addInterface(std::string_view interfaceName)
{
    auto interfaceIterator = interfaces.insert(std::string(interfaceName));
    return {*(interfaceIterator.first)};
}

bool Connector::addConnectionVector(const std::vector<std::string>& connection)
{
    if (connection.size() <= 1) {
        return false;
    }
    if (connection.size() == 2) {
        addConnection(connection[0], connection[1]);
        return true;
    }

    InterfaceDirection direction{InterfaceDirection::BIDIRECTIONAL};
    std::vector<std::string> newTags;
    auto directionValue = getDirection(connection[2]);
    if (directionValue) {
        direction = *directionValue;
    } else {
        newTags.push_back(connection[2]);
    }

    for (int ii = 3; ii < connection.size(); ++ii) {
        newTags.push_back(connection[ii]);
    }
    addConnection(connection[0], connection[1], direction, newTags);
    return true;
}

void Connector::addConnection(std::string_view interface1,
                              std::string_view interface2,
                              InterfaceDirection direction,
                              const std::vector<std::string>& connectionTags)
{
    std::vector<std::size_t> svtags;
    svtags.reserve(connectionTags.size());
    for (const auto& tag : connectionTags) {
        svtags.push_back(addTag(tag));
    }
    auto iview1 = addInterface(interface1);
    auto iview2 = addInterface(interface2);
    Connection conn{iview1, iview2, direction, std::move(svtags)};
    if (iview1.compare(0, 6, "REGEX:") == 0) {
        switch (direction) {
            case InterfaceDirection::TO_FROM:
                std::swap(conn.interface1, conn.interface2);
                matchers.emplace_back(std::move(conn));
                break;
            case InterfaceDirection::FROM_TO:
                matchers.emplace_back(std::move(conn));
                break;
            case InterfaceDirection::BIDIRECTIONAL:
                matchers.emplace_back(conn);
                std::swap(conn.interface1, conn.interface2);
                matchers.emplace_back(std::move(conn));
                break;
        }
    } else {
        switch (direction) {
            case InterfaceDirection::TO_FROM:
                connections.emplace(iview2, std::move(conn));
                break;
            case InterfaceDirection::FROM_TO:
                connections.emplace(iview1, std::move(conn));
                break;
            case InterfaceDirection::BIDIRECTIONAL:
                connections.emplace(iview2, conn);
                if (iview1 != iview2) {
                    connections.emplace(iview1, std::move(conn));
                }
                break;
        }
    }
}

class RegexMatcher {
  public:
    RegexMatcher() = default;

    std::regex rmatch;
    std::vector<std::string> keys;
    std::string_view interface1;
    std::string_view interface2;
    std::vector<std::size_t> tags;
    std::string generateMatch(std::string_view testString)
    {
        std::match_results<typename decltype(testString)::const_iterator> matchResults{};
        if (std::regex_match(testString.begin(), testString.end(), matchResults, rmatch)) {
            std::string matcher(interface2);
            if (matcher.compare(0, 6, "REGEX:") == 0) {
                matcher.erase(0, 6);
                for (std::size_t ii = 0; ii < keys.size(); ++ii) {
                    auto keyloc = matcher.find(keys[ii]);
                    while (keyloc != std::string::npos) {
                        auto endloc = matcher.find_first_of(')', keyloc);
                        matcher.replace(matcher.begin() + keyloc - 1,
                                        matcher.begin() + endloc + 1,
                                        matchResults[ii + 1].first,
                                        matchResults[ii + 1].second);
                        keyloc = matcher.find(keys[ii]);
                    }
                }
            }

            return matcher;
        }
        return {};
    }
};

void Connector::loadTextFile(const std::string& filename)
{
    using namespace gmlc::utilities::stringOps;  // NOLINT
    AppTextParser aparser(filename);
    auto cnts = aparser.preParseFile({});

    aparser.reset();

    if (!aparser.configString().empty()) {
        App::loadConfigOptions(aparser);
        auto app = generateParser();
        std::istringstream sstr(aparser.configString());
        app->parse_from_stream(sstr);
    }
    connections.reserve(connections.size() + cnts[0]);
    std::string str;
    int lineNumber;
    while (aparser.loadNextLine(str, lineNumber)) {
        /* time key type value units*/
        auto blk = splitlineQuotes(str, ",\t ", default_quote_chars, delimiter_compression::on);
        for (auto& seq : blk) {
            CLI::detail::remove_quotes(seq);
        }
        addConnectionVector(blk);
    }
}

void Connector::loadJsonFile(const std::string& jsonString,
                             bool enableFederateInterfaceRegistration)
{
    loadJsonFileConfiguration("connector", jsonString, enableFederateInterfaceRegistration);

    auto doc = fileops::loadJson(jsonString);

    if (doc.isMember("connector")) {
        auto connectorConfig = doc["connector"];
        matchTargetEndpoints =
            fileops::getOrDefault(connectorConfig, "match_target_endpoints", matchTargetEndpoints);
        matchMultiple = fileops::getOrDefault(connectorConfig, "match_multiple", matchMultiple);
        alwaysCheckRegex =
            fileops::getOrDefault(connectorConfig, "always_check_regex", alwaysCheckRegex);
    }
    auto connectionArray = doc["connections"];
    if (connectionArray.isArray()) {
        connections.reserve(connections.size() + connectionArray.size());
        for (const auto& connectionElement : connectionArray) {
            std::vector<std::string> connectionObject;
            for (const auto& subElement : connectionElement) {
                connectionObject.push_back(subElement.asString());
            }
            addConnectionVector(connectionObject);
        }
    }
}

std::vector<Connection>
    Connector::buildPossibleConnectionList(std::string_view startingInterface,
                                           const std::vector<std::size_t>& tagList) const
{
    std::vector<Connection> matches;
    auto [first, last] = connections.equal_range(startingInterface);
    if (first != connections.end()) {
        std::set<std::string_view> searched;
        searched.insert(startingInterface);

        for (auto match = first; match != last; ++match) {
            if (match->second.tags.empty() ||
                std::find_first_of(tagList.begin(),
                                   tagList.end(),
                                   match->second.tags.begin(),
                                   match->second.tags.end()) != tagList.end()) {
                matches.emplace_back(match->second);
                if (matches.back().interface1 != startingInterface) {
                    std::swap(matches.back().interface1, matches.back().interface2);
                }
            }
        }
        std::size_t cascadeIndex{0};
        while (cascadeIndex < matches.size()) {
            if (searched.find(matches[cascadeIndex].interface2) != searched.end()) {
                ++cascadeIndex;
                continue;
            }
            searched.insert(matches[cascadeIndex].interface2);
            std::tie(first, last) = connections.equal_range(matches[cascadeIndex].interface2);
            if (first != connections.end()) {
                for (auto match = first; match != last; ++match) {
                    if (match->second.tags.empty() ||
                        std::find_first_of(tagList.begin(),
                                           tagList.end(),
                                           match->second.tags.begin(),
                                           match->second.tags.end()) != tagList.end()) {
                        matches.emplace_back(match->second);
                        if (matches.back().interface1 != matches[cascadeIndex].interface2) {
                            std::swap(matches.back().interface1, matches.back().interface2);
                        }
                        if (searched.find(matches.back().interface2) != searched.end()) {
                            // this would already be references and create a cyclic reference
                            matches.pop_back();
                        }
                    }
                }
            }
            ++cascadeIndex;
        }
    }
    if (matches.empty() || alwaysCheckRegex) {
        if (!regexMatchers.empty()) {
            for (const auto& rmatcher : regexMatchers) {
                if (rmatcher->tags.empty() ||
                    std::find_first_of(tagList.begin(),
                                       tagList.end(),
                                       rmatcher->tags.begin(),
                                       rmatcher->tags.end()) != tagList.end()) {
                    auto mstring = rmatcher->generateMatch(startingInterface);
                    if (!mstring.empty()) {
                        Connection connection;
                        connection.stringBuffer = std::make_shared<std::string>(mstring);
                        connection.interface1 = rmatcher->interface1;
                        connection.tags = rmatcher->tags;
                        connection.interface2 = *connection.stringBuffer;
                        connection.direction = InterfaceDirection::FROM_TO;
                        matches.push_back(std::move(connection));
                    }
                }
            }
        }
    }
    return matches;
}

static std::set<std::string_view>
    generateAliases(const std::string_view target,
                    const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    std::set<std::string_view> matches;
    auto [first, last] = aliases.equal_range(target);
    if (first == aliases.end()) {
        return matches;
    }
    matches.emplace(target);
    std::vector<std::string_view> matchList;
    for (auto match = first; match != last; ++match) {
        matches.emplace(match->second);
        matchList.emplace_back(match->second);
    }
    std::size_t cascadeIndex{1};
    while (cascadeIndex < matchList.size()) {
        std::tie(first, last) = aliases.equal_range(matchList[cascadeIndex]);
        for (auto match = first; match != last; ++match) {
            auto [iterator, newAlias] = matches.emplace(match->second);
            if (newAlias) {
                matchList.emplace_back(match->second);
            }
        }
        ++cascadeIndex;
    }
    return matches;
}

bool Connector::makePotentialConnection(
    std::string_view interface,
    const std::vector<std::size_t>& tagList,
    std::unordered_map<std::string_view, PotentialConnections>& potentials,
    const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    auto connectionOptions = buildPossibleConnectionList(interface, tagList);
    for (const auto& option : connectionOptions) {
        auto located = potentials.find(option.interface2);
        if (located != potentials.end()) {
            /* source, target*/
            located->second.used = true;
            return true;
        }
        auto aliasList = generateAliases(option.interface2, aliases);
        for (const auto& alias : aliasList) {
            located = potentials.find(alias);
            if (located != potentials.end()) {
                /* source, target*/
                located->second.used = true;
                return true;
            }
        }
    }
    return false;
}

bool Connector::checkPotentialConnection(
    std::string_view interfaceName,
    const std::vector<std::size_t>& tagList,
    std::unordered_set<std::string_view>& possibleConnections,
    std::unordered_map<std::string_view, PotentialConnections>& potentials,
    const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    static auto nullConnector = [this](std::string_view, std::string_view) {};
    /** potential inputs*/
    auto matched =
        makeTargetConnection(interfaceName, tagList, possibleConnections, aliases, nullConnector);
    if (matched > 0) {
        return true;
    }
    if (makePotentialConnection(interfaceName, tagList, potentials, aliases)) {
        return true;
    }
    if (!aliases.empty()) {
        auto aliasList = generateAliases(interfaceName, aliases);
        for (const auto& alias : aliasList) {
            if (alias == interfaceName) {
                continue;
            }
            if (makePotentialConnection(alias, tagList, potentials, aliases)) {
                return true;
            }
        }
    }
    return false;
}

int Connector::makeTargetConnection(
    std::string_view origin,
    const std::vector<std::size_t>& tagList,
    std::unordered_set<std::string_view>& possibleConnections,
    const std::unordered_multimap<std::string_view, std::string_view>& aliases,
    const std::function<void(std::string_view, std::string_view)>& callback)
{
    int matched{0};
    auto connectionOptions = buildPossibleConnectionList(origin, tagList);
    for (const auto& option : connectionOptions) {
        auto located = possibleConnections.find(option.interface2);
        if (located != possibleConnections.end()) {
            /* source, target*/
            callback(origin, option.interface2);
            ++matched;
            if (!matchMultiple) {
                return matched;
            }

        } else {
            if (!aliases.empty()) {
                auto aliasList = generateAliases(option.interface2, aliases);
                for (const auto& alias : aliasList) {
                    if (alias == option.interface2) {
                        continue;
                    }
                    located = possibleConnections.find(alias);
                    if (located != possibleConnections.end()) {
                        callback(origin, option.interface2);
                        ++matched;
                        if (!matchMultiple) {
                            return matched;
                        }
                        break;
                    }
                }
            }
        }
    }
    if (!aliases.empty()) {
        auto aliasList = generateAliases(origin, aliases);
        for (const auto& alias : aliasList) {
            if (alias == origin) {
                continue;
            }
            auto aliasOptions = buildPossibleConnectionList(alias, tagList);
            for (const auto& option : aliasOptions) {
                auto located = possibleConnections.find(option.interface2);
                if (located != possibleConnections.end()) {
                    /* source, target*/
                    callback(origin, option.interface2);
                    ++matched;
                    if (!matchMultiple) {
                        return matched;
                    }

                } else {
                    if (!aliases.empty()) {
                        auto interfaceAliasList = generateAliases(option.interface2, aliases);
                        for (const auto& interfaceAlias : interfaceAliasList) {
                            if (alias == option.interface2) {
                                continue;
                            }
                            located = possibleConnections.find(interfaceAlias);
                            if (located != possibleConnections.end()) {
                                callback(origin, option.interface2);
                                ++matched;
                                if (!matchMultiple) {
                                    return matched;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return matched;
}

void Connector::makeConnections(ConnectionsList& possibleConnections)
{
    int logLevel = fed->getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL);
    auto inputConnector = [this, logLevel](std::string_view origin, std::string_view source) {
        core.dataLink(source, origin);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting input {} to publication {}", origin, source));
        }
    };
    auto pubConnector = [this, logLevel](std::string_view origin, std::string_view target) {
        core.dataLink(origin, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting publication {} to input {}", origin, target));
        }
    };
    auto sourceEndpointConnector = [this, logLevel](std::string_view origin,
                                                    std::string_view target) {
        core.linkEndpoints(origin, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting source endpoint {} to target endpoint {}",
                                        origin,
                                        target));
        }
    };
    auto targetEndpointConnector = [this, logLevel](std::string_view origin,
                                                    std::string_view source) {
        core.linkEndpoints(source, origin);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting target endpoint {} to source endpoint {}",
                                        origin,
                                        source));
        }
    };

    auto tagList = possibleConnections.tagCodes;
    /** unconnected inputs*/
    for (const auto& uInp : possibleConnections.unconnectedInputs) {
        matchCount += makeTargetConnection(
            uInp, tagList, possibleConnections.pubs, possibleConnections.aliases, inputConnector);
    }
    /** unconnected publications*/
    for (const auto& uPub : possibleConnections.unconnectedPubs) {
        matchCount += makeTargetConnection(
            uPub, tagList, possibleConnections.inputs, possibleConnections.aliases, pubConnector);
    }

    /** unconnected source endpoints*/
    for (const auto& uEnd : possibleConnections.unconnectedSourceEndpoints) {
        matchCount += makeTargetConnection(uEnd,
                                           tagList,
                                           possibleConnections.endpoints,
                                           possibleConnections.aliases,
                                           sourceEndpointConnector);
    }

    if (matchTargetEndpoints) {
        /** unconnected target endpoints*/
        for (const auto& uEnd : possibleConnections.unconnectedTargetEndpoints) {
            matchCount += makeTargetConnection(uEnd,
                                               tagList,
                                               possibleConnections.endpoints,
                                               possibleConnections.aliases,
                                               targetEndpointConnector);
        }
    }
    if (logLevel >= HELICS_LOG_LEVEL_SUMMARY) {
        fed->logInfoMessage(fmt::format("{} connections made", matchCount));
    }
}

void Connector::establishPotentialInterfaces(ConnectionsList& possibleConnections)
{
    auto nullConnector = [this](std::string_view, std::string_view) {};
    auto tagList = possibleConnections.tagCodes;
    /** potential inputs*/
    for (auto& pInp : possibleConnections.potentialInputs) {
        if (checkPotentialConnection(pInp.first,
                                     tagList,
                                     possibleConnections.pubs,
                                     possibleConnections.potentialPubs,
                                     possibleConnections.aliases)) {
            pInp.second.used = true;
        }
    }
    /* potential publications*/
    for (auto& pPub : possibleConnections.potentialPubs) {
        if (pPub.second.used) {
            continue;
        }
        if (checkPotentialConnection(pPub.first,
                                     tagList,
                                     possibleConnections.inputs,
                                     possibleConnections.potentialInputs,
                                     possibleConnections.aliases)) {
            pPub.second.used = true;
        }
    }

    /* potential endpoints*/
    for (auto& pEnd : possibleConnections.potentialEndpoints) {
        if (pEnd.second.used) {
            continue;
        }
        if (checkPotentialConnection(pEnd.first,
                                     tagList,
                                     possibleConnections.endpoints,
                                     possibleConnections.potentialEndpoints,
                                     possibleConnections.aliases)) {
            pEnd.second.used = true;
        }
    }
    /** now try to match unconnected interfaces to some of the potential ones*/
    /** unconnected inputs*/
    for (const auto& uInp : possibleConnections.unconnectedInputs) {
        if (makePotentialConnection(
                uInp, tagList, possibleConnections.potentialPubs, possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uInp, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uInp) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            tagList,
                                            possibleConnections.potentialPubs,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }

    /** unconnected publications*/
    for (const auto& uPub : possibleConnections.unconnectedPubs) {
        if (makePotentialConnection(
                uPub, tagList, possibleConnections.potentialInputs, possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uPub, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uPub) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            tagList,
                                            possibleConnections.potentialInputs,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }

    /** unconnected source endpoints*/
    for (const auto& uEnd : possibleConnections.unconnectedSourceEndpoints) {
        if (makePotentialConnection(uEnd,
                                    tagList,
                                    possibleConnections.potentialEndpoints,
                                    possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uEnd, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uEnd) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            tagList,
                                            possibleConnections.potentialEndpoints,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }

    /** unconnected target endpoints*/
    for (const auto& uEnd : possibleConnections.unconnectedTargetEndpoints) {
        if (makePotentialConnection(uEnd,
                                    tagList,
                                    possibleConnections.potentialEndpoints,
                                    possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uEnd, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uEnd) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            tagList,
                                            possibleConnections.potentialEndpoints,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }

    for (const auto& uInp : possibleConnections.unknownInputs) {
        auto fnd = possibleConnections.potentialInputs.find(uInp);
        if (fnd != possibleConnections.potentialInputs.end()) {
            fnd->second.used = true;
        }
        auto aliasList = generateAliases(uInp, possibleConnections.aliases);
        for (const auto& alias : aliasList) {
            if (alias == uInp) {
                continue;
            }
            fnd = possibleConnections.potentialInputs.find(alias);
            if (fnd != possibleConnections.potentialInputs.end()) {
                fnd->second.used = true;
            }
        }
    }

    for (const auto& uPub : possibleConnections.unknownPubs) {
        auto fnd = possibleConnections.potentialPubs.find(uPub);
        if (fnd != possibleConnections.potentialPubs.end()) {
            fnd->second.used = true;
        }
        auto aliasList = generateAliases(uPub, possibleConnections.aliases);
        for (const auto& alias : aliasList) {
            if (alias == uPub) {
                continue;
            }
            fnd = possibleConnections.potentialPubs.find(alias);
            if (fnd != possibleConnections.potentialPubs.end()) {
                fnd->second.used = true;
            }
        }
    }

    for (const auto& uEpt : possibleConnections.unknownEndpoints) {
        auto fnd = possibleConnections.potentialEndpoints.find(uEpt);
        if (fnd != possibleConnections.potentialEndpoints.end()) {
            fnd->second.used = true;
        }
        auto aliasList = generateAliases(uEpt, possibleConnections.aliases);
        for (const auto& alias : aliasList) {
            if (alias == uEpt) {
                continue;
            }
            fnd = possibleConnections.potentialEndpoints.find(alias);
            if (fnd != possibleConnections.potentialEndpoints.end()) {
                fnd->second.used = true;
            }
        }
    }
    int logLevel = fed->getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL);
    for (auto& possibleFed : possibleConnections.federatesWithPotentialInterfaces) {
        Json::Value establishInterfaces;
        establishInterfaces["command"] = "register_interfaces";
        std::vector<std::remove_reference_t<decltype(*possibleConnections.potentialInputs.begin())>>
            enabledInputs;
        std::copy_if(possibleConnections.potentialInputs.begin(),
                     possibleConnections.potentialInputs.end(),
                     std::back_inserter(enabledInputs),
                     [possibleFed](auto& pInterface) {
                         return (pInterface.second.federate == possibleFed &&
                                 pInterface.second.used == true);
                     });
        if (!enabledInputs.empty()) {
            establishInterfaces["inputs"] = Json::arrayValue;
            for (const auto& input : enabledInputs) {
                establishInterfaces["inputs"].append(std::string(input.first));
                ++interfacesRequested;
                if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
                    fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                                    fmt::format("federate {} request input {}",
                                                possibleFed,
                                                input.first));
                }
            }
        }
        std::vector<std::remove_reference_t<decltype(*possibleConnections.potentialPubs.begin())>>
            enabledPublications;
        std::copy_if(possibleConnections.potentialPubs.begin(),
                     possibleConnections.potentialPubs.end(),
                     std::back_inserter(enabledPublications),
                     [possibleFed](auto& pInterface) {
                         return (pInterface.second.federate == possibleFed &&
                                 pInterface.second.used == true);
                     });
        if (!enabledPublications.empty()) {
            establishInterfaces["publications"] = Json::arrayValue;
            for (const auto& pub : enabledPublications) {
                establishInterfaces["publications"].append(std::string(pub.first));
                ++interfacesRequested;
                if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
                    fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                                    fmt::format("federate {} request publication {}",
                                                possibleFed,
                                                pub.first));
                }
            }
        }

        std::vector<
            std::remove_reference_t<decltype(*possibleConnections.potentialEndpoints.begin())>>
            enabledEndpoints;
        std::copy_if(possibleConnections.potentialEndpoints.begin(),
                     possibleConnections.potentialEndpoints.end(),
                     std::back_inserter(enabledEndpoints),
                     [possibleFed](auto& pInterface) {
                         return (pInterface.second.federate == possibleFed &&
                                 pInterface.second.used == true);
                     });
        if (!enabledEndpoints.empty()) {
            establishInterfaces["endpoints"] = Json::arrayValue;
            for (const auto& ept : enabledEndpoints) {
                establishInterfaces["endpoints"].append(std::string(ept.first));
                ++interfacesRequested;
                if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
                    fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                                    fmt::format("federate {} request endpoint {}",
                                                possibleFed,
                                                ept.first));
                }
            }
        }
        fed->sendCommand(possibleFed, fileops::generateJsonString(establishInterfaces));
        if (logLevel >= HELICS_LOG_LEVEL_SUMMARY) {
            fed->logInfoMessage(fmt::format("{} interfaces requested", interfacesRequested));
        }
    }
}

void Connector::generateRegexMatchers()
{
    for (auto& rmatch : matchers) {
        auto rmatcher = std::make_unique<RegexMatcher>();
        std::string rstring{rmatch.interface1.substr(6, std::string_view::npos)};
        auto nvloc = rstring.find("(?<");
        while (nvloc != std::string::npos) {
            auto finishloc = rstring.find_first_of('>', nvloc + 2);
            rmatcher->keys.push_back(rstring.substr(nvloc + 1, finishloc - nvloc));
            rstring.erase(rstring.begin() + nvloc + 1, rstring.begin() + finishloc + 1);
            nvloc = rstring.find("(?<");
        }
        rmatcher->interface2 = rmatch.interface2;
        rmatcher->tags = rmatch.tags;
        try {
            rmatcher->rmatch = std::regex(rstring);
            regexMatchers.push_back(std::move(rmatcher));
        }
        catch (const std::regex_error& e) {
            fed->localError(-101, e.what());
        }
    }
}

void Connector::initialize()
{
    auto cmode = fed->getCurrentMode();
    if (cmode == Federate::Modes::STARTUP) {
        if (!matchers.empty()) {
            generateRegexMatchers();
        }
        fed->enterInitializingModeIterative();

        auto connectionsData =
            generateConnectionsList(fed->query("root", "unconnected_interfaces"));
        if (connectionsData.hasPotentialInterfaces) {
            establishPotentialInterfaces(connectionsData);
            fed->enterInitializingModeIterative();
            // need to do this twice to sync enverything
            fed->enterInitializingModeIterative();
            connectionsData = generateConnectionsList(fed->query("root", "unconnected_interfaces"));
        }
        makeConnections(connectionsData);
        fed->enterInitializingMode();
    }
}

void Connector::runTo([[maybe_unused]] Time stopTime_input)
{
    auto cmode = fed->getCurrentMode();
    if (cmode == Federate::Modes::STARTUP) {
        initialize();
    }
    if (cmode < Federate::Modes::EXECUTING) {
        fed->enterExecutingMode();
    } else {
        fed->disconnect();
    }
}

}  // namespace helics::apps
