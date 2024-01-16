/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Connector.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <optional>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <deque>



namespace helics::apps {

    /** data class for information from queries*/
    class ConnectionsList
    {
    public:
        std::unordered_multimap<std::string_view,std::string_view> aliases;
        std::vector<std::string_view> unconnectedPubs;
        std::vector<std::string_view> unconnectedInputs;
        std::vector<std::string_view> unconnectedTargetEndpoints;
        std::vector<std::string_view> unconnectedSourceEndpoints;
        std::unordered_set<std::string_view> pubs;
        std::unordered_set<std::string_view> inputs;
        std::unordered_set<std::string_view> endpoints;
        std::deque<std::string> interfaces;
    };

    ConnectionsList generateConnectionsList(const std::string& connectionData)
    {
        ConnectionsList connections;
        auto json=fileops::loadJsonStr(connectionData);
        if (json.isMember("aliases"))
        {
            for (auto& alias : json["aliases"])
            {
                std::string_view alias1=connections.interfaces.emplace_back(alias[0].asString());
                std::string_view alias2=connections.interfaces.emplace_back(alias[1].asString());
                connections.aliases.emplace(alias1, alias2);
            }
        }
        for (auto& core : json["cores"])
        {
            if (core.isMember("federates"))
            {
                for (auto& fed : core["federates"])
                {
                    if (fed.isMember("connected_inputs"))
                    {
                        for (auto& input : fed["connected_inputs"])
                        {
                            std::string_view input1=connections.interfaces.emplace_back(input.asString());
                            connections.inputs.insert(input1);
                        }
                    }
                    if (fed.isMember("connected_publications"))
                    {
                        for (auto& pub : fed["connected_publications"])
                        {
                            std::string_view pub1=connections.interfaces.emplace_back(pub.asString());
                            connections.pubs.insert(pub1);
                        }
                    }
                    if (fed.isMember("unconnected_inputs"))
                    {
                        for (auto& input : fed["unconnected_inputs"])
                        {
                            std::string_view input1=connections.interfaces.emplace_back(input.asString());
                            connections.unconnectedInputs.push_back(input1);
                            connections.inputs.insert(input1);
                        }
                    }
                    if (fed.isMember("unconnected_publications"))
                    {
                        for (auto& pub : fed["unconnected_publications"])
                        {
                            std::string_view pub1=connections.interfaces.emplace_back(pub.asString());
                            connections.unconnectedPubs.push_back(pub1);
                            connections.pubs.insert(pub1);
                        }
                    }

                    if (fed.isMember("unconnected_target_endpoints"))
                    {
                        for (auto& endpoint : fed["unconnected_target_endpoints"])
                        {
                            std::string_view end1=connections.interfaces.emplace_back(endpoint.asString());
                            connections.unconnectedTargetEndpoints.push_back(end1);
                            connections.endpoints.insert(end1);
                        }
                    }
                    if (fed.isMember("unconnected_source_endpoints"))
                    {
                        for (auto& endpoint : fed["unconnected_source_endpoints"])
                        {
                            std::string_view end1=connections.interfaces.emplace_back(endpoint.asString());
                            connections.unconnectedSourceEndpoints.push_back(end1);
                            connections.endpoints.insert(end1);
                        }
                    }
                    if (fed.isMember("connected_endpoints"))
                    {
                        for (auto& endpoint : fed["connected_endpoints"])
                        {
                            std::string_view end1=connections.interfaces.emplace_back(endpoint.asString());
                            connections.endpoints.insert(end1);
                        }
                    }
                }
            }
        }
        return connections;
    }

Connector::Connector(std::vector<std::string> args): App("connector", std::move(args)),core(fed->getCorePointer())
{
    processArgs();
}

Connector::Connector(int argc, char* argv[]): App("connector", argc, argv),core(fed->getCorePointer())
{
    processArgs();
}

void Connector::processArgs()
{
    auto app = generateParser();

    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
        app->helics_parse(remArgs);
        if (!masterFileName.empty()) {
            loadFile(masterFileName);
        }
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}

const std::unordered_map<std::string_view,InterfaceDirection> directionNames
{{"from_to",InterfaceDirection::FROM_TO},
    {"1",InterfaceDirection::FROM_TO},
    {"FROM_TO",InterfaceDirection::FROM_TO},
    {"to_from",InterfaceDirection::TO_FROM},
    {"TO_FROM",InterfaceDirection::TO_FROM},
    {"-1",InterfaceDirection::TO_FROM},
    {"bidirectional",InterfaceDirection::BIDIRECTIONAL},
    {"BIDIRECTIONAL",InterfaceDirection::BIDIRECTIONAL},
    {"0",InterfaceDirection::BIDIRECTIONAL},
    {"bi",InterfaceDirection::BIDIRECTIONAL},
    {"BI",InterfaceDirection::BIDIRECTIONAL}
};

std::optional<InterfaceDirection> getDirection(std::string_view direction)
{
    auto res=directionNames.find(direction);
    if (res != directionNames.end())
    {
        return res->second;
    }
    return std::nullopt;
}

std::unique_ptr<helicsCLI11App> Connector::generateParser()
{
    auto app = std::make_unique<helicsCLI11App>("Command line options for the Connector App");
    app->add_option_function<std::vector<std::vector<std::string>>>(
        "--connection",
        [this](const std::vector<std::vector<std::string>> &args) {
            for (auto& conn : args)
            {
                addConnectionVector(conn);
            }
           
        },
        "specify connections to make in the cosimulation")->expected(2,CLI::detail::expected_max_vector_size)->type_name("[INTERFACE1,INTERFACE2,DIRECTIONALITY,TXT...]");

    return app;
}

Connector::Connector(std::string_view appName, const FederateInfo& fi): App(appName, fi),core(fed->getCorePointer())
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, const std::shared_ptr<Core>& core, const FederateInfo& fi):
    App(appName, core, fi),core(fed->getCorePointer())
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, CoreApp& core, const FederateInfo& fi):
    App(appName, core, fi),core(fed->getCorePointer())
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, const std::string& configString):
    App(appName, configString),core(fed->getCorePointer())
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
    Connector::loadJsonFile(configString);
}

