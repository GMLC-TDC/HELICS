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
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
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

class TemplateMatcher {
  public:
    std::string templateName;
    std::string_view federate;

  private:
    std::vector<std::unordered_map<std::string_view, std::pair<std::string, std::string>>>
        templatePossibilities;
    std::vector<std::tuple<std::string, std::string, std::string>> usedTemplates;
    std::vector<std::string> intermediaries;
    std::vector<std::deque<std::string>> keys;
    std::vector<std::size_t> combinations{0};

  public:
    /* methods*/
    void initialize();

    bool loadTemplate(const nlohmann::json& iTemplate);
    [[nodiscard]] std::optional<std::tuple<std::string_view, std::string_view, std::string_view>>
        isTemplateMatch(std::string_view testString) const;
    void setAsUsed(std::tuple<std::string_view, std::string_view, std::string_view> match);
    [[nodiscard]] std::size_t possibilitiesCount() const { return combinations.back(); }
    std::string instantiateTemplate(std::size_t index);
    [[nodiscard]] bool isUsed() const { return !usedTemplates.empty(); }
    nlohmann::json usedInterfaceGeneration();
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
    std::vector<TemplateMatcher> potentialPublicationTemplates;
    std::vector<TemplateMatcher> potentialInputTemplates;
    std::vector<TemplateMatcher> potentialEndpointTemplates;
    bool hasPotentialInterfaces{false};
};

static void loadTags(ConnectionsList& connections, const nlohmann::json& tags)
{
    for (const auto& val : tags.items()) {
        if (val.key() == "tags") {
            auto tagVect =
                gmlc::utilities::stringOps::splitlineQuotes(val.value().get<std::string>());
            connections.tagStrings.insert(connections.tagStrings.end(),
                                          tagVect.begin(),
                                          tagVect.end());
        } else {
            if (!val.value().is_string() || isTrueString(val.value().get<std::string>())) {
                connections.tagStrings.emplace_back(val.key());
            }
        }
    }
}

bool TemplateMatcher::loadTemplate(const nlohmann::json& iTemplate)
{
    templateName = fileops::getName(iTemplate);

    auto tnameIndex = templateName.find("${");
    if (tnameIndex == std::string::npos) {
        return false;
    }
    if (tnameIndex == 0) {
        intermediaries.emplace_back("");
    } else {
        intermediaries.push_back(templateName.substr(0, tnameIndex));
    }
    std::vector<std::string> valueNames;
    std::size_t index{0};
    while (tnameIndex != std::string::npos) {
        auto close = templateName.find_first_of('}', tnameIndex);
        const std::string tname = templateName.substr(tnameIndex + 2, close - tnameIndex - 2);
        if (iTemplate.contains("fields")) {
            if (!iTemplate["fields"].contains(tname)) {
                return false;
            }
        } else {
            if (!iTemplate.contains(tname)) {
                return false;
            }
        }

        valueNames.push_back(tname);

        tnameIndex = templateName.find("${", close + 1);
        if (tnameIndex == close + 1) {
            return false;
        }
        if (tnameIndex == std::string::npos) {
            intermediaries.push_back(templateName.substr(close + 1));
        } else {
            intermediaries.push_back(templateName.substr(close + 1, tnameIndex - close - 1));
        }

        ++index;
    }

    templatePossibilities.resize(valueNames.size());
    keys.resize(valueNames.size());
    const nlohmann::json& fieldRoot =
        (iTemplate.contains("fields")) ? iTemplate["fields"] : iTemplate;

    for (index = 0; index < valueNames.size(); ++index) {
        for (const auto& val : fieldRoot[valueNames[index]]) {
            std::pair<std::string, std::string> typeAndUnits;
            std::string_view keyval;
            if (val.is_array()) {
                keyval = keys[index].emplace_back(val[0].get<std::string>());
                switch (val.size()) {
                    case 1:
                        break;
                    case 2:
                        typeAndUnits.first = val[1].get<std::string>();
                        break;
                    case 3:
                    default:
                        typeAndUnits.first = val[1].get<std::string>();
                        typeAndUnits.second = val[2].get<std::string>();
                        break;
                }
            } else {
                keyval = keys[index].emplace_back(val.get<std::string>());
            }
            templatePossibilities[index].emplace(keyval, typeAndUnits);
        }
    }

    initialize();
    return true;
}

