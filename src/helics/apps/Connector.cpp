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



namespace helics::apps {


Connector::Connector(std::vector<std::string> args): App("connector_${#}", std::move(args))
{
    processArgs();
}

Connector::Connector(int argc, char* argv[]): App("connector_${#}", argc, argv)
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
    auto *opt=app->add_option(
        "--connection",
        [this](CLI::results_t res) {
           addConnectionVector(res);
        },
        "specify connections to make in the cosimulation")->expected(2,CLI::detail::expected_max_vector_size)->type_name("[INTERFACE1,INTERFACE2,DIRECTIONALITY,TXT...]");

    return app;
}

Connector::Connector(std::string_view appName, const FederateInfo& fi): App(appName, fi)
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, const std::shared_ptr<Core>& core, const FederateInfo& fi):
    App(appName, core, fi)
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, CoreApp& core, const FederateInfo& fi):
    App(appName, core, fi)
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
}

Connector::Connector(std::string_view appName, const std::string& configString):
    App(appName, configString)
{
    fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
    Connector::loadJsonFile(configString);
}

std::string_view Connector::addTag(const std::string &tagName)
{
    auto it=tags.insert(std::string(tagName));
    return std::string_view(*(it.first));
}

bool Connector::addConnectionVector(const std::vector<std::string>& v1)
{
    if (v1.size() == 2)
    {
        addConnection(v1[0],v1[1]);
    }
    else
    {
        InterfaceDirection direction{ InterfaceDirection::BIDIRECTIONAL };
        std::vector<std::string> tags;
        auto d = getDirection(v1[2]);
        if (d)
        {
            direction=*d;
        }
        else {
            tags.push_back(v1[2]);
        }

        for (int ii = 3; ii < v1.size(); ++ii)
        {
            tags.push_back(v1[ii]);
        }
        addConnection(v1[0],v1[1],direction,tags);
    }
}

void Connector::addConnection(std::string_view interface1,
    std::string_view interface2,
    InterfaceDirection direction,
    std::vector<std::string> tags)
{
    std::vector<std::string_view> svtags;
    svtags.reserve(tags.size());
    for (const auto& t1 : tags)
    {
        svtags.push_back(addTag(t1));
    }
    Connection conn{ std::string(interface1),std::string(interface2),direction,std::move(svtags) };
    switch (direction)
    {
    case InterfaceDirection::TO_FROM:
        connections.emplace(interface1,std::move(conn));
        break;
    case InterfaceDirection::FROM_TO:
        connections.emplace(interface2,std::move(conn));
        break;
    case InterfaceDirection::BIDIRECTIONAL:
        connections.emplace(interface1,conn);
        connections.emplace(interface2,std::move(conn));
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

    int lcount = 0;
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






                if (playerConfig.find("time_units") != playerConfig.end())
                {
                    if (playerConfig["time_units"] == "ns")
                    {
                        timeMultiplier = 1e-9;
                    }
                }
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
            std::string key;
        }
    }
}

void Connector::initialize()
{
    auto md = fed->getCurrentMode();
    if (md == Federate::Modes::STARTUP) {
        fed->enterInitializingModeIterative();
        
    }
}


void Connector::runTo(Time stopTime_input)
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