std::string_view Connector::addTag(std::string_view tagName)
{
    auto it=tags.insert(std::string(tagName));
    return std::string_view(*(it.first));
}

std::string_view Connector::addInterface(std::string_view interfaceName)
{
    auto it=interfaces.insert(std::string(interfaceName));
    return std::string_view(*(it.first));
}

bool Connector::addConnectionVector(const std::vector<std::string>& v1)
{
    if (v1.size() <= 1)
    {
        return false;
    }
    if (v1.size() == 2)
    {
        addConnection(v1[0],v1[1]);
        return true;
    }

        InterfaceDirection direction{ InterfaceDirection::BIDIRECTIONAL };
        std::vector<std::string> newTags;
        auto d = getDirection(v1[2]);
        if (d)
        {
            direction=*d;
        }
        else {
            newTags.push_back(v1[2]);
        }

        for (int ii = 3; ii < v1.size(); ++ii)
        {
            newTags.push_back(v1[ii]);
        }
        addConnection(v1[0],v1[1],direction,newTags);
    return true;
}

void Connector::addConnection(std::string_view interface1,
    std::string_view interface2,
    InterfaceDirection direction,
    std::vector<std::string> connectionTags)
{
    std::vector<std::string_view> svtags;
    svtags.reserve(connectionTags.size());
    for (const auto& t1 : connectionTags)
    {
        svtags.push_back(addTag(t1));
    }
    auto iview1=addInterface(interface1);
    auto iview2=addInterface(interface2);
    Connection conn{ iview1,iview2,direction,std::move(svtags) };
    switch (direction)
    {
    case InterfaceDirection::TO_FROM:
        connections.emplace(iview2,std::move(conn));
        break;
    case InterfaceDirection::FROM_TO:
        connections.emplace(iview1,std::move(conn));
        break;
    case InterfaceDirection::BIDIRECTIONAL:
        connections.emplace(iview2,conn);
        if (iview1 != iview2)
        {
            connections.emplace(iview1,std::move(conn));
        }
        break;
    }
}