void TemplateMatcher::initialize()
{
    combinations.resize(keys.size() + 1);
    std::size_t possibilities = 1;
    for (const auto& field : keys) {
        possibilities *= field.size();
    }
    combinations.back() = possibilities;
    for (std::size_t ii = 0; ii < keys.size(); ++ii) {
        possibilities /= keys[ii].size();
        combinations[ii] = possibilities;
    }
}

std::optional<std::tuple<std::string_view, std::string_view, std::string_view>>
    TemplateMatcher::isTemplateMatch(std::string_view testString) const
{
    std::vector<std::size_t> intermediateIndices;
    intermediateIndices.reserve(intermediaries.size());
    std::size_t index{0};
    for (const auto& intermedial : intermediaries) {
        if (intermedial.empty()) {
            if (intermediateIndices.empty()) {
                intermediateIndices.push_back(index);
            } else {
                intermediateIndices.push_back(std::string::npos);
            }

        } else {
            index = testString.find_first_of(intermedial, index);
            if (index == std::string_view::npos) {
                return std::nullopt;
            }
            intermediateIndices.push_back(index);
            index += intermedial.size();
        }
    }
    index = 0;
    std::string_view iType;
    std::string_view iUnits;
    while (index < intermediateIndices.size() - 1) {
        auto index1 = intermediateIndices[index] + intermediaries[index].size();
        auto length = intermediateIndices[index + 1] - index1;
        auto tString = testString.substr(index1, length);
        auto loc = templatePossibilities[index].find(tString);
        if (loc == templatePossibilities[index].end()) {
            return std::nullopt;
        }
        if (!loc->second.first.empty()) {
            iType = loc->second.first;
        }
        if (!loc->second.second.empty()) {
            iUnits = loc->second.second;
        }
        ++index;
    }
    return {std::make_tuple(testString, iType, iUnits)};
}

void TemplateMatcher::setAsUsed(
    std::tuple<std::string_view, std::string_view, std::string_view> match)
{
    usedTemplates.emplace_back(std::get<0>(match), std::get<1>(match), std::get<2>(match));
}

std::string TemplateMatcher::instantiateTemplate(std::size_t index)
{
    if (index >= combinations.back()) {
        return {};
    }
    std::string rval = intermediaries.front();
    for (std::size_t ii = 0; ii < keys.size(); ++ii) {
        const std::size_t subIndex = index / combinations[ii];
        index = index % combinations[ii];
        rval.append(keys[ii][subIndex]);
        rval.append(intermediaries[ii + 1]);
    }
    return rval;
}

nlohmann::json TemplateMatcher::usedInterfaceGeneration()
{
    nlohmann::json generator = nlohmann::json::object();
    generator["name"] = templateName;
    generator["interfaces"] = nlohmann::json::array();
    std::sort(usedTemplates.begin(), usedTemplates.end());
    std::string_view prev;
    for (const auto& used : usedTemplates) {
        if (std::get<0>(used) == prev) {
            continue;
        }
        prev = std::get<0>(used);
        nlohmann::json iArray = nlohmann::json::array();
        iArray.push_back(std::get<0>(used));
        iArray.push_back(std::get<1>(used));
        iArray.push_back(std::get<2>(used));
        generator["interfaces"].push_back(iArray);
    }
    return generator;
}

