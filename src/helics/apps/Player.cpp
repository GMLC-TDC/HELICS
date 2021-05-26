/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Player.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/base64.h"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/timeStringOps.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/** test if a string has a base64 wrapper*/
static int hasB64Wrapper(const std::string& str);
/** function to decode data strings for messages*/
static std::string decode(std::string&& stringToDecode);

// static const std::regex creg
// (R"raw((-?\d+(\.\d+)?|\.\d+)[\s,]*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

/*
std::shared_ptr<CombinationFederate> fed;
std::vector<ValueSetter> points;
std::set<std::pair<std::string, std::string>> tags;
std::vector<Publication> publications;
std::vector<Endpoint> endpoints;
std::map<std::string, int> pubids;
std::map<std::string, int> eptids;
*/

namespace helics {
namespace apps {
    static inline bool vComp(const ValueSetter& v1, const ValueSetter& v2)
    {
        return (v1.time == v2.time) ? (v1.iteration < v2.iteration) : (v1.time < v2.time);
    }
    static inline bool mComp(const MessageHolder& m1, const MessageHolder& m2)
    {
        return (m1.sendTime < m2.sendTime);
    }

    Player::Player(std::vector<std::string> args): App("player", std::move(args)) { processArgs(); }

    Player::Player(int argc, char* argv[]): App("player", argc, argv) { processArgs(); }

    void Player::processArgs()
    {
        auto app = generateParser();

        if (!deactivated) {
            fed->setFlagOption(helics_flag_source_only);
            app->helics_parse(remArgs);
            if (!masterFileName.empty()) {
                loadFile(masterFileName);
            }
        } else if (helpMode) {
            app->remove_helics_specifics();
            std::cout << app->help();
        }
    }

    std::unique_ptr<helicsCLI11App> Player::generateParser()
    {
        auto app = std::make_unique<helicsCLI11App>("Command line options for the Player App");
        app->add_option(
            "--marker",
            nextPrintTimeStep,
            "print a statement indicating time advancement every <arg> period during the simulation");
        app->add_option(
               "--datatype",
               [this](CLI::results_t res) {
                   defType = helics::getTypeFromString(res[0]);
                   return (defType != helics::data_type::helics_custom);
               },
               "type of the publication data type to use",
               false)
            ->take_last()
            ->ignore_underscore();

        app->add_option(
               "--time_units",
               [this](CLI::results_t res) {
                   try {
                       units = gmlc::utilities::timeUnitsFromString(res[0]);
                       timeMultiplier = toSecondMultiplier(units);
                       return true;
                   }
                   catch (...) {
                       return false;
                   }
               },
               "the default units on the timestamps used in file based input",
               false)
            ->take_last()
            ->ignore_underscore();

        return app;
    }

