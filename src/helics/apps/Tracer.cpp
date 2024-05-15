/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Tracer.hpp"

#include "../application_api/Filters.hpp"
#include "../application_api/Subscriptions.hpp"
#include "../application_api/ValueFederate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace helics::apps {
Tracer::Tracer(std::string_view appName, FederateInfo& fedInfo): App(appName, fedInfo)
{
    fed->setFlagOption(HELICS_FLAG_OBSERVER);
}

Tracer::Tracer(std::vector<std::string> args): App("tracer", std::move(args))
{
    processArgs();
    initialSetup();
}

Tracer::Tracer(int argc, char* argv[]): App("tracer", argc, argv)
{
    processArgs();
    initialSetup();
}

void Tracer::processArgs()
{
    auto app = buildArgParserApp();
    if (!deactivated) {
        app->parse(remArgs);
    } else if (helpMode) {
        app->remove_helics_specifics();
        std::cout << app->help();
    }
}
Tracer::Tracer(std::string_view appName,
               const std::shared_ptr<Core>& core,
               const FederateInfo& fedInfo): App(appName, core, fedInfo)
{
    initialSetup();
}

Tracer::Tracer(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    App(appName, core, fedInfo)
{
    initialSetup();
}

Tracer::Tracer(std::string_view name, const std::string& configString): App(name, configString)
{
    processArgs();
    initialSetup();
}

Tracer::~Tracer() = default;

void Tracer::initialSetup()
{
    if (!deactivated) {
        fed->setFlagOption(HELICS_FLAG_OBSERVER);
        loadInputFiles();
    }
}

void Tracer::loadJsonFile(const std::string& jsonString, bool enableFederateInterfaceRegistration)
{
    loadJsonFileConfiguration("tracer", jsonString, enableFederateInterfaceRegistration);

    auto subCount = fed->getInputCount();
    for (int ii = 0; ii < subCount; ++ii) {
        subscriptions.emplace_back(fed->getInput(ii));
        subkeys.emplace(subscriptions.back().getName(), static_cast<int>(subscriptions.size()) - 1);
    }
    auto eptCount = fed->getEndpointCount();
    for (int ii = 0; ii < eptCount; ++ii) {
        endpoints.emplace_back(fed->getEndpoint(ii));
        eptNames[endpoints.back().getName()] = static_cast<int>(endpoints.size() - 1);
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

void Tracer::loadTextFile(const std::string& textFile)
{
    using namespace gmlc::utilities::stringOps;  // NOLINT
    App::loadTextFile(textFile);
    std::ifstream infile(textFile);
    std::string str;
    int lineCount = 0;
    while (std::getline(infile, str)) {
        ++lineCount;
        if (str.empty()) {
            continue;
        }
        auto firstChar = str.find_first_not_of(" \t\n\r\0");
        if ((firstChar == std::string::npos) || (str[firstChar] == '#') ||
            (str[firstChar] == '!')) {
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

void Tracer::initialize()
{
    auto state = fed->getCurrentMode();
    if (state == Federate::Modes::STARTUP) {
        generateInterfaces();

        fed->enterInitializingMode();
        captureForCurrentTime(-1.0);
    }
}

void Tracer::generateInterfaces()
{
    for (auto& tag : subkeys) {
        if (tag.second == -1) {
            addSubscription(tag.first);
        }
    }

    loadCaptureInterfaces();
}

void Tracer::loadCaptureInterfaces()
{
    for (auto& capt : captureInterfaces) {
        auto res = waitForInit(fed.get(), capt);
        if (res) {
            fed->query("root", "global_flush", HELICS_SEQUENCING_MODE_ORDERED);
            auto pubs = vectorizeQueryResult(
                fed->query(capt, "publications", HELICS_SEQUENCING_MODE_ORDERED));
            for (auto& pub : pubs) {
                addSubscription(pub);
            }
        }
    }
}

void Tracer::captureForCurrentTime(Time currentTime, int iteration)
{
    for (auto& sub : subscriptions) {
        if (sub.isUpdated()) {
            auto val = sub.getValue<std::string>();

            if (printMessage) {
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
                if (skiplog) {
                    std::cout << valstr << '\n';
                } else {
                    spdlog::info(valstr);
                }
            }
            if (valueCallback) {
                valueCallback(currentTime, sub.getTarget(), val);
            }
        }
    }

    for (auto& ept : endpoints) {
        while (ept.hasMessage()) {
            auto mess = ept.getMessage();
            if (printMessage) {
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
                if (skiplog) {
                    std::cout << messstr << '\n';
                } else {
                    spdlog::info(messstr);
                }
            }
            if (endpointMessageCallback) {
                endpointMessageCallback(currentTime, ept.getName(), std::move(mess));
            }
        }
    }

    // get the clone endpoints
    if (cloneEndpoint) {
        while (cloneEndpoint->hasMessage()) {
            auto mess = cloneEndpoint->getMessage();
            if (printMessage) {
                std::string messstr;
                if (mess->data.size() < 50) {
                    messstr = fmt::format("[{}]message from {} to {}::{}",
                                          static_cast<double>(currentTime),
                                          mess->source,
                                          mess->original_dest,
                                          mess->data.to_string());
                } else {
                    messstr = fmt::format("[{}]message from %s to %s:: size %d",
                                          static_cast<double>(currentTime),
                                          mess->source,
                                          mess->original_dest,
                                          mess->data.size());
                }
                if (skiplog) {
                    std::cout << messstr << '\n';
                } else {
                    spdlog::info(messstr);
                }
            }
            if (clonedMessageCallback) {
                clonedMessageCallback(currentTime, std::move(mess));
            }
        }
    }
}

/** run the Player until the specified time*/
void Tracer::runTo(Time runToTime)
{
    auto state = fed->getCurrentMode();
    if (state == Federate::Modes::STARTUP) {
        initialize();
        state = Federate::Modes::INITIALIZING;
    }

    if (state == Federate::Modes::INITIALIZING) {
        fed->enterExecutingMode();
        captureForCurrentTime(0.0);
    }

    Time nextPrintTime = 10.0;
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
            if (grantedTime >= nextPrintTime) {
                std::cout << "processed for time " << static_cast<double>(grantedTime) << "\n";
                nextPrintTime += 10.0;
            }
        }
    }
    catch (...) {
    }
}
/** add a subscription to record*/
void Tracer::addSubscription(std::string_view key)
{
    auto res = subkeys.find(key);
    if ((res == subkeys.end()) || (res->second == -1)) {
        subscriptions.push_back(fed->registerSubscription(key));
        auto index = static_cast<int>(subscriptions.size()) - 1;
        subkeys[subscriptions.back().getTarget()] = index;  // this is a potential replacement
    }
}

/** add an endpoint*/
void Tracer::addEndpoint(std::string_view endpoint)
{
    auto res = eptNames.find(endpoint);
    if ((res == eptNames.end()) || (res->second == -1)) {
        endpoints.emplace_back(InterfaceVisibility::GLOBAL, fed, endpoint);
        auto index = static_cast<int>(endpoints.size()) - 1;
        eptNames[endpoints.back().getName()] = index;  // this is a potential replacement
    }
}
void Tracer::addSourceEndpointClone(std::string_view sourceEndpoint)
{
    if (!cFilt) {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addSourceTarget(sourceEndpoint);
}

void Tracer::addDestEndpointClone(std::string_view destEndpoint)
{
    if (!cFilt) {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addDestinationTarget(destEndpoint);
}

void Tracer::addCapture(std::string_view captureDesc)
{
    captureInterfaces.emplace_back(captureDesc);
}

std::shared_ptr<helicsCLI11App> Tracer::buildArgParserApp()
{
    using gmlc::utilities::stringOps::removeQuotes;
    using gmlc::utilities::stringOps::splitlineQuotes;

    auto app = std::make_shared<helicsCLI11App>("Command line options for the Tracer App");
    app->add_flag("--allow_iteration", allow_iteration, "allow iteration on values")
        ->ignore_underscore();
    app->add_flag("--print", printMessage, "print messages to the screen");
    app->add_flag("--skiplog", skiplog, "print messages to the screen through cout");
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
