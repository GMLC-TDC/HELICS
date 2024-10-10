/*
Copyright (c) 2017-2024,
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
static int hasB64Wrapper(std::string_view str);
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

namespace helics::apps {

static inline bool vComp(const ValueSetter& v1, const ValueSetter& v2)
{
    return (v1.time == v2.time) ? (v1.iteration < v2.iteration) : (v1.time < v2.time);
}
static inline bool mComp(const MessageHolder& m1, const MessageHolder& m2)
{
    return (m1.sendTime < m2.sendTime);
}

Player::Player(std::vector<std::string> args): App("player_${#}", std::move(args))
{
    processArgs();
    initialSetup();
}

Player::Player(int argc, char* argv[]): App("player_${#}", argc, argv)
{
    processArgs();
    initialSetup();
}

void Player::processArgs()
{
    auto app = generateParser();

    if (!deactivated) {
        app->helics_parse(remArgs);
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}

void Player::initialSetup()
{
    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
        loadInputFiles();
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
               return (defType != helics::DataType::HELICS_CUSTOM);
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

Player::Player(std::string_view appName, const FederateInfo& fedInfo): App(appName, fedInfo)
{
    initialSetup();
}

Player::Player(std::string_view appName,
               const std::shared_ptr<Core>& core,
               const FederateInfo& fedInfo): App(appName, core, fedInfo)
{
    initialSetup();
}

Player::Player(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    App(appName, core, fedInfo)
{
    initialSetup();
}

Player::Player(std::string_view appName, const std::string& configString):
    App(appName, configString)
{
    processArgs();
    initialSetup();
}

void Player::addMessage(Time sendTime,
                        std::string_view src,
                        std::string_view dest,
                        std::string_view payload)
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
                        std::string_view src,
                        std::string_view dest,
                        std::string_view payload)
{
    messages.resize(messages.size() + 1);
    messages.back().sendTime = sendTime;
    messages.back().mess.data = payload;
    messages.back().mess.source = src;
    messages.back().mess.dest = dest;
    messages.back().mess.time = actionTime;
}

helics::Time Player::extractTime(std::string_view str, int lineNumber) const
{
    try {
        if (units == time_units::ns)  // ns
        {
            return {std::stoll(std::string(str)), time_units::ns};
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
    using namespace gmlc::utilities::stringOps;  // NOLINT

    AppTextParser aparser(filename);
    auto cnts = aparser.preParseFile({'m', 'M'});

    int mcnt = cnts[1] + cnts[2];
    int pcnt = cnts[0] - mcnt;
    int pIndex = static_cast<int>(points.size());
    points.resize(points.size() + pcnt);
    int mIndex = static_cast<int>(messages.size());
    messages.resize(messages.size() + mcnt);

    aparser.reset();

    if (!aparser.configString().empty()) {
        App::loadConfigOptions(aparser);
        auto app = generateParser();
        std::istringstream sstr(aparser.configString());
        app->parse_from_stream(sstr);
    }
    std::string str;
    int lineNumber;
    while (aparser.loadNextLine(str, lineNumber)) {
        /* time key type value units*/
        auto blk = splitlineBracket(str, ",\t ", default_bracket_chars, delimiter_compression::on);

        trimString(blk[0]);
        if ((blk[0].front() == 'm') || (blk[0].front() == 'M')) {
            // deal with messages
            switch (blk.size()) {
                case 5:
                    if ((messages[mIndex].sendTime = extractTime(blk[1], lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }

                    messages[mIndex].mess.source = blk[2];
                    messages[mIndex].mess.dest = blk[3];
                    messages[mIndex].mess.time = messages[mIndex].sendTime;
                    messages[mIndex].mess.data = decode(std::move(blk[4]));
                    break;
                case 6:
                    if ((messages[mIndex].sendTime = extractTime(blk[1], lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }

                    messages[mIndex].mess.source = blk[3];
                    messages[mIndex].mess.dest = blk[4];
                    if ((messages[mIndex].mess.time = extractTime(blk[2], lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }
                    messages[mIndex].mess.data = decode(std::move(blk[5]));
                    break;
                default:
                    std::cerr << "unknown message format line " << lineNumber << '\n';
                    break;
            }
            ++mIndex;
        } else {
            if (blk.size() == 2) {
                auto cloc = blk[0].find_last_of(':');
                if (cloc == std::string::npos) {
                    if ((points[pIndex].time = extractTime(trim(blk[0]), lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }
                } else {
                    if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                           lineNumber)) == Time::minVal()) {
                        continue;
                    }
                    points[pIndex].iteration = std::stoi(blk[0].substr(cloc + 1));
                }
                if (pIndex > 0) {
                    points[pIndex].pubName = points[static_cast<size_t>(pIndex) - 1].pubName;
                } else {
                    std::cerr
                        << "lines without publication name but follow one with a publication line "
                        << lineNumber << '\n';
                }
                points[pIndex].value = decode(std::move(blk[1]));
                ++pIndex;
            } else if (blk.size() == 3) {
                auto cloc = blk[0].find_last_of(':');
                if (cloc == std::string::npos) {
                    if ((points[pIndex].time = extractTime(trim(blk[0]), lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }
                } else {
                    if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                           lineNumber)) == Time::minVal()) {
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
                    if ((points[pIndex].time = extractTime(trim(blk[0]), lineNumber)) ==
                        Time::minVal()) {
                        continue;
                    }
                } else {
                    if ((points[pIndex].time = extractTime(trim(blk[0]).substr(0, cloc),
                                                           lineNumber)) == Time::minVal()) {
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
                std::cerr << "unknown publish format line " << lineNumber << '\n';
            }
        }
    }
}

void Player::loadJsonFile(const std::string& jsonString, bool enableFederateInterfaceRegistration)
{
    loadJsonFileConfiguration("player", jsonString, enableFederateInterfaceRegistration);

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

    auto doc = fileops::loadJson(jsonString);

    if (doc.contains("player")) {
        auto& playerConfig = doc["player"];
        if (playerConfig.contains("time_units")) {
            if (playerConfig["time_units"].get<std::string>() == "ns") {
                timeMultiplier = 1e-9;
            }
        }
    }
    auto& pointArray = doc["points"];
    if (pointArray.is_array()) {
        points.reserve(points.size() + pointArray.size());
        for (const auto& pointElement : pointArray) {
            Time ptime;
            int iterationIndex = 0;
            if (pointElement.contains("time")) {
                const auto& telement = pointElement["time"];
                auto str = telement.is_number() ? std::to_string(telement.get<double>()) :
                                                  telement.get<std::string>();
                auto cloc = str.find_last_of(':');
                if (cloc == std::string::npos) {
                    ptime = fileops::loadJsonTime(str, units);
                } else {
                    ptime = fileops::loadJsonTime(str.substr(0, cloc - 1), units);
                    try {
                        iterationIndex = std::stoi(str.substr(cloc + 1));
                    }
                    catch (const std::exception&) {
                        iterationIndex = 0;
                    }
                }
            } else if (pointElement.contains("t")) {
                auto str = pointElement["t"].get<std::string>();
                auto cloc = str.find_last_of(':');
                if (cloc == std::string::npos) {
                    ptime = fileops::loadJsonTime(str, units);
                } else {
                    ptime = fileops::loadJsonTime(str.substr(0, cloc - 1), units);
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
            if (pointElement.contains("value")) {
                auto& M = pointElement["value"];
                if (M.is_number_integer()) {
                    val = M.get<int64_t>();
                } else if (M.is_number()) {
                    val = M.get<double>();
                } else {
                    val = M.get<std::string>();
                }
            } else if (pointElement.contains("v")) {
                auto& M = pointElement["v"];
                if (M.is_number_integer()) {
                    val = M.get<int64_t>();
                } else if (M.is_number()) {
                    val = M.get<double>();
                } else {
                    val = M.get<std::string>();
                }
            }
            std::string type;
            if (pointElement.contains("type")) {
                type = pointElement["type"].get<std::string>();
            }
            if (pointElement.contains("iteration")) {
                iterationIndex = pointElement["iteration"].get<int>();
            }
            std::string key;
            if (pointElement.contains("key")) {
                key = pointElement["key"].get<std::string>();
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

    auto& messageArray = doc["messages"];
    if (messageArray.is_array()) {
        messages.reserve(messages.size() + messageArray.size());
        for (const auto& messageElement : messageArray) {
            Time ptime;
            if (messageElement.contains("time")) {
                ptime = fileops::loadJsonTime(messageElement["time"], units);
            } else if (messageElement.contains("t")) {
                ptime = fileops::loadJsonTime(messageElement["t"], units);
            } else {
                std::cout << "time not specified\n";
                continue;
            }
            std::string src;
            if (messageElement.contains("src")) {
                src = messageElement["src"].get<std::string>();
            }
            if (messageElement.contains("source")) {
                src = messageElement["source"].get<std::string>();
            }
            std::string dest;
            if (messageElement.contains("dest")) {
                dest = messageElement["dest"].get<std::string>();
            }
            if (messageElement.contains("destination")) {
                dest = messageElement["destination"].get<std::string>();
            }
            Time sendTime = ptime;
            std::string type;
            if (messageElement.contains("sendtime")) {
                ptime = fileops::loadJsonTime(messageElement["sendtime"], units);
            }

            messages.resize(messages.size() + 1);
            messages.back().sendTime = sendTime;
            messages.back().mess.source = src;
            messages.back().mess.dest = dest;
            messages.back().mess.time = ptime;
            if (messageElement.contains("data")) {
                auto str = messageElement["data"].get<std::string>();
                if (messageElement.contains("encoding")) {
                    if (messageElement["encoding"].get<std::string>() == "base64") {
                        auto offset = hasB64Wrapper(str);
                        if (offset == 0) {
                            messages.back().mess.data =
                                gmlc::utilities::base64_decode_to_string(str);
                            continue;
                        }
                    }
                }
                messages.back().mess.data = decode(std::move(str));
            } else if (messageElement.contains("message")) {
                auto str = messageElement["message"].get<std::string>();
                if (messageElement.contains("encoding")) {
                    if (messageElement["encoding"].get<std::string>() == "base64") {
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
    if (md == Federate::Modes::STARTUP) {
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
    if (md == Federate::Modes::STARTUP) {
        initialize();
    }
    if (md < Federate::Modes::EXECUTING) {
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
    bool moreToSend{true};
    int nextIteration{0};
    int currentIteration{0};
    while (moreToSend) {
        auto nextSendTime = Time::maxVal();
        if (isValidIndex(pointIndex, points)) {
            nextSendTime = std::min(nextSendTime, points[pointIndex].time);
            nextIteration = points[pointIndex].iteration;
        }
        if (isValidIndex(messageIndex, messages)) {
            nextSendTime = std::min(nextSendTime, messages[messageIndex].sendTime);
            nextIteration = 0;
        }
        if (nextSendTime == Time::maxVal()) {
            moreToSend = false;
            continue;
        }
        if (nextSendTime > stopTime_input) {
            break;
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
            fed->requestTimeIterative(nextSendTime, IterationRequest::FORCE_ITERATION);
            ++currentIteration;
            sendInformation(nextSendTime, currentIteration);
        }
    }
    while (fed->getCurrentTime() < stopTime_input) {
        fed->requestTime(stopTime_input);
    }
}

void Player::addPublication(std::string_view name, DataType type, std::string_view pubUnits)
{
    // skip already existing publications
    if (pubids.find(name) != pubids.end()) {
        std::cerr << "publication already exists\n";
        return;
    }
    if (!useLocal) {
        publications.emplace_back(
            helics::InterfaceVisibility::GLOBAL, fed.get(), name, type, pubUnits);
    } else {
        auto kp = name.find_first_of("./");
        if (kp == std::string::npos) {
            publications.emplace_back(fed.get(), name, type, pubUnits);
        } else {
            publications.emplace_back(
                helics::InterfaceVisibility::GLOBAL, fed.get(), name, type, pubUnits);
        }
    }
    std::string_view keyname = publications.back().getName();
    auto index = static_cast<int>(publications.size()) - 1;
    pubids[keyname] = index;
    if (useLocal) {
        auto& fedName = fed->getName();
        if (keyname.compare(0, fedName.size(), fedName) == 0) {
            auto localName = keyname.substr(fedName.size() + 1);
            pubids[localName] = index;
        }
    }
}

void Player::addEndpoint(std::string_view endpointName, std::string_view endpointType)
{
    // skip already existing publications
    if (eptids.find(endpointName) != eptids.end()) {
        std::cerr << "Endpoint already exists\n";
        return;
    }
    if (!useLocal) {
        endpoints.emplace_back(helics::InterfaceVisibility::GLOBAL,
                               fed.get(),
                               endpointName,
                               endpointType);
    } else {
        auto kp = endpointName.find_first_of("./");
        if (kp == std::string::npos) {
            endpoints.emplace_back(fed.get(), endpointName, endpointType);
        } else {
            endpoints.emplace_back(helics::InterfaceVisibility::GLOBAL,
                                   fed.get(),
                                   endpointName,
                                   endpointType);
        }
    }
    eptids[endpoints.back().getName()] = static_cast<int>(endpoints.size()) - 1;
}

}  // namespace helics::apps

static int hasB64Wrapper(std::string_view str)
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
            return helics::fileops::JsonAsString(helics::fileops::loadJsonStr(stringToDecode));
        }
        catch (const nlohmann::json::exception&) {
            return gmlc::utilities::stringOps::removeQuotes(stringToDecode);
        }
    }
    // move is required since you are returning the rvalue and we want to move from the rvalue input
    return std::move(stringToDecode);
}