    Player::Player(const std::string& appName, const FederateInfo& fi): App(appName, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Player::Player(const std::string& appName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Player::Player(const std::string& appName, CoreApp& core, const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Player::Player(const std::string& appName, const std::string& configString):
        App(appName, configString)
    {
        fed->setFlagOption(helics_flag_source_only);
        Player::loadJsonFile(configString);
    }

    void Player::addMessage(Time sendTime,
                            const std::string& src,
                            const std::string& dest,
                            const std::string& payload)
    {
        messages.resize(messages.size() + 1);
        messages.back().sendTime = sendTime;
        messages.back().mess.data = payload;
        messages.back().mess.source = src;
        messages.back().mess.dest = dest;
        messages.back().mess.time = sendTime;
    }

    void Player::addMessage(Time sendTime,
                            Time actionTime,
                            const std::string& src,
                            const std::string& dest,
                            const std::string& payload)
    {
        messages.resize(messages.size() + 1);
        messages.back().sendTime = sendTime;
        messages.back().mess.data = payload;
        messages.back().mess.source = src;
        messages.back().mess.dest = dest;
        messages.back().mess.time = actionTime;
    }

    helics::Time Player::extractTime(const std::string& str, int lineNumber) const
    {
        try {
            if (units == time_units::ns)  // ns
            {
                return {std::stoll(str), time_units::ns};
            }
            return loadTimeFromString(str, units);
        }
        catch (const std::invalid_argument&) {
            std::cerr << "ill formed time on line " << lineNumber << '\n';
            return helics::Time::minVal();
        }
    }

    void Player::loadTextFile(const std::string& filename)
    {
        App::loadTextFile(filename);
        using namespace gmlc::utilities::stringOps;  // NOLINT
        std::ifstream infile(filename);
        std::string str;

        int mcnt = 0;
        int pcnt = 0;
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
                ++mcnt;
            } else {
                ++pcnt;
            }
        }
        int pIndex = static_cast<int>(points.size());
        points.resize(points.size() + pcnt);
        int mIndex = static_cast<int>(messages.size());
        messages.resize(messages.size() + mcnt);
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
            auto blk =
                splitlineBracket(str, ",\t ", default_bracket_chars, delimiter_compression::on);

            trimString(blk[0]);
            if ((blk[0].front() == 'm') || (blk[0].front() == 'M')) {
                // deal with messages
                switch (blk.size()) {
                    case 5:
                        if ((messages[mIndex].sendTime = extractTime(blk[1], lcount)) ==
                            Time::minVal()) {
                            continue;
                        }

                        messages[mIndex].mess.source = blk[2];
                        messages[mIndex].mess.dest = blk[3];
                        messages[mIndex].mess.time = messages[mIndex].sendTime;
                        messages[mIndex].mess.data = decode(std::move(blk[4]));
                        break;
                    case 6:
                        if ((messages[mIndex].sendTime = extractTime(blk[1], lcount)) ==
                            Time::minVal()) {
                            continue;
                        }

                        messages[mIndex].mess.source = blk[3];
                        messages[mIndex].mess.dest = blk[4];
                        if ((messages[mIndex].mess.time = extractTime(blk[2], lcount)) ==
                            Time::minVal()) {
                            continue;
                        }
                        messages[mIndex].mess.data = decode(std::move(blk[5]));
                        break;
                    default:
                        std::cerr << "unknown message format line " << lcount << '\n';
                        break;
                }
                ++mIndex;
            } else {
                if (blk.size() == 2) {
                    auto cloc = blk[0].find_last_of(':');
                    if (cloc == std::string::npos) {
                        if ((points[pIndex].time = extractTime(trim(blk[0]), lcount)) ==
                            Time::minVal()) {
                            continue;
                        }
                    } else {
                        if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                               lcount)) == Time::minVal()) {
                            continue;
                        }
                        points[pIndex].iteration = std::stoi(blk[0].substr(cloc + 1));
                    }
                    if (pIndex > 0) {
                        points[pIndex].pubName = points[static_cast<size_t>(pIndex) - 1].pubName;
                    } else {
                        std::cerr
                            << "lines without publication name but follow one with a publication line "
                            << lcount << '\n';
                    }
                    points[pIndex].value = decode(std::move(blk[1]));
                    ++pIndex;
                } else if (blk.size() == 3) {
                    auto cloc = blk[0].find_last_of(':');
                    if (cloc == std::string::npos) {
                        if ((points[pIndex].time = extractTime(trim(blk[0]), lcount)) ==
                            Time::minVal()) {
                            continue;
                        }
                    } else {
                        if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                               lcount)) == Time::minVal()) {
                            continue;
                        }
                        points[pIndex].iteration = std::stoi(blk[0].substr(cloc + 1));
                    }
                    if ((blk[1].empty()) && (pIndex > 0)) {
                        points[pIndex].pubName = points[static_cast<size_t>(pIndex) - 1].pubName;
                    } else {
                        points[pIndex].pubName = blk[1];
                    }

                    points[pIndex].value = decode(std::move(blk[2]));
                    ++pIndex;
                } else if (blk.size() == 4) {
                    auto cloc = blk[0].find_last_of(':');
                    if (cloc == std::string::npos) {
                        if ((points[pIndex].time = extractTime(trim(blk[0]), lcount)) ==
                            Time::minVal()) {
                            continue;
                        }
                    } else {
                        if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                               lcount)) == Time::minVal()) {
                            continue;
                        }
                        points[pIndex].iteration = std::stoi(blk[0].substr(cloc + 1));
                    }
                    if ((blk[1].empty()) && (pIndex > 0)) {
                        points[pIndex].pubName = points[static_cast<size_t>(pIndex) - 1].pubName;
                    } else {
                        points[pIndex].pubName = blk[1];
                    }
                    points[pIndex].type = blk[2];
                    points[pIndex].value = decode(std::move(blk[3]));
                    ++pIndex;
                } else {
                    std::cerr << "unknown publish format line " << lcount << '\n';
                }
            }
        }
    }

    void Player::loadJsonFile(const std::string& jsonString)
    {
        loadJsonFileConfiguration("player", jsonString);

        auto pubCount = fed->getPublicationCount();
        for (int ii = 0; ii < pubCount; ++ii) {
            publications.emplace_back(fed->getPublication(ii));
            pubids[publications.back().getName()] = static_cast<int>(publications.size() - 1);
        }
        auto eptCount = fed->getEndpointCount();
        for (int ii = 0; ii < eptCount; ++ii) {
            endpoints.emplace_back(fed->getEndpoint(ii));
            eptids[endpoints.back().getName()] = static_cast<int>(endpoints.size() - 1);
        }

        auto doc = loadJson(jsonString);

        if (doc.isMember("player")) {
            auto playerConfig = doc["player"];
            if (playerConfig.isMember("time_units")) {
                if (playerConfig["time_units"].asString() == "ns") {
                    timeMultiplier = 1e-9;
                }
            }
        }
        auto pointArray = doc["points"];
        if (pointArray.isArray()) {
            points.reserve(points.size() + pointArray.size());
            for (const auto& pointElement : pointArray) {
                Time ptime;
                int iterationIndex = 0;
                if (pointElement.isMember("time")) {
                    auto str = pointElement["time"].asString();
                    auto cloc = str.find_last_of(':');
                    if (cloc == std::string::npos) {
                        ptime = loadJsonTime(str, units);
                    } else {
                        ptime = loadJsonTime(str.substr(0, cloc - 1), units);
                        try {
                            iterationIndex = std::stoi(str.substr(cloc + 1));
                        }
                        catch (const std::exception&) {
                            iterationIndex = 0;
                        }
                    }
                } else if (pointElement.isMember("t")) {
                    auto str = pointElement["t"].asString();
                    auto cloc = str.find_last_of(':');
                    if (cloc == std::string::npos) {
                        ptime = loadJsonTime(str, units);
                    } else {
                        ptime = loadJsonTime(str.substr(0, cloc - 1), units);
                        try {
                            iterationIndex = std::stoi(str.substr(cloc + 1));
                        }
                        catch (const std::exception&) {
                            iterationIndex = 0;
                        }
                    }
                } else {
                    std::cout << "time not specified\n";
                    continue;
                }
                defV val;
                if (pointElement.isMember("value")) {
                    auto M = pointElement["value"];
                    if (M.isInt64()) {
                        val = M.asInt64();
                    } else if (M.isDouble()) {
                        val = M.asDouble();
                    } else {
                        val = M.asString();
                    }
                } else if (pointElement.isMember("v")) {
                    auto M = pointElement["v"];
                    if (M.isInt64()) {
                        val = M.asInt64();
                    } else if (M.isDouble()) {
                        val = M.asDouble();
                    } else {
                        val = M.asString();
                    }
                }
                std::string type;
                if (pointElement.isMember("type")) {
                    type = pointElement["type"].asString();
                }
                if (pointElement.isMember("iteration")) {
                    iterationIndex = pointElement["iteration"].asInt();
                }
                std::string key;
                if (pointElement.isMember("key")) {
                    key = pointElement["key"].asString();
                } else {
                    std::cout << "key not specified\n";
                    continue;
                }
                points.resize(points.size() + 1);
                points.back().time = ptime;
                points.back().iteration = iterationIndex;
                points.back().pubName = key;
                points.back().value = std::move(val);
                if (!type.empty()) {
                    points.back().type = type;
                }
            }
        }

        auto messageArray = doc["messages"];
        if (messageArray.isArray()) {
            messages.reserve(messages.size() + messageArray.size());
            for (const auto& messageElement : messageArray) {
                Time ptime;
                if (messageElement.isMember("time")) {
                    ptime = loadJsonTime(messageElement["time"], units);
                } else if (messageElement.isMember("t")) {
                    ptime = loadJsonTime(messageElement["t"], units);
                } else {
                    std::cout << "time not specified\n";
                    continue;
                }
                std::string src;
                if (messageElement.isMember("src")) {
                    src = messageElement["src"].asString();
                }
                if (messageElement.isMember("source")) {
                    src = messageElement["source"].asString();
                }
                std::string dest;
                if (messageElement.isMember("dest")) {
                    dest = messageElement["dest"].asString();
                }
                if (messageElement.isMember("destination")) {
                    dest = messageElement["destination"].asString();
                }
                Time sendTime = ptime;
                std::string type;
                if (messageElement.isMember("sendtime")) {
                    ptime = loadJsonTime(messageElement["sendtime"], units);
                }

                messages.resize(messages.size() + 1);
                messages.back().sendTime = sendTime;
                messages.back().mess.source = src;
                messages.back().mess.dest = dest;
                messages.back().mess.time = ptime;
                if (messageElement.isMember("data")) {
                    auto str = messageElement["data"].asString();
                    if (messageElement.isMember("encoding")) {
                        if (messageElement["encoding"].asString() == "base64") {
                            auto offset = hasB64Wrapper(str);
                            if (offset == 0) {
                                messages.back().mess.data =
                                    gmlc::utilities::base64_decode_to_string(str);
                                continue;
                            }
                        }
                    }
                    messages.back().mess.data = decode(std::move(str));
                } else if (messageElement.isMember("message")) {
                    auto str = messageElement["message"].asString();
                    if (messageElement.isMember("encoding")) {
                        if (messageElement["encoding"].asString() == "base64") {
                            auto offset = hasB64Wrapper(str);
                            if (offset == 0)  // directly encoded no wrapper
                            {
                                messages.back().mess.data =
                                    gmlc::utilities::base64_decode_to_string(str);
                                continue;
                            }
                        }
                    }
                    messages.back().mess.data = decode(std::move(str));
                }
            }
        }
    }

    void Player::sortTags()
    {
        std::sort(points.begin(), points.end(), vComp);
        std::sort(messages.begin(), messages.end(), mComp);
        // collapse tags to the reduced list
        for (auto& vs : points) {
            auto fnd = tags.find(vs.pubName);
            if (fnd != tags.end()) {
                if (fnd->second.empty()) {
                    tags[vs.pubName] = vs.type;
                }
            } else {
                tags.emplace(vs.pubName, vs.type);
            }
        }

        for (auto& ms : messages) {
            epts.emplace(ms.mess.source);
        }
    }

    /** helper function to generate the publications*/
    void Player::generatePublications()
    {
        for (auto& tname : tags) {
            if (pubids.find(tname.first) == pubids.end()) {
                addPublication(tname.first, helics::getTypeFromString(tname.second));
            }
        }
    }

    /** helper function to generate the publications*/
    void Player::generateEndpoints()
    {
        for (const auto& ename : epts) {
            if (eptids.find(ename) == eptids.end()) {
                addEndpoint(ename);
            }
        }
    }

    void Player::cleanUpPointList()
    {
        // load up the indexes
        for (auto& vs : points) {
            vs.index = pubids[vs.pubName];
        }
        /** load the indices for the message*/
        for (auto& ms : messages) {
            ms.index = eptids[ms.mess.source];
        }
    }

    void Player::initialize()
    {
        auto md = fed->getCurrentMode();
        if (md == Federate::modes::startup) {
            sortTags();
            generatePublications();
            generateEndpoints();
            cleanUpPointList();
            fed->enterInitializingMode();
        }
    }

    void Player::sendInformation(Time sendTime, int iteration)
    {
        if (isValidIndex(pointIndex, points)) {
            while (points[pointIndex].time < sendTime) {
                publications[points[pointIndex].index].publish(points[pointIndex].value);
                ++pointIndex;
                if (pointIndex >= points.size()) {
                    break;
                }
            }
            if (isValidIndex(pointIndex, points)) {
                while ((points[pointIndex].time == sendTime) &&
                       (points[pointIndex].iteration == iteration)) {
                    publications[points[pointIndex].index].publish(points[pointIndex].value);
                    ++pointIndex;
                    if (pointIndex >= points.size()) {
                        break;
                    }
                }
            }
        }
        if (isValidIndex(messageIndex, messages)) {
            while (messages[messageIndex].sendTime <= sendTime) {
                endpoints[messages[messageIndex].index].send(messages[messageIndex].mess);
                ++messageIndex;
                if (messageIndex >= messages.size()) {
                    break;
                }
            }
        }
    }

    void Player::runTo(Time stopTime_input)
    {
        auto md = fed->getCurrentMode();
        if (md == Federate::modes::startup) {
            initialize();
        }
        if (md < Federate::modes::executing) {
            sendInformation(-Time::epsilon());

            fed->enterExecutingMode();
            // send the stuff at timeZero
            sendInformation(timeZero);
        } else {
            auto ctime = fed->getCurrentTime();
            if (isValidIndex(pointIndex, points)) {
                while (points[pointIndex].time <= ctime) {
                    ++pointIndex;
                    if (pointIndex >= points.size()) {
                        break;
                    }
                }
            }
            if (isValidIndex(messageIndex, messages)) {
                while (messages[messageIndex].sendTime <= ctime) {
                    ++messageIndex;
                    if (messageIndex >= messages.size()) {
                        break;
                    }
                }
            }
        }

        Time nextPrintTime = (nextPrintTimeStep > timeZero) ? nextPrintTimeStep : Time::maxVal();
        bool moreToSend = true;
        Time nextSendTime = timeZero;
        int nextIteration = 0;
        int currentIteration = 0;
        while (moreToSend) {
            nextSendTime = Time::maxVal();
            if (isValidIndex(pointIndex, points)) {
                nextSendTime = std::min(nextSendTime, points[pointIndex].time);
                nextIteration = points[pointIndex].iteration;
            }
            if (isValidIndex(messageIndex, messages)) {
                nextSendTime = std::min(nextSendTime, messages[messageIndex].sendTime);
                nextIteration = 0;
            }
            if (nextSendTime > stopTime_input) {
                break;
            }
            if (nextSendTime == Time::maxVal()) {
                moreToSend = false;
                continue;
            }
            if ((nextIteration == 0) || (nextSendTime > fed->getCurrentTime())) {
                auto newTime = fed->requestTime(nextSendTime);
                currentIteration = 0;
                sendInformation(newTime, currentIteration);

                if (newTime >= nextPrintTime) {
                    std::cout << "processed for time " << static_cast<double>(newTime) << "\n";
                    nextPrintTime += nextPrintTimeStep;
                }
            } else {
                fed->requestTimeIterative(nextSendTime, iteration_request::force_iteration);
                ++currentIteration;
                sendInformation(nextSendTime, currentIteration);
            }
        }
    }

    void Player::addPublication(const std::string& key, data_type type, const std::string& pubUnits)
    {
        // skip already existing publications
        if (pubids.find(key) != pubids.end()) {
            std::cerr << "publication already exists\n";
        }
        if (!useLocal) {
            publications.emplace_back(GLOBAL, fed.get(), key, type, pubUnits);
        } else {
            auto kp = key.find_first_of("./");
            if (kp == std::string::npos) {
                publications.emplace_back(fed.get(), key, type, pubUnits);
            } else {
                publications.emplace_back(GLOBAL, fed.get(), key, type, pubUnits);
            }
        }
        pubids[key] = static_cast<int>(publications.size()) - 1;
    }

    void Player::addEndpoint(const std::string& endpointName, const std::string& endpointType)
    {
        // skip already existing publications
        if (eptids.find(endpointName) != eptids.end()) {
            std::cerr << "Endpoint already exists\n";
        }
        if (!useLocal) {
            endpoints.emplace_back(GLOBAL, fed.get(), endpointName, endpointType);
        } else {
            auto kp = endpointName.find_first_of("./");
            if (kp == std::string::npos) {
                endpoints.emplace_back(fed.get(), endpointName, endpointType);
            } else {
                endpoints.emplace_back(GLOBAL, fed.get(), endpointName, endpointType);
            }
        }
        eptids[endpointName] = static_cast<int>(endpoints.size()) - 1;
    }

}  // namespace apps
}  // namespace helics

