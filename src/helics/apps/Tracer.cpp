/*
Copyright (c) 2017-2021,
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
#include "../common/fmt_format.h"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
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

namespace helics {
namespace apps {
    Tracer::Tracer(const std::string& appName, FederateInfo& fi): App(appName, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Tracer::Tracer(std::vector<std::string> args): App("tracer", std::move(args)) { processArgs(); }

    Tracer::Tracer(int argc, char* argv[]): App("tracer", argc, argv) { processArgs(); }

    void Tracer::processArgs()
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
    Tracer::Tracer(const std::string& appName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Tracer::Tracer(const std::string& appName, CoreApp& core, const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_observer);
    }

    Tracer::Tracer(const std::string& name, const std::string& file): App(name, file)
    {
        fed->setFlagOption(helics_flag_observer);
        Tracer::loadJsonFile(file);
    }

    Tracer::~Tracer() = default;

    void Tracer::loadJsonFile(const std::string& jsonString)
    {
        loadJsonFileConfiguration("tracer", jsonString);

        auto subCount = fed->getInputCount();
        for (int ii = 0; ii < subCount; ++ii) {
            subscriptions.emplace_back(fed->getInput(ii));
            subkeys.emplace(subscriptions.back().getName(),
                            static_cast<int>(subscriptions.size()) - 1);
        }
        auto eptCount = fed->getEndpointCount();
        for (int ii = 0; ii < eptCount; ++ii) {
            endpoints.emplace_back(fed->getEndpoint(ii));
            eptNames[endpoints.back().getName()] = static_cast<int>(endpoints.size() - 1);
        }

        auto doc = loadJson(jsonString);

        auto tags = doc["tag"];
        if (tags.isArray()) {
            for (const auto& tag : tags) {
                addSubscription(tag.asString());
            }
        } else if (tags.isString()) {
            addSubscription(tags.asString());
        }
        auto sourceClone = doc["sourceclone"];
        if (sourceClone.isArray()) {
            for (const auto& sc : sourceClone) {
                addSourceEndpointClone(sc.asString());
            }
        } else if (sourceClone.isString()) {
            addSourceEndpointClone(sourceClone.asString());
        }
        auto destClone = doc["destclone"];
        if (destClone.isArray()) {
            for (const auto& dc : destClone) {
                addDestEndpointClone(dc.asString());
            }
        } else if (destClone.isString()) {
            addDestEndpointClone(destClone.asString());
        }
        auto clones = doc["clone"];
        if (clones.isArray()) {
            for (const auto& clone : clones) {
                addSourceEndpointClone(clone.asString());
                addDestEndpointClone(clone.asString());
            }
        } else if (clones.isString()) {
            addSourceEndpointClone(clones.asString());
            addDestEndpointClone(clones.asString());
        }
        auto captures = doc["capture"];
        if (captures.isArray()) {
            for (const auto& capture : captures) {
                addCapture(capture.asString());
            }
        } else if (captures.isString()) {
            addCapture(captures.asString());
        }
    }

    void Tracer::loadTextFile(const std::string& textFile)
    {
        using namespace gmlc::utilities::stringOps;  // NOLINT
        App::loadTextFile(textFile);
        std::ifstream infile(textFile);
        std::string str;
        int lc = 0;
        while (std::getline(infile, str)) {
            ++lc;
            if (str.empty()) {
                continue;
            }
            auto fc = str.find_first_not_of(" \t\n\r\0");
            if ((fc == std::string::npos) || (str[fc] == '#') || (str[fc] == '!')) {
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
                    } else if ((blk[0] == "sourceclone") || (blk[0] == "source") ||
                               (blk[0] == "src")) {
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
                        std::cerr << "Unable to process line " << lc << ':' << str << '\n';
                    }
                    break;
                case 3:
                    if (blk[0] == "clone") {
                        if ((blk[1] == "source") || (blk[1] == "src")) {
                            addSourceEndpointClone(removeQuotes(blk[2]));
                        } else if ((blk[1] == "dest") || (blk[1] == "destination")) {
                            addDestEndpointClone(removeQuotes(blk[2]));
                        } else {
                            std::cerr << "Unable to process line " << lc << ':' << str << '\n';
                        }
                    } else {
                        std::cerr << "Unable to process line " << lc << ':' << str << '\n';
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
        if (state == Federate::modes::startup) {
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
                fed->query("root", "global_flush", helics_sequencing_mode_ordered);
                auto pubs = vectorizeQueryResult(
                    fed->query(capt, "publications", helics_sequencing_mode_ordered));
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
                                              currentTime,
                                              mess->source,
                                              mess->dest,
                                              mess->data.to_string());
                    } else {
                        messstr = fmt::format("[{}]message from {} to {}:: size {}",
                                              currentTime,
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
                                              currentTime,
                                              mess->source,
                                              mess->original_dest,
                                              mess->data.to_string());
                    } else {
                        messstr = fmt::format("[{}]message from %s to %s:: size %d",
                                              currentTime,
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
        if (state == Federate::modes::startup) {
            initialize();
            state = Federate::modes::initializing;
        }

        if (state == Federate::modes::initializing) {
            fed->enterExecutingMode();
            captureForCurrentTime(0.0);
        }

        Time nextPrintTime = 10.0;
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
                if (T >= nextPrintTime) {
                    std::cout << "processed for time " << static_cast<double>(T) << "\n";
                    nextPrintTime += 10.0;
                }
            }
        }
        catch (...) {
        }
    }
    /** add a subscription to record*/
    void Tracer::addSubscription(const std::string& key)
    {
        auto res = subkeys.find(key);
        if ((res == subkeys.end()) || (res->second == -1)) {
            subscriptions.push_back(helics::make_subscription(*fed, key));
            auto index = static_cast<int>(subscriptions.size()) - 1;
            subkeys[key] = index;  // this is a potential replacement
        }
    }

    /** add an endpoint*/
    void Tracer::addEndpoint(const std::string& endpoint)
    {
        auto res = eptNames.find(endpoint);
        if ((res == eptNames.end()) || (res->second == -1)) {
            endpoints.emplace_back(GLOBAL, fed, endpoint);
            auto index = static_cast<int>(endpoints.size()) - 1;
            eptNames[endpoint] = index;  // this is a potential replacement
        }
    }
    void Tracer::addSourceEndpointClone(const std::string& sourceEndpoint)
    {
        if (!cFilt) {
            cFilt = std::make_unique<CloningFilter>(fed.get());
            cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
            cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
        }
        cFilt->addSourceTarget(sourceEndpoint);
    }

    void Tracer::addDestEndpointClone(const std::string& destEndpoint)
    {
        if (!cFilt) {
            cFilt = std::make_unique<CloningFilter>(fed.get());
            cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
            cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
        }
        cFilt->addDestinationTarget(destEndpoint);
    }

    void Tracer::addCapture(const std::string& captureDesc)
    {
        captureInterfaces.push_back(captureDesc);
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
        auto* clone_group = app->add_option_group(
            "cloning", "Options related to endpoint cloning operations and specifications");
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
            ->add_option(
                "--destclone",
                "existing endpoints to capture all packets with the specified endpoint as a "
                "destination, this argument may be specified multiple time")
            ->each([this](const std::string& clone) { addSourceEndpointClone(clone); })
            ->delimiter(',')
            ->ignore_underscore()
            ->type_size(-1);

        auto* capture_group = app->add_option_group(
            "capture_group", "Options related to capturing publications, endpoints, or federates");
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
            ->add_option(
                "--capture",
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

}  // namespace apps
}  // namespace helics