static void fedPotentialInterfaceList(ConnectionsList& connections, const nlohmann::json& fed)
{
    connections.hasPotentialInterfaces = true;
    const std::string_view federateName = connections.federatesWithPotentialInterfaces.emplace_back(
        fed["attributes"]["name"].get<std::string>());
    const auto& potInterfaces = fed["potential_interfaces"];
    if (potInterfaces.contains("inputs")) {
        for (const auto& input : potInterfaces["inputs"]) {
            if (input.is_object()) {
                auto name = fileops::getName(input);
                if (name.empty()) {
                    continue;
                }
                const std::string_view input1 = connections.interfaces.emplace_back(name);
                connections.potentialInputs.emplace(
                    input1, PotentialConnections{federateName, input1, false});
            } else if (input.is_string()) {
                const std::string_view input1 =
                    connections.interfaces.emplace_back(input.get<std::string>());
                connections.potentialInputs.emplace(
                    input1, PotentialConnections{federateName, input1, false});
            }
        }
    }
    if (potInterfaces.contains("publications")) {
        for (const auto& pub : potInterfaces["publications"]) {
            if (pub.is_object()) {
                auto name = fileops::getName(pub);
                if (name.empty()) {
                    continue;
                }
                const std::string_view pub1 = connections.interfaces.emplace_back(name);
                connections.potentialPubs.emplace(pub1,
                                                  PotentialConnections{federateName, pub1, false});
            } else if (pub.is_string()) {
                const std::string_view pub1 =
                    connections.interfaces.emplace_back(pub.get<std::string>());
                connections.potentialPubs.emplace(pub1,
                                                  PotentialConnections{federateName, pub1, false});
            }
        }
    }
    if (potInterfaces.contains("endpoints")) {
        for (const auto& endpoint : potInterfaces["endpoints"]) {
            if (endpoint.is_object()) {
                auto name = fileops::getName(endpoint);
                if (name.empty()) {
                    continue;
                }
                const std::string_view endpoint1 = connections.interfaces.emplace_back(name);
                connections.potentialEndpoints.emplace(
                    endpoint1, PotentialConnections{federateName, endpoint1, false});
            } else {
                const std::string_view endpoint1 =
                    connections.interfaces.emplace_back(endpoint.get<std::string>());
                connections.potentialEndpoints.emplace(
                    endpoint1, PotentialConnections{federateName, endpoint1, false});
            }
        }
    }
    if (potInterfaces.contains("publication_templates")) {
        for (const auto& pubTemplate : potInterfaces["publication_templates"]) {
            if (pubTemplate.is_object()) {
                TemplateMatcher temp;
                temp.federate = federateName;
                if (temp.loadTemplate(pubTemplate)) {
                    connections.potentialPublicationTemplates.push_back(std::move(temp));
                }
            }
        }
    }
    if (potInterfaces.contains("input_templates")) {
        for (const auto& inpTemplate : potInterfaces["input_templates"]) {
            if (inpTemplate.is_object()) {
                TemplateMatcher temp;
                temp.federate = federateName;
                if (temp.loadTemplate(inpTemplate)) {
                    connections.potentialInputTemplates.push_back(std::move(temp));
                }
            }
        }
    }
    if (potInterfaces.contains("endpoint_templates")) {
        for (const auto& endTemplate : potInterfaces["endpoint_templates"]) {
            if (endTemplate.is_object()) {
                TemplateMatcher temp;
                temp.federate = federateName;
                if (temp.loadTemplate(endTemplate)) {
                    connections.potentialEndpointTemplates.push_back(std::move(temp));
                }
            }
        }
    }
}