static int hasB64Wrapper(const std::string& str)
{
    if (str.front() == '\"') {
        if (str.size() < 8) {
            return 0;
        }
        if ((str.compare(2, 3, "64[") == 0) && (str[str.size() - 2] == ']')) {
            return 5;
        }
        if (str.size() < 11) {
            return 0;
        }
        if ((str.compare(5, 3, "64[") == 0) && (str[str.size() - 2] == ']')) {
            return 8;
        }
    } else {
        if (str.size() < 6) {
            return 0;
        }
        if ((str.compare(1, 3, "64[") == 0) && (str.back() == ']')) {
            return 4;
        }
        if (str.size() < 9) {
            return 0;
        }
        if ((str.compare(4, 3, "64[") == 0) && (str.back() == ']')) {
            return 7;
        }
    }

    return 0;
}

static std::string decode(std::string&& stringToDecode)
{
    if (stringToDecode.empty()) {
        return std::string();
    }
    auto offset = hasB64Wrapper(stringToDecode);
    if (offset != 0) {
        if (stringToDecode.back() == '\"') {
            stringToDecode.pop_back();
        }

        stringToDecode.pop_back();
        return gmlc::utilities::base64_decode_to_string(stringToDecode, offset);
    }

    if ((stringToDecode.front() == '"') || (stringToDecode.front() == '\'')) {
        try {
            return JsonAsString(loadJsonStr(stringToDecode));
        }
        catch (const Json::Exception&) {
            return gmlc::utilities::stringOps::removeQuotes(stringToDecode);
        }
    }
    // move is required since you are returning the rvalue and we want to move from the rvalue input
    return std::move(stringToDecode);
}
