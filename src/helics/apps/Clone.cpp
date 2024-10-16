/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Clone.hpp"

#include "../application_api/Filters.hpp"
#include "../application_api/queryFunctions.hpp"
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
                                       static_cast<int>(str2encode.size())) +
        ']';
}

namespace helics::apps {
Clone::Clone(std::string_view appName, FederateInfo& fedInfo): App(appName, fedInfo)
{
    initialSetup();
}

Clone::Clone(std::vector<std::string> args): App("Clone", std::move(args))
{
    processArgs();
    initialSetup();
}

Clone::Clone(int argc, char* argv[]): App("Clone", argc, argv)
{
    processArgs();
    initialSetup();
}

void Clone::processArgs()
{
    auto app = buildArgParserApp();
    if (!deactivated) {
        app->parse(remArgs);
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}

Clone::Clone(std::string_view appName,
             const std::shared_ptr<Core>& core,
             const FederateInfo& fedInfo): App(appName, core, fedInfo)
{
    initialSetup();
}

Clone::Clone(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    App(appName, core, fedInfo)
{
    initialSetup();
}

Clone::Clone(std::string_view appName, const std::string& jsonString): App(appName, jsonString)
{
    processArgs();
    initialSetup();
}

Clone::~Clone()
{
    try {
        if (!fileSaved && !outFileName.empty()) {
            saveFile(outFileName);
        }
    }
    catch (...) {
    }
}

void Clone::initialSetup()
{
    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_OBSERVER);
        loadInputFiles();
    }
}

void Clone::saveFile(const std::string& filename)
{
    if (filename.empty()) {
        if (!outFileName.empty()) {
            saveFile(outFileName);
        }
        return;
    }
    nlohmann::json doc = fileops::loadJsonStr(fedConfig);
    doc["defaultglobal"] = true;
    if (!cloneSubscriptionNames.empty()) {
        doc["optional"] = true;

        doc["subscriptions"] = nlohmann::json(nlohmann::json::array());
        for (auto& sub : cloneSubscriptionNames) {
            nlohmann::json subsc;
            subsc["key"] = sub;
            doc["subscriptions"].push_back(subsc);
        }
    }
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

    std::ofstream outfile(filename);
    outfile << doc << std::endl;
    fileSaved = true;
}

void Clone::initialize()
{
    generateInterfaces();

    pubPointCount.resize(subids.size(), 0);

    fed->enterInitializingMode();
    captureForCurrentTime(-1.0);

    fed->enterExecutingMode();
    captureForCurrentTime(0.0);
}

void Clone::generateInterfaces()
{
    auto res = waitForInit(fed.get(), captureFederate);
    if (res) {
        fed->query("root", "global_flush", HELICS_SEQUENCING_MODE_ORDERED);
        auto pubs = vectorizeQueryResult(
            fed->query(captureFederate, "publications", HELICS_SEQUENCING_MODE_ORDERED));
        for (auto& pub : pubs) {
            if (pub.empty()) {
                continue;
            }
            addSubscription(pub);
        }
        auto epts = vectorizeQueryResult(
            fed->query(captureFederate, "endpoints", HELICS_SEQUENCING_MODE_ORDERED));
        for (auto& ept : epts) {
            if (ept.empty()) {
                continue;
            }
            addSourceEndpointClone(ept);
        }
        cloneSubscriptionNames =
            vectorizeQueryResult(queryFederateSubscriptions(fed.get(), captureFederate));
        // get rid of any empty strings that may have come to be
        cloneSubscriptionNames.erase(std::remove(cloneSubscriptionNames.begin(),
                                                 cloneSubscriptionNames.end(),
                                                 std::string{}),
                                     cloneSubscriptionNames.end());

        fedConfig = fed->query(captureFederate, "config", HELICS_SEQUENCING_MODE_ORDERED);
    }
}

void Clone::captureForCurrentTime(Time currentTime, int iteration)
{
    for (auto& sub : subscriptions) {
        if (sub.isUpdated()) {
            auto val = sub.getValue<std::string>();
            const int subid = subids[sub.getHandle()];
            points.emplace_back(currentTime, subid, val);
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
            if (pubPointCount[subid] == 0) {
                points.back().first = true;
            }
            ++pubPointCount[subid];
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
void Clone::runTo(Time runToTime)
{
    initialize();

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
void Clone::addSubscription(std::string_view key)
{
    auto res = subkeys.find(key);
    if ((res == subkeys.end()) || (res->second == -1)) {
        subscriptions.emplace_back(fed->registerSubscription(key));
        auto index = static_cast<int>(subscriptions.size()) - 1;
        auto subid = subscriptions.back().getHandle();
        subids[subid] = index;  // this is a new element
        subkeys[subscriptions.back().getTarget()] = index;  // this is a potential replacement
    }
}

void Clone::addSourceEndpointClone(std::string_view sourceEndpoint)
{
    if (!cFilt) {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addSourceTarget(sourceEndpoint);
}

void Clone::setFederateToClone(std::string_view federateName)
{
    captureFederate = federateName;
}

std::tuple<Time, std::string, std::string> Clone::getValue(int index) const
{
    if (isValidIndex(index, points)) {
        return {points[index].time,
                subscriptions[points[index].index].getTarget(),
                points[index].value};
    }
    return {Time(), std::string(), std::string()};
}

std::unique_ptr<Message> Clone::getMessage(int index) const
{
    if (isValidIndex(index, messages)) {
        return std::make_unique<Message>(*messages[index]);
    }
    return nullptr;
}

std::shared_ptr<helicsCLI11App> Clone::buildArgParserApp()
{
    auto app = std::make_shared<helicsCLI11App>("Command line options for the Clone App");
    if (!app) {
        throw(FunctionExecutionFailure("unable to allocate application CLI"));
    }
    app->add_flag("--allow_iteration", allow_iteration, "allow iteration on values")
        ->ignore_underscore();

    app->add_option("--output,-o", outFileName, "the output file for recording the data")
        ->capture_default_str();
    app->add_option("capture", captureFederate, "name of the federate to clone");

    return app;
}

}  // namespace helics::apps
