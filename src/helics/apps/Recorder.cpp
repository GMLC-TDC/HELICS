/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Recorder.hpp"

#include "../application_api/Filters.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/base64.h"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

/** encode the string in base64 if needed otherwise just return the string*/
static std::string encode(std::string_view str2encode)
{
    return std::string("b64[") +
        gmlc::utilities::base64_encode(reinterpret_cast<const unsigned char*>(str2encode.data()),
                                       str2encode.size()) +
        ']';
}

namespace helics::apps {
Recorder::Recorder(std::string_view appName, FederateInfo& fedInfo): App(appName, fedInfo)
{
    initialSetup();
}

Recorder::Recorder(std::vector<std::string> args): App("recorder", std::move(args))
{
    processArgs();
    initialSetup();
}

Recorder::Recorder(int argc, char* argv[]): App("recorder", argc, argv)
{
    processArgs();
    initialSetup();
}

void Recorder::processArgs()
{
    auto app = buildArgParserApp();
    if (!deactivated) {
        app->parse(remArgs);
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}

Recorder::Recorder(std::string_view appName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fedInfo): App(appName, core, fedInfo)
{
    initialSetup();
}

Recorder::Recorder(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    App(appName, core, fedInfo)
{
    initialSetup();
}

Recorder::Recorder(std::string_view appName, const std::string& jsonString):
    App(appName, jsonString)
{
    processArgs();
    initialSetup();
}

Recorder::~Recorder()
{
    try {
        saveFile(outFileName);
    }
    catch (...) {
    }
}

void Recorder::initialSetup()
{
    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_OBSERVER);
        loadInputFiles();
    }
}

void Recorder::loadJsonFile(const std::string& jsonString, bool enableFederateInterfaceRegistration)
{
    loadJsonFileConfiguration("recorder", jsonString, enableFederateInterfaceRegistration);

    auto subCount = fed->getInputCount();
    for (int ii = 0; ii < subCount; ++ii) {
        subscriptions.emplace_back(fed->getInput(ii));
        targets.emplace_back(subscriptions.back().getTarget());
        subids.emplace(subscriptions.back().getHandle(),
                       static_cast<int>(subscriptions.size()) - 1);
        subkeys.emplace(subscriptions.back().getTarget(),
                        static_cast<int>(subscriptions.size()) - 1);
    }
    auto eptCount = fed->getEndpointCount();
    for (int ii = 0; ii < eptCount; ++ii) {
        endpoints.emplace_back(fed->getEndpoint(ii));
        eptNames[endpoints.back().getName()] = static_cast<int>(endpoints.size() - 1);
        eptids.emplace(endpoints.back().getHandle(), static_cast<int>(endpoints.size() - 1));
    }

    auto doc = fileops::loadJson(jsonString);

    auto tags = doc["tag"];
    if (tags.is_array()) {
        for (const auto& tag : tags) {
            addSubscription(tag.get<std::string>());
        }
    } else if (tags.is_string()) {
        addSubscription(tags.get<std::string>());
    }
    auto sourceClone = doc["sourceclone"];
    if (sourceClone.is_array()) {
        for (const auto& clone : sourceClone) {
            addSourceEndpointClone(clone.get<std::string>());
        }
    } else if (sourceClone.is_string()) {
        addSourceEndpointClone(sourceClone.get<std::string>());
    }
    auto destClone = doc["destclone"];
    if (destClone.is_array()) {
        for (const auto& clone : destClone) {
            addDestEndpointClone(clone.get<std::string>());
        }
    } else if (destClone.is_string()) {
        addDestEndpointClone(destClone.get<std::string>());
    }
    auto clones = doc["clone"];
    if (clones.is_array()) {
        for (const auto& clone : clones) {
            addSourceEndpointClone(clone.get<std::string>());
            addDestEndpointClone(clone.get<std::string>());
        }
    } else if (clones.is_string()) {
        addSourceEndpointClone(clones.get<std::string>());
        addDestEndpointClone(clones.get<std::string>());
    }
    auto captures = doc["capture"];
    if (captures.is_array()) {
        for (const auto& capture : captures) {
            addCapture(capture.get<std::string>());
        }
    } else if (captures.is_string()) {
        addCapture(captures.get<std::string>());
    }
}

void Recorder::loadTextFile(const std::string& textFile)
{
    using namespace gmlc::utilities::stringOps;  // NOLINT

    std::ifstream infile(textFile);
    std::string str;
    int lineCount = 0;
    while (std::getline(infile, str)) {
        ++lineCount;
        if (str.empty()) {
            continue;
        }
        auto firstChar = str.find_first_not_of(" \t\n\r\0");
        if ((firstChar == std::string::npos) || (str[firstChar] == '#')) {
            continue;
        }
        auto blk = splitlineQuotes(str, ",\t ", default_quote_chars, delimiter_compression::on);

        switch (blk.size()) {
            case 1:
                addSubscription(removeQuotes(blk[0]));
                break;
            case 2:
                if ((blk[0] == "subscription") || (blk[0] == "s") || (blk[0] == "sub") ||
                    (blk[0] == "tag")) {
                    addSubscription(removeQuotes(blk[1]));
                } else if ((blk[0] == "endpoint") || (blk[0] == "ept") || (blk[0] == "e")) {
                    addEndpoint(removeQuotes(blk[1]));
                } else if ((blk[0] == "sourceclone") || (blk[0] == "source") || (blk[0] == "src")) {
                    addSourceEndpointClone(removeQuotes(blk[1]));
                } else if ((blk[0] == "destclone") || (blk[0] == "dest") ||
                           (blk[0] == "destination")) {
                    addDestEndpointClone(removeQuotes(blk[1]));
                } else if (blk[0] == "capture") {
                    addCapture(removeQuotes(blk[1]));
                } else if (blk[0] == "clone") {
                    addSourceEndpointClone(removeQuotes(blk[1]));
                    addDestEndpointClone(removeQuotes(blk[1]));
                } else {
                    std::cerr << "Unable to process line " << lineCount << ':' << str << '\n';
                }
                break;
            case 3:
                if (blk[0] == "clone") {
                    if ((blk[1] == "source") || (blk[1] == "src")) {
                        addSourceEndpointClone(removeQuotes(blk[2]));
                    } else if ((blk[1] == "dest") || (blk[1] == "destination")) {
                        addDestEndpointClone(removeQuotes(blk[2]));
                    } else {
                        std::cerr << "Unable to process line " << lineCount << ':' << str << '\n';
                    }
                } else {
                    std::cerr << "Unable to process line " << lineCount << ':' << str << '\n';
                }
                break;
            default:
                break;
        }
    }
    infile.close();
}

void Recorder::writeJsonFile(const std::string& filename)
{
    nlohmann::json doc;
    if (!points.empty()) {
        doc["points"] = nlohmann::json(nlohmann::json::array());
        for (auto& point : points) {
            nlohmann::json pointData;
            pointData["key"] = subscriptions[point.index].getTarget();
            pointData["value"] = point.value;
            pointData["time"] = static_cast<double>(point.time);
            if (point.iteration > 0) {
                pointData["iteration"] = point.iteration;
            }
            if (point.first) {
                pointData["type"] = subscriptions[point.index].getPublicationType();
            }
            doc["points"].push_back(pointData);
        }
    }

    if (!messages.empty()) {
        doc["messages"] = nlohmann::json(nlohmann::json::array());
        for (auto& mess : messages) {
            nlohmann::json message;
            message["time"] = static_cast<double>(mess->time);
            message["src"] = mess->source;
            if ((!mess->original_source.empty()) && (mess->original_source != mess->source)) {
                message["original_source"] = mess->original_source;
            }
            if ((mess->dest.size() < 7) ||
                (mess->dest.compare(mess->dest.size() - 6, 6, "cloneE") != 0)) {
                message["dest"] = mess->dest;
                message["orig_dest"] = mess->original_dest;
            } else {
                message["dest"] = mess->original_dest;
            }
            if (isBinaryData(mess->data)) {
                if (isEscapableData(mess->data)) {
                    message["message"] = std::string(mess->data.to_string());
                } else {
                    message["encoding"] = "base64";
                    message["message"] = encode(std::string(mess->data.to_string()));
                }

            } else {
                message["message"] = std::string(mess->data.to_string());
            }
            doc["messages"].push_back(message);
        }
    }

    std::ofstream out(filename);
    out << doc << std::endl;
}

void Recorder::writeTextFile(const std::string& filename)
{
    std::ofstream outFile(filename.empty() ? outFileName : filename);
    if (!points.empty()) {
        outFile << "#time \ttag\t type*\t value\n";
    }
    for (auto& point : points) {
        if (point.first) {
            outFile << static_cast<double>(point.time) << "\t\t"
                    << subscriptions[point.index].getTarget() << '\t'
                    << subscriptions[point.index].getPublicationType() << '\t'
                    << generateJsonQuotedString(point.value) << '\n';
        } else {
            if (point.iteration > 0) {
                outFile << static_cast<double>(point.time) << ':' << point.iteration << "\t\t"
                        << subscriptions[point.index].getTarget() << '\t'
                        << generateJsonQuotedString(point.value) << '\n';
            } else {
                outFile << static_cast<double>(point.time) << "\t\t"
                        << subscriptions[point.index].getTarget() << '\t'
                        << generateJsonQuotedString(point.value) << '\n';
            }
        }
    }
    if (!messages.empty()) {
        outFile << "# m\t time \tsource\t dest\t message\n";
    }
    for (auto& mess : messages) {
        outFile << "m\t" << static_cast<double>(mess->time) << '\t' << mess->source << '\t';
        if ((mess->dest.size() < 7) ||
            (mess->dest.compare(mess->dest.size() - 6, 6, "cloneE") != 0)) {
            outFile << mess->dest;
        } else {
            outFile << mess->original_dest;
        }
        if (isBinaryData(mess->data)) {
            if (isEscapableData(mess->data)) {
                outFile << "\t" << generateJsonQuotedString(std::string(mess->data.to_string()))
                        << "\n";
            } else {
                outFile << "\t\"" << encode(mess->data.to_string()) << "\"\n";
            }

        } else {
            outFile << "\t\"" << mess->data.to_string() << "\"\n";
        }
    }
}

void Recorder::initialize()
{
    fed->enterInitializingModeIterative();
    generateInterfaces();

    vStat.resize(subids.size());
    for (auto& val : subkeys) {
        vStat[val.second].key = val.first;
    }

    fed->enterInitializingMode();
    captureForCurrentTime(-1.0);

    fed->enterExecutingMode();
    captureForCurrentTime(0.0);
}

void Recorder::generateInterfaces()
{
    for (auto& tag : subkeys) {
        if (tag.second == -1) {
            addSubscription(tag.first);
        }
    }
    for (auto& ept : eptNames) {
        if (ept.second == -1) {
            addEndpoint(ept.first);
        }
    }
    loadCaptureInterfaces();
}

void Recorder::loadCaptureInterfaces()
{
    for (auto& capt : captureInterfaces) {
        auto pubs =
            vectorizeQueryResult(fed->query(capt, "publications", HELICS_SEQUENCING_MODE_FAST));
        for (auto& pub : pubs) {
            addSubscription(pub);
        }
    }
}

void Recorder::captureForCurrentTime(Time currentTime, int iteration)
{
    for (auto& sub : subscriptions) {
        if (sub.isUpdated()) {
            auto val = sub.getValue<std::string>();
            const int subId = subids[sub.getHandle()];
            points.emplace_back(currentTime, subId, val);
            if (iteration > 0) {
                points.back().iteration = iteration;
            }
            if (verbose) {
                std::string valstr;
                if (val.size() < 150) {
                    if (iteration > 0) {
                        valstr = fmt::format("[{}:{}]value {}={}",
                                             static_cast<double>(currentTime),
                                             iteration,
                                             sub.getTarget(),
                                             val);
                    } else {
                        valstr = fmt::format("[{}]value {}={}",
                                             static_cast<double>(currentTime),
                                             sub.getTarget(),
                                             val);
                    }
                } else {
                    if (iteration > 0) {
                        valstr = fmt::format("[{}:{}]value {}=block[{}]",
                                             static_cast<double>(currentTime),
                                             iteration,
                                             sub.getTarget(),
                                             val.size());
                    } else {
                        valstr = fmt::format("[{}]value {}=block[{}]",
                                             static_cast<double>(currentTime),
                                             sub.getTarget(),
                                             val.size());
                    }
                }
                spdlog::info(valstr);
            }
            if (vStat[subId].cnt == 0) {
                points.back().first = true;
            }
            ++vStat[subId].cnt;
            vStat[subId].lastVal = val;
            vStat[subId].time = -1.0;
        }
    }

    for (auto& ept : endpoints) {
        while (ept.hasMessage()) {
            auto mess = ept.getMessage();
            if (verbose) {
                std::string messstr;
                if (mess->data.size() < 50) {
                    messstr = fmt::format("[{}]message from {} to {}::{}",
                                          static_cast<double>(currentTime),
                                          mess->source,
                                          mess->dest,
                                          mess->data.to_string());
                } else {
                    messstr = fmt::format("[{}]message from {} to {}:: size {}",
                                          static_cast<double>(currentTime),
                                          mess->source,
                                          mess->dest,
                                          mess->data.size());
                }
                spdlog::info(messstr);
            }
            messages.push_back(std::move(mess));
        }
    }
    // get the clone endpoints
    if (cloneEndpoint) {
        while (cloneEndpoint->hasMessage()) {
            messages.push_back(cloneEndpoint->getMessage());
        }
    }
}

/** run the Player until the specified time*/
void Recorder::runTo(Time runToTime)
{
    if (fed->getCurrentMode() == Federate::Modes::STARTUP) {
        initialize();
    }

    if (!mapfile.empty()) {
        std::ofstream out(mapfile);
        for (auto& stat : vStat) {
            //    out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time)
            //    << '\t' << stat.lastVal
            //        << '\n';
            fmt::print(out,
                       "{}\t{}\t{}\t{}\n",
                       stat.key,
                       stat.cnt,
                       static_cast<double>(stat.time),
                       stat.lastVal);
        }
        out.flush();
    }
    Time nextPrintTime = (nextPrintTimeStep > timeZero) ? nextPrintTimeStep : Time::maxVal();
    try {
        int iteration = 0;
        while (true) {
            helics::Time grantedTime;
            if (allow_iteration) {
                auto ItRes =
                    fed->requestTimeIterative(runToTime, IterationRequest::ITERATE_IF_NEEDED);
                if (ItRes.state == IterationResult::NEXT_STEP) {
                    iteration = 0;
                }
                grantedTime = ItRes.grantedTime;
                captureForCurrentTime(grantedTime, iteration);
                ++iteration;
            } else {
                grantedTime = fed->requestTime(runToTime);
                captureForCurrentTime(grantedTime);
            }
            if (!mapfile.empty()) {
                std::ofstream out(mapfile);
                for (auto& stat : vStat) {
                    fmt::print(out,
                               "{}\t{}\t{}\t{}\n",
                               stat.key,
                               stat.cnt,
                               static_cast<double>(stat.time),
                               stat.lastVal);
                }
                out.flush();
            }
            if (grantedTime >= runToTime) {
                break;
            }
            if ((grantedTime >= nextPrintTime) && (nextPrintTimeStep > timeZero)) {
                std::cout << "processed for time " << static_cast<double>(grantedTime) << "\n";
                nextPrintTime += nextPrintTimeStep;
            }
        }
    }
    catch (...) {
    }
}
/** add a subscription to record*/
void Recorder::addSubscription(std::string_view key)
{
    auto res = subkeys.find(key);
    if ((res == subkeys.end()) || (res->second == -1)) {
        subscriptions.emplace_back(fed->registerSubscription(key));
        targets.emplace_back(key);
        auto index = static_cast<int>(subscriptions.size()) - 1;
        auto subId = subscriptions.back().getHandle();
        subids[subId] = index;  // this is a new element
        subkeys[subscriptions.back().getTarget()] = index;  // this is a potential replacement
    }
}
/** add an endpoint*/
void Recorder::addEndpoint(std::string_view endpoint)
{
    auto res = eptNames.find(endpoint);
    if ((res == eptNames.end()) || (res->second == -1)) {
        endpoints.emplace_back(InterfaceVisibility::GLOBAL, fed.get(), endpoint);
        auto index = static_cast<int>(endpoints.size()) - 1;
        auto endpointId = endpoints.back().getHandle();
        eptids.emplace(endpointId, index);  // this is a new element
        eptNames[endpoints.back().getName()] = index;  // this is a potential replacement
    }
}

void Recorder::addSourceEndpointClone(std::string_view sourceEndpoint)
{
    if (!cFilt) {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addSourceTarget(sourceEndpoint);
}

void Recorder::addDestEndpointClone(std::string_view destEndpoint)
{
    if (!cFilt) {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addDestinationTarget(destEndpoint);
}

void Recorder::addCapture(std::string_view captureDesc)
{
    captureInterfaces.emplace_back(captureDesc);
}

std::tuple<Time, std::string_view, std::string> Recorder::getValue(std::size_t index) const
{
    if (isValidIndex(index, points)) {
        return {points[index].time, targets[points[index].index], points[index].value};
    }
    return {Time(), std::string(), std::string()};
}

std::unique_ptr<Message> Recorder::getMessage(std::size_t index) const
{
    if (isValidIndex(index, messages)) {
        return std::make_unique<Message>(*messages[index]);
    }
    return nullptr;
}

/** save the data to a file*/
void Recorder::saveFile(const std::string& filename)
{
    auto lastP = filename.find_last_of('.');
    auto ext = (lastP != std::string::npos) ? filename.substr(lastP) : std::string{};
    if ((ext == ".json") || (ext == ".JSON")) {
        writeJsonFile(filename);
    } else {
        writeTextFile(filename);
    }
}

std::shared_ptr<helicsCLI11App> Recorder::buildArgParserApp()
{
    using gmlc::utilities::stringOps::removeQuotes;
    using gmlc::utilities::stringOps::splitlineQuotes;

    auto app = std::make_shared<helicsCLI11App>("Command line options for the Recorder App");
    app->add_flag("--allow_iteration", allow_iteration, "allow iteration on values")
        ->ignore_underscore();
    app->add_option(
        "--marker",
        nextPrintTimeStep,
        "print a statement indicating time advancement every <arg> period during the simulation");
    app->add_flag("--verbose", verbose, "print all value results to the screen");
    app->add_option("--mapfile",
                    mapfile,
                    "write progress to a map file for concurrent progress monitoring");

    app->add_option("--output,-o", outFileName, "the output file for recording the data")
        ->capture_default_str();

    auto* clone_group =
        app->add_option_group("cloning",
                              "Options related to endpoint cloning operations and specifications");
    clone_group->add_option("--clone", "existing endpoints to clone all packets to and from")
        ->each([this](const std::string& clone) {
            addDestEndpointClone(clone);
            addSourceEndpointClone(clone);
        })
        ->delimiter(',')
        ->type_size(-1);

    clone_group
        ->add_option(
            "--sourceclone",
            "existing endpoints to capture generated packets from, this argument may be specified multiple time")
        ->each([this](const std::string& clone) { addSourceEndpointClone(clone); })
        ->delimiter(',')
        ->ignore_underscore()
        ->type_size(-1);

    clone_group
        ->add_option("--destclone",
                     "existing endpoints to capture all packets with the specified endpoint as a "
                     "destination, this argument may be specified multiple time")
        ->each([this](const std::string& clone) { addSourceEndpointClone(clone); })
        ->delimiter(',')
        ->ignore_underscore()
        ->type_size(-1);

    auto* capture_group =
        app->add_option_group("capture_group",
                              "Options related to capturing publications, endpoints, or federates");
    capture_group
        ->add_option(
            "--tag,--publication,--pub",
            "tags(publications) to record, this argument may be specified any number of times")
        ->each([this](const std::string& tag) {
            auto taglist = splitlineQuotes(tag);
            for (const auto& tagname : taglist) {
                subkeys.emplace(removeQuotes(tagname), -1);
            }
        })
        ->type_size(-1);

    capture_group
        ->add_option("--endpoints",
                     "endpoints to capture, this argument may be specified multiple time")
        ->each([this](const std::string& ept) {
            auto eptlist = splitlineQuotes(ept);
            for (const auto& eptname : eptlist) {
                eptNames.emplace(removeQuotes(eptname), -1);
            }
        })
        ->type_size(-1);

    capture_group
        ->add_option("--capture",
                     "capture all the publications of a particular federate capture=\"fed1;fed2\"  "
                     "supports multiple arguments or a semicolon/comma separated list")
        ->each([this](const std::string& capt) {
            auto captFeds = splitlineQuotes(capt);
            for (auto& captFed : captFeds) {
                auto actCapt = removeQuotes(captFed);
                captureInterfaces.push_back(actCapt);
            }
        })
        ->type_size(-1);

    return app;
}

}  // namespace helics::apps
