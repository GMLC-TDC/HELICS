/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Clone.hpp"

#include "../application_api/Filters.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
#include "../common/fmt_ostream.h"
#include "../core/helicsCLI11.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/base64.h"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** encode the string in base64 if needed otherwise just return the string*/
static std::string encode(const std::string& str2encode)
{
    return std::string("b64[") +
        gmlc::utilities::base64_encode(reinterpret_cast<const unsigned char*>(str2encode.c_str()),
                                       static_cast<int>(str2encode.size())) +
        ']';
}

namespace helics {
namespace apps {
    Clone::Clone(const std::string& appName, FederateInfo& fi): App(appName, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Clone::Clone(std::vector<std::string> args): App("Clone", std::move(args)) { processArgs(); }

    Clone::Clone(int argc, char* argv[]): App("Clone", argc, argv) { processArgs(); }

    void Clone::processArgs()
    {
        auto app = buildArgParserApp();
        if (!deactivated) {
            fed->setFlagOption(helics_flag_observer);
            app->parse(remArgs);
            if (!masterFileName.empty()) {
                loadFile(masterFileName);
            }
        } else if (helpMode) {
            app->remove_helics_specifics();
            std::cout << app->help();
        }
    }

    Clone::Clone(const std::string& appName,
                 const std::shared_ptr<Core>& core,
                 const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Clone::Clone(const std::string& appName, CoreApp& core, const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Clone::Clone(const std::string& appName, const std::string& jsonString):
        App(appName, jsonString)
    {
        fed->setFlagOption(helics_flag_observer);
        Clone::loadJsonFile(jsonString);
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

    void Clone::saveFile(const std::string& filename)
    {
        if (filename.empty()) {
            if (!outFileName.empty()) {
                saveFile(outFileName);
            }
            return;
        }
        Json::Value doc = loadJsonStr(fedConfig);
        doc["defaultglobal"] = true;
        if (!cloneSubscriptionNames.empty()) {
            doc["optional"] = true;

            doc["subscriptions"] = Json::Value(Json::arrayValue);
            for (auto& sub : cloneSubscriptionNames) {
                Json::Value subsc;
                subsc["key"] = sub;
                doc["subscriptions"].append(subsc);
            }
        }
        if (!points.empty()) {
            doc["points"] = Json::Value(Json::arrayValue);
            for (auto& v : points) {
                Json::Value point;
                point["key"] = subscriptions[v.index].getTarget();
                point["value"] = v.value;
                point["time"] = static_cast<double>(v.time);
                if (v.iteration > 0) {
                    point["iteration"] = v.iteration;
                }
                if (v.first) {
                    point["type"] = subscriptions[v.index].getPublicationType();
                }
                doc["points"].append(point);
            }
        }

        if (!messages.empty()) {
            doc["messages"] = Json::Value(Json::arrayValue);
            for (auto& mess : messages) {
                Json::Value message;
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
                        message["message"] = mess->data.to_string();
                    } else {
                        message["encoding"] = "base64";
                        message["message"] = encode(mess->data.to_string());
                    }

                } else {
                    message["message"] = mess->data.to_string();
                }
                doc["messages"].append(message);
            }
        }

        std::ofstream o(filename);
        o << doc << std::endl;
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
            fed->query("root", "global_flush", helics_sequencing_mode_ordered);
            auto pubs = vectorizeQueryResult(
                fed->query(captureFederate, "publications", helics_sequencing_mode_ordered));
            for (auto& pub : pubs) {
                if (pub.empty()) {
                    continue;
                }
                addSubscription(pub);
            }
            auto epts = vectorizeQueryResult(
                fed->query(captureFederate, "endpoints", helics_sequencing_mode_ordered));
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

            fedConfig = fed->query(captureFederate, "config", helics_sequencing_mode_ordered);
        }
    }

    void Clone::captureForCurrentTime(Time currentTime, int iteration)
    {
        for (auto& sub : subscriptions) {
            if (sub.isUpdated()) {
                auto val = sub.getValue<std::string>();
                int ii = subids[sub.getHandle()];
                points.emplace_back(currentTime, ii, val);
                if (iteration > 0) {
                    points.back().iteration = iteration;
                }
                if (verbose) {
                    std::string valstr;
                    if (val.size() < 150) {
                        if (iteration > 0) {
                            valstr = fmt::format(
                                "[{}:{}]value {}={}", currentTime, iteration, sub.getTarget(), val);
                        } else {
                            valstr =
                                fmt::format("[{}]value {}={}", currentTime, sub.getTarget(), val);
                        }
                    } else {
                        if (iteration > 0) {
                            valstr = fmt::format("[{}:{}]value {}=block[{}]",
                                                 currentTime,
                                                 iteration,
                                                 sub.getTarget(),
                                                 val.size());
                        } else {
                            valstr = fmt::format("[{}]value {}=block[{}]",
                                                 currentTime,
                                                 sub.getTarget(),
                                                 val.size());
                        }
                    }
                    spdlog::info(valstr);
                }
                if (pubPointCount[ii] == 0) {
                    points.back().first = true;
                }
                ++pubPointCount[ii];
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
                helics::Time T;
                if (allow_iteration) {
                    auto ItRes =
                        fed->requestTimeIterative(runToTime, iteration_request::iterate_if_needed);
                    if (ItRes.state == iteration_result::next_step) {
                        iteration = 0;
                    }
                    T = ItRes.grantedTime;
                    captureForCurrentTime(T, iteration);
                    ++iteration;
                } else {
                    T = fed->requestTime(runToTime);
                    captureForCurrentTime(T);
                }

                if (T >= runToTime) {
                    break;
                }
                if ((T >= nextPrintTime) && (nextPrintTimeStep > timeZero)) {
                    std::cout << "processed for time " << static_cast<double>(T) << "\n";
                    nextPrintTime += nextPrintTimeStep;
                }
            }
        }
        catch (...) {
        }
    }
    /** add a subscription to record*/
    void Clone::addSubscription(const std::string& key)
    {
        auto res = subkeys.find(key);
        if ((res == subkeys.end()) || (res->second == -1)) {
            subscriptions.emplace_back(fed->registerSubscription(key));
            auto index = static_cast<int>(subscriptions.size()) - 1;
            auto id = subscriptions.back().getHandle();
            subids[id] = index;  // this is a new element
            subkeys[key] = index;  // this is a potential replacement
        }
    }

    void Clone::addSourceEndpointClone(const std::string& sourceEndpoint)
    {
        if (!cFilt) {
            cFilt = std::make_unique<CloningFilter>(fed.get());
            cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
            cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
        }
        cFilt->addSourceTarget(sourceEndpoint);
    }

    void Clone::setFederateToClone(const std::string& federateName)
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

        app->add_option("--output,-o", outFileName, "the output file for recording the data", true);
        app->add_option("capture", captureFederate, "name of the federate to clone");

        return app;
    }

}  // namespace apps
}  // namespace helics