static void fedConnectionList(ConnectionsList& connections, const nlohmann::json& fed)
{
    try {
        if (fed.contains("tags")) {
            loadTags(connections, fed["tags"]);
        }
        if (fed.contains("connected_inputs")) {
            for (const auto& input : fed["connected_inputs"]) {
                const std::string_view input1 =
                    connections.interfaces.emplace_back(input.get<std::string>());
                connections.inputs.insert(input1);
            }
        }
        if (fed.contains("connected_publications")) {
            for (const auto& pub : fed["connected_publications"]) {
                const std::string_view pub1 =
                    connections.interfaces.emplace_back(pub.get<std::string>());
                connections.pubs.insert(pub1);
            }
        }
        if (fed.contains("unconnected_inputs")) {
            for (const auto& input : fed["unconnected_inputs"]) {
                const std::string_view input1 =
                    connections.interfaces.emplace_back(input.get<std::string>());
                connections.unconnectedInputs.push_back(input1);
                connections.inputs.insert(input1);
            }
        }
        if (fed.contains("unconnected_publications")) {
            for (const auto& pub : fed["unconnected_publications"]) {
                const std::string_view pub1 =
                    connections.interfaces.emplace_back(pub.get<std::string>());
                connections.unconnectedPubs.push_back(pub1);
                connections.pubs.insert(pub1);
            }
        }

        if (fed.contains("unconnected_target_endpoints")) {
            for (const auto& endpoint : fed["unconnected_target_endpoints"]) {
                const std::string_view end1 =
                    connections.interfaces.emplace_back(endpoint.get<std::string>());
                connections.unconnectedTargetEndpoints.push_back(end1);
                connections.endpoints.insert(end1);
            }
        }
        if (fed.contains("unconnected_source_endpoints")) {
            for (const auto& endpoint : fed["unconnected_source_endpoints"]) {
                const std::string_view end1 =
                    connections.interfaces.emplace_back(endpoint.get<std::string>());
                connections.unconnectedSourceEndpoints.push_back(end1);
                connections.endpoints.insert(end1);
            }
        }
        if (fed.contains("connected_endpoints")) {
            for (const auto& endpoint : fed["connected_endpoints"]) {
                const std::string_view end1 =
                    connections.interfaces.emplace_back(endpoint.get<std::string>());
                connections.endpoints.insert(end1);
            }
        }
        if (fed.contains("potential_interfaces")) {
            fedPotentialInterfaceList(connections, fed);
        }
    }
    catch (const nlohmann::json::exception& /*ev*/) {
        // TODO(PT): I think this is going to be almost impossible now, but someday might
        // want to create a response
        return;
    }
}
static void coreConnectionList(ConnectionsList& connections, const nlohmann::json& core)
{
    if (core.contains("tags")) {
        loadTags(connections, core["tags"]);
    }
    if (core.contains("federates")) {
        for (const auto& fed : core["federates"]) {
            fedConnectionList(connections, fed);
        }
    }
}

static void brokerConnectionList(ConnectionsList& connections, nlohmann::json& broker)
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
    if (json.contains("aliases")) {
        for (auto& alias : json["aliases"]) {
            const std::string_view alias1 =
                connections.interfaces.emplace_back(alias[0].get<std::string>());
            const std::string_view alias2 =
                connections.interfaces.emplace_back(alias[1].get<std::string>());
            connections.aliases.emplace(alias1, alias2);
        }
    }
    if (json.contains("tags")) {
        loadTags(connections, json["tags"]);
    }
    if (json.contains("unknown_inputs")) {
        for (const auto& input : json["unknown_inputs"]) {
            connections.unknownInputs.push_back(input.get<std::string>());
        }
    }
    if (json.contains("unknown_publications")) {
        for (const auto& pub : json["unknown_publications"]) {
            connections.unknownPubs.push_back(pub.get<std::string>());
        }
    }
    if (json.contains("unknown_endpoints")) {
        for (const auto& ept : json["unknown_endpoints"]) {
            connections.unknownEndpoints.push_back(ept.get<std::string>());
        }
    }
    for (auto& broker : json["brokers"]) {
        brokerConnectionList(connections, broker);
    }
    for (auto& core : json["cores"]) {
        coreConnectionList(connections, core);
    }
    connections.tagStrings.emplace_back("default");
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
                     const std::shared_ptr<Core>& coreObj,
                     const FederateInfo& fedInfo):
    App(appName, coreObj, fedInfo), core((fed) ? fed->getCorePointer() : nullptr)
{
    initialSetup();
}