void Connector::loadTextFile(const std::string& filename)
{
    App::loadTextFile(filename);
    using namespace gmlc::utilities::stringOps;  // NOLINT
    std::ifstream infile(filename);
    std::string str;

    int ccnt = 0;
    bool mlineComment = false;
    // count the lines
    while (std::getline(infile, str)) {
        if (str.empty()) {
            continue;
        }
        auto fc = str.find_first_not_of(" \t\n\r\0");
        if (fc == std::string::npos) {
            continue;
        }
        if (mlineComment) {
            if (fc + 2 < str.size()) {
                if ((str[fc] == '#') && (str[fc + 1] == '#') && (str[fc + 2] == ']')) {
                    mlineComment = false;
                }
            }
            continue;
        }
        if (str[fc] == '#') {
            if (fc + 2 < str.size()) {
                if ((str[fc + 1] == '#') && (str[fc + 2] == '[')) {
                    mlineComment = true;
                }
            }
            continue;
        }
        if ((str[fc] == 'm') || (str[fc] == 'M')) {
            ++ccnt;
        }
    }
    connections.reserve(connections.size()+ccnt);
    // now start over and actual do the loading
    infile.close();
    infile.open(filename);
    while (std::getline(infile, str)) {
        if (str.empty()) {
            continue;
        }
        auto fc = str.find_first_not_of(" \t\n\r\0");
        if (fc == std::string::npos) {
            continue;
        }
        if (mlineComment) {
            if (fc + 2 < str.size()) {
                if ((str[fc] == '#') && (str[fc + 1] == '#') && (str[fc + 2] == ']')) {
                    mlineComment = false;
                }
            }
            continue;
        }
        if (str[fc] == '#') {
            if (fc + 2 < str.size()) {
                if ((str[fc + 1] == '#') && (str[fc + 2] == '[')) {
                    mlineComment = true;
                } else if (str[fc + 1] == '!') {
                    /*  //allow configuration inside the regular text file
                    */
                }
            }
            continue;
        }
        /* time key type value units*/
        auto blk = splitlineBracket(str, ",\t ", default_bracket_chars, delimiter_compression::on);
        addConnectionVector(blk);

    }
}

void Connector::loadJsonFile(const std::string& jsonString)
{
    loadJsonFileConfiguration("connector", jsonString);

    auto doc = fileops::loadJson(jsonString);

    if (doc.isMember("connector")) {
        auto playerConfig = doc["connector"];
        
    }
    auto connectionArray = doc["connections"];
    if (connectionArray.isArray()) {
        connections.reserve(connections.size() + connectionArray.size());
        for (const auto& connectionElement : connectionArray) {
            std::vector<std::string> connectionObject;
            for (const auto& subElement : connectionElement)
            {
                connectionObject.push_back(subElement.asString());
            }
            addConnectionVector(connectionObject);
        }
    }
}

std::vector<Connection> Connector::buildPossibleConnectionList(std::string_view startingInterface) const
{
    std::vector<Connection> matches;
    auto [first, last] = connections.equal_range(startingInterface);
    if (first == connections.end())
    {
        return matches;
    }
    std::set<std::string_view> searched;
    searched.insert(startingInterface);

    for (auto match = first; match != last; ++match)
    {
        matches.emplace_back(match->second);
        if (matches.back().interface1 != startingInterface)
        {
            std::swap(matches.back().interface1,matches.back().interface2);
        }
    }
    std::size_t cascadeIndex{0};
    while (cascadeIndex < matches.size())
    {
        if (searched.find(matches[cascadeIndex].interface2) != searched.end())
        {
            ++cascadeIndex;
            continue;
        }
        searched.insert(matches[cascadeIndex].interface2);
        std::tie(first, last) = connections.equal_range(matches[cascadeIndex].interface2);
        if (first != connections.end())
        {
            for (auto match = first; match != last; ++match)
            {
                matches.emplace_back(match->second);
                if (matches.back().interface1 != matches[cascadeIndex].interface2)
                {
                    std::swap(matches.back().interface1,matches.back().interface2);
                }
                if (searched.find(matches.back().interface2) != searched.end())
                {
                    //this would already be references and create a cyclic reference
                    matches.pop_back();
                }
            }
        }
        ++cascadeIndex;
    }
    return matches;
}

static std::set<std::string_view> generateAliases(const std::string_view target, const std::unordered_multimap<std::string_view, std::string_view>& aliases)
{
    std::set<std::string_view> matches;
    auto [first, last] = aliases.equal_range(target);
    if (first == aliases.end())
    {
        return matches;
    }
    matches.emplace(target);
    std::vector<std::string_view> matchList;
    for (auto match = first; match != last; ++match)
    {
        matches.emplace(match->second);
        matchList.emplace_back(match->second);
    }
    std::size_t cascadeIndex{1};
    while (cascadeIndex < matchList.size())
    {
        std::tie(first, last) = aliases.equal_range(matchList[cascadeIndex]);
            for (auto match = first; match != last; ++match)
            {
                auto [iterator,newAlias]=matches.emplace(match->second);
                if (newAlias)
                {
                    matchList.emplace_back(match->second);
                }
            }
        ++cascadeIndex;
    }
    return matches;
}