Connector::Connector(std::string_view appName, CoreApp& coreObj, const FederateInfo& fedInfo):
    App(appName, coreObj, fedInfo), core((fed) ? fed->getCorePointer() : nullptr)
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
    const std::size_t hash = std::hash<std::string_view>()(tagName);
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

    for (std::size_t ii = 3; ii < connection.size(); ++ii) {
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
    Connection conn{iview1, iview2, direction, std::move(svtags), nullptr};
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

    if (doc.contains("connector")) {
        const auto& connectorConfig = doc["connector"];
        matchTargetEndpoints =
            fileops::getOrDefault(connectorConfig, "match_target_endpoints", matchTargetEndpoints);
        matchMultiple = fileops::getOrDefault(connectorConfig, "match_multiple", matchMultiple);
        alwaysCheckRegex =
            fileops::getOrDefault(connectorConfig, "always_check_regex", alwaysCheckRegex);
    }
    auto connectionArray = doc["connections"];
    if (connectionArray.is_array()) {
        connections.reserve(connections.size() + connectionArray.size());
        for (const auto& connectionElement : connectionArray) {
            std::vector<std::string> connectionObject;
            for (const auto& subElement : connectionElement) {
                connectionObject.push_back(subElement.get<std::string>());
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

bool Connector::makePotentialTemplateConnection(
    std::string_view interface,
    const std::vector<std::size_t>& tagList,
    std::vector<TemplateMatcher>& potentialTemplates,
    const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    auto connectionOptions = buildPossibleConnectionList(interface, tagList);
    for (const auto& option : connectionOptions) {
        for (auto& matcher : potentialTemplates) {
            auto match = matcher.isTemplateMatch(option.interface2);
            if (match) {
                matcher.setAsUsed(*match);
                return true;
            }
        }
        auto aliasList = generateAliases(option.interface2, aliases);
        for (const auto& alias : aliasList) {
            for (auto& matcher : potentialTemplates) {
                auto match = matcher.isTemplateMatch(alias);
                if (match) {
                    matcher.setAsUsed(*match);
                    return true;
                }
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
    std::vector<TemplateMatcher>& potentialTemplates,
    const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    static auto nullConnector = [](std::string_view, std::string_view) {};
    /** potential inputs*/
    auto matched =
        makeTargetConnection(interfaceName, tagList, possibleConnections, aliases, nullConnector);
    if (matched > 0) {
        return true;
    }
    if (makePotentialConnection(interfaceName, tagList, potentials, aliases)) {
        return true;
    }
    if (!potentialTemplates.empty()) {
        if (makePotentialTemplateConnection(interfaceName, tagList, potentialTemplates, aliases)) {
            return true;
        }
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
            if (!potentialTemplates.empty()) {
                if (makePotentialTemplateConnection(alias, tagList, potentialTemplates, aliases)) {
                    return true;
                }
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
    const int logLevel = fed->getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL);
    auto inputConnector = [this, logLevel](std::string_view target, std::string_view source) {
        core.dataLink(source, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting input {} to publication {}", target, source));
        }
    };
    auto pubConnector = [this, logLevel](std::string_view source, std::string_view target) {
        core.dataLink(source, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting publication {} to input {}", source, target));
        }
    };
    auto sourceEndpointConnector = [this, logLevel](std::string_view source,
                                                    std::string_view target) {
        core.linkEndpoints(source, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting source endpoint {} to target endpoint {}",
                                        source,
                                        target));
        }
    };
    auto targetEndpointConnector = [this, logLevel](std::string_view target,
                                                    std::string_view source) {
        core.linkEndpoints(source, target);
        if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
            fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                            fmt::format("connecting target endpoint {} to source endpoint {}",
                                        target,
                                        source));
        }
    };

    const auto& tagList = possibleConnections.tagCodes;
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

void Connector::scanPotentialInterfaces(ConnectionsList& possibleConnections)
{
    /** potential inputs*/
    for (auto& pInp : possibleConnections.potentialInputs) {
        if (checkPotentialConnection(pInp.first,
                                     possibleConnections.tagCodes,
                                     possibleConnections.pubs,
                                     possibleConnections.potentialPubs,
                                     possibleConnections.potentialPublicationTemplates,
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
                                     possibleConnections.tagCodes,
                                     possibleConnections.inputs,
                                     possibleConnections.potentialInputs,
                                     possibleConnections.potentialInputTemplates,
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
                                     possibleConnections.tagCodes,
                                     possibleConnections.endpoints,
                                     possibleConnections.potentialEndpoints,
                                     possibleConnections.potentialEndpointTemplates,
                                     possibleConnections.aliases)) {
            pEnd.second.used = true;
        }
    }
}

void Connector::scanPotentialInterfaceTemplates(ConnectionsList& possibleConnections)
{
    /** now run through the potential interface templates as if they were directly listed*/
    for (auto& inpTemplate : possibleConnections.potentialInputTemplates) {
        for (std::size_t ii = 0; ii < inpTemplate.possibilitiesCount(); ++ii) {
            const std::string possibility = inpTemplate.instantiateTemplate(ii);
            if (checkPotentialConnection(possibility,
                                         possibleConnections.tagCodes,
                                         possibleConnections.pubs,
                                         possibleConnections.potentialPubs,
                                         possibleConnections.potentialPublicationTemplates,
                                         possibleConnections.aliases)) {
                auto check = inpTemplate.isTemplateMatch(possibility);
                if (check) {
                    inpTemplate.setAsUsed(*check);
                }
            }
        }
    }
    for (auto& pubTemplate : possibleConnections.potentialPublicationTemplates) {
        for (std::size_t ii = 0; ii < pubTemplate.possibilitiesCount(); ++ii) {
            const std::string possibility = pubTemplate.instantiateTemplate(ii);
            if (checkPotentialConnection(possibility,
                                         possibleConnections.tagCodes,
                                         possibleConnections.inputs,
                                         possibleConnections.potentialInputs,
                                         possibleConnections.potentialInputTemplates,
                                         possibleConnections.aliases)) {
                auto check = pubTemplate.isTemplateMatch(possibility);
                if (check) {
                    pubTemplate.setAsUsed(*check);
                }
            }
        }
    }
    for (auto& endTemplate : possibleConnections.potentialEndpointTemplates) {
        for (std::size_t ii = 0; ii < endTemplate.possibilitiesCount(); ++ii) {
            const std::string possibility = endTemplate.instantiateTemplate(ii);
            if (checkPotentialConnection(possibility,
                                         possibleConnections.tagCodes,
                                         possibleConnections.endpoints,
                                         possibleConnections.potentialEndpoints,
                                         possibleConnections.potentialEndpointTemplates,
                                         possibleConnections.aliases)) {
                auto check = endTemplate.isTemplateMatch(possibility);
                if (check) {
                    endTemplate.setAsUsed(*check);
                }
            }
        }
    }
}

void Connector::scanUnconnectedInterfaces(ConnectionsList& possibleConnections)
{
    /** unconnected inputs*/
    for (const auto& uInp : possibleConnections.unconnectedInputs) {
        if (makePotentialConnection(uInp,
                                    possibleConnections.tagCodes,
                                    possibleConnections.potentialPubs,
                                    possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uInp, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uInp) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            possibleConnections.tagCodes,
                                            possibleConnections.potentialPubs,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }

    /** unconnected publications*/
    for (const auto& uPub : possibleConnections.unconnectedPubs) {
        if (makePotentialConnection(uPub,
                                    possibleConnections.tagCodes,
                                    possibleConnections.potentialInputs,
                                    possibleConnections.aliases)) {
            continue;
        }
        if (!possibleConnections.aliases.empty()) {
            auto aliasList = generateAliases(uPub, possibleConnections.aliases);
            for (const auto& alias : aliasList) {
                if (alias == uPub) {
                    continue;
                }
                if (makePotentialConnection(alias,
                                            possibleConnections.tagCodes,
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
                                    possibleConnections.tagCodes,
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
                                            possibleConnections.tagCodes,
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
                                    possibleConnections.tagCodes,
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
                                            possibleConnections.tagCodes,
                                            possibleConnections.potentialEndpoints,
                                            possibleConnections.aliases)) {
                    break;
                }
            }
        }
    }
}

static void scanUnknownInterfaces(ConnectionsList& possibleConnections)
{
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
}

static int addUsedPotentialInterfaceToCommand(
    nlohmann::json& potentialCommand,
    const std::unordered_map<std::string_view, PotentialConnections>& potentials,
    const std::string& possibleFed,
    int logLevel,
    const std::string& type,
    Federate* fed)
{
    int interfaceCount{0};
    std::vector<std::remove_cv_t<std::remove_reference_t<decltype(*potentials.begin())>>>
        enabledInterfaces;
    std::copy_if(potentials.begin(),
                 potentials.end(),
                 std::back_inserter(enabledInterfaces),
                 [possibleFed](auto& pInterface) {
                     return (pInterface.second.federate == possibleFed &&
                             pInterface.second.used == true);
                 });
    if (!enabledInterfaces.empty()) {
        potentialCommand[type] = nlohmann::json::array();
        for (const auto& iface : enabledInterfaces) {
            potentialCommand[type].push_back(std::string(iface.first));
            ++interfaceCount;
            if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
                fed->logMessage(
                    HELICS_LOG_LEVEL_CONNECTIONS,
                    fmt::format("federate {} request {} {}", possibleFed, type, iface.first));
            }
        }
    }
    return interfaceCount;
}

static int addUsedPotentialInterfaceTemplates(nlohmann::json& potentialCommand,
                                              std::vector<TemplateMatcher>& potentials,
                                              const std::string& possibleFed,
                                              int logLevel,
                                              const std::string& type,
                                              Federate* fed)
{
    bool hasInterfaceTemplates{false};
    for (auto& ifaceTemplate : potentials) {
        if (ifaceTemplate.federate != possibleFed) {
            continue;
        }
        if (!ifaceTemplate.isUsed()) {
            continue;
        }
        hasInterfaceTemplates = true;
        break;
    }
    if (hasInterfaceTemplates) {
        potentialCommand[type] = nlohmann::json::array();
        for (auto& ifaceTemplate : potentials) {
            if (ifaceTemplate.federate != possibleFed) {
                continue;
            }
            if (!ifaceTemplate.isUsed()) {
                continue;
            }
            potentialCommand[type].push_back(ifaceTemplate.usedInterfaceGeneration());
            if (logLevel >= HELICS_LOG_LEVEL_CONNECTIONS) {
                fed->logMessage(HELICS_LOG_LEVEL_CONNECTIONS,
                                fmt::format("federate {} request {} {}",
                                            possibleFed,
                                            type,
                                            fileops::generateJsonString(
                                                ifaceTemplate.usedInterfaceGeneration())));
            }
        }
    }
    return 0;
}

void Connector::establishPotentialInterfaces(ConnectionsList& possibleConnections)
{
    scanPotentialInterfaces(possibleConnections);
    scanPotentialInterfaceTemplates(possibleConnections);

    /** now try to match unconnected interfaces to some of the potential ones*/
    scanUnconnectedInterfaces(possibleConnections);
    /** check for unknown interface connections to potential interfaces*/
    scanUnknownInterfaces(possibleConnections);

    const int logLevel = fed->getIntegerProperty(HELICS_PROPERTY_INT_LOG_LEVEL);
    for (auto& possibleFed : possibleConnections.federatesWithPotentialInterfaces) {
        nlohmann::json establishInterfaces;
        establishInterfaces["command"] = "register_interfaces";
        addUsedPotentialInterfaceToCommand(establishInterfaces,
                                           possibleConnections.potentialInputs,
                                           possibleFed,
                                           logLevel,
                                           "inputs",
                                           fed.get());
        addUsedPotentialInterfaceToCommand(establishInterfaces,
                                           possibleConnections.potentialPubs,
                                           possibleFed,
                                           logLevel,
                                           "publications",
                                           fed.get());
        addUsedPotentialInterfaceToCommand(establishInterfaces,
                                           possibleConnections.potentialEndpoints,
                                           possibleFed,
                                           logLevel,
                                           "endpoints",
                                           fed.get());

        addUsedPotentialInterfaceTemplates(establishInterfaces,
                                           possibleConnections.potentialPublicationTemplates,
                                           possibleFed,
                                           logLevel,
                                           "templated_publications",
                                           fed.get());
        addUsedPotentialInterfaceTemplates(establishInterfaces,
                                           possibleConnections.potentialInputTemplates,
                                           possibleFed,
                                           logLevel,
                                           "templated_inputs",
                                           fed.get());
        addUsedPotentialInterfaceTemplates(establishInterfaces,
                                           possibleConnections.potentialEndpointTemplates,
                                           possibleFed,
                                           logLevel,
                                           "templated_endpoints",
                                           fed.get());
        const std::string commandStr = fileops::generateJsonString(establishInterfaces);
        fed->sendCommand(possibleFed, commandStr);
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