int Connector::makeTargetConnection(std::string_view origin, std::unordered_set<std::string_view>& possibleConnections, const std::unordered_multimap<std::string_view, std::string_view>& aliases,const std::function<void(std::string_view,std::string_view)> &callback)
{
    int matched{0};
    auto connectionOptions=buildPossibleConnectionList(origin);
    for (auto& option : connectionOptions)
    {
        auto located=possibleConnections.find(option.interface2);
        if (located != possibleConnections.end())
        {
            /* source, target*/
            callback(origin,option.interface2);
            ++matched;
            if (!matchMultiple)
            {
                return matched;
            }

        }
        else
        {
            if (!aliases.empty())
            {
                auto aliasList = generateAliases(option.interface2, aliases);
                for (auto& alias : aliasList)
                {
                    located=possibleConnections.find(alias);
                    if (located != possibleConnections.end())
                    {
                        callback(origin,option.interface2);
                        ++matched;
                        if (!matchMultiple)
                        {
                            return matched;
                        }
                        break;
                    }
                }
            }
        }
    }
    if (!aliases.empty())
    {
        auto aliasList = generateAliases(origin, aliases);
        for (auto& alias : aliasList)
        {
            if (alias == origin)
            {
                continue;
            }
            auto aliasOptions = buildPossibleConnectionList(alias);
            for (auto& option : aliasOptions)
            {
                auto located = possibleConnections.find(option.interface2);
                if (located != possibleConnections.end())
                {
                    /* source, target*/
                    callback(origin,option.interface2);
                    ++matched;
                    if (!matchMultiple)
                    {
                        return matched;
                    }

                }
                else
                {
                    if (!aliases.empty())
                    {
                        auto interfaceAliasList = generateAliases(option.interface2, aliases);
                        for (auto& interfaceAlias : interfaceAliasList)
                        {
                            located=possibleConnections.find(interfaceAlias);
                            if (located != possibleConnections.end())
                            {
                                callback(origin,option.interface2);
                                ++matched;
                                if (!matchMultiple)
                                {
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
    auto inputConnector=[this](std::string_view origin, std::string_view target) {core.dataLink(target, origin); };
    auto pubConnector=[this](std::string_view origin, std::string_view target) {core.dataLink(origin,target); };
    auto sourceEndpointConnector=[this](std::string_view origin, std::string_view target) {core.linkEndpoints(origin,target); };
    auto targetEndpointConnector=[this](std::string_view origin, std::string_view target) {core.linkEndpoints(target,origin); };
    /** unconnected inputs*/
    for (const auto& uInp : possibleConnections.unconnectedInputs)
    {
        matchCount += makeTargetConnection(uInp, possibleConnections.pubs, possibleConnections.aliases, inputConnector);
    }
    /** unconnected publications*/
    for (const auto& uPub : possibleConnections.unconnectedPubs)
    {
        matchCount+= makeTargetConnection(uPub, possibleConnections.inputs, possibleConnections.aliases, pubConnector);
    }

    /** unconnected source endpoints*/
    for (const auto& uEnd : possibleConnections.unconnectedSourceEndpoints)
    {
        matchCount+= makeTargetConnection(uEnd, possibleConnections.endpoints, possibleConnections.aliases, sourceEndpointConnector);
    }

    if (matchTargetEndpoints)
    {
        /** unconnected target endpoints*/
        for (const auto& uEnd : possibleConnections.unconnectedTargetEndpoints)
        {
            matchCount+= makeTargetConnection(uEnd, possibleConnections.endpoints, possibleConnections.aliases, targetEndpointConnector);
        }
    }
    
}

void Connector::initialize()
{
    auto md = fed->getCurrentMode();
    if (md == Federate::Modes::STARTUP) {
        fed->enterInitializingModeIterative();

        auto connectionsData=generateConnectionsList(fed->query("root","unconnected_interfaces"));
        makeConnections(connectionsData);
        fed->enterInitializingMode();
    }
}


void Connector::runTo([[maybe_unused]] Time stopTime_input)
{
    auto md = fed->getCurrentMode();
    if (md == Federate::Modes::STARTUP) {
        initialize();
    }
    if (md < Federate::Modes::EXECUTING) {
        
        fed->enterExecutingMode();
    } else {
        fed->disconnect();
    }
}



}  // namespace helics::apps
