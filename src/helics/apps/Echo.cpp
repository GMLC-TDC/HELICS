/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Echo.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace helics {
namespace apps {
    Echo::Echo(std::vector<std::string> args): App("echo", std::move(args)) { processArgs(); }

    Echo::Echo(int argc, char* argv[]): App("echo", argc, argv) { processArgs(); }

    void Echo::processArgs()
    {
        helicsCLI11App app("Options specific to the Echo App");
        app.add_option("--delay", delayTime, "the delay with which the echo app will echo message");
        if (!deactivated) {
            fed->setFlagOption(helics_flag_event_triggered);
            app.parse(remArgs);
            if (!masterFileName.empty()) {
                loadFile(masterFileName);
            }
        } else if (helpMode) {
            app.remove_helics_specifics();
            std::cout << app.help();
        }
    }

    Echo::Echo(const std::string& name, const FederateInfo& fi): App(name, fi)
    {
        fed->setFlagOption(helics_flag_event_triggered);
    }

    Echo::Echo(const std::string& name, const std::shared_ptr<Core>& core, const FederateInfo& fi):
        App(name, core, fi)
    {
        fed->setFlagOption(helics_flag_event_triggered);
    }

    Echo::Echo(const std::string& name, CoreApp& core, const FederateInfo& fi): App(name, core, fi)
    {
        fed->setFlagOption(helics_flag_event_triggered);
    }

    Echo::Echo(const std::string& name, const std::string& jsonString): App(name, jsonString)
    {
        fed->setFlagOption(helics_flag_event_triggered);
        Echo::loadJsonFile(jsonString);
    }

    Echo::Echo(Echo&& other_echo) noexcept:
        App(std::move(other_echo)), endpoints(std::move(other_echo.endpoints)),
        delayTime(other_echo.delayTime)
    {
    }

    Echo& Echo::operator=(Echo&& other_echo) noexcept
    {
        endpoints = std::move(other_echo.endpoints);
        std::lock_guard<std::mutex> lock(delayTimeLock);
        delayTime = other_echo.delayTime;
        echoCounter = other_echo.echoCounter;
        App::operator=(std::move(other_echo));
        return *this;
    }

    void Echo::runTo(Time stopTime_input)
    {
        auto md = fed->getCurrentMode();
        if (md == Federate::modes::startup) {
            initialize();
        }
        if (md < Federate::modes::executing) {
            fed->enterExecutingMode();
        } else if (md == Federate::modes::finalize) {
            return;
        }
        auto ctime = fed->getCurrentTime();
        while (ctime < stopTime_input) {
            ctime = fed->requestTime(stopTime_input);
        }
    }

    void Echo::setEchoDelay(Time delay)
    {
        std::lock_guard<std::mutex> lock(delayTimeLock);
        delayTime = delay;
    }

    void Echo::echoMessage(const Endpoint& ept, Time currentTime)
    {
        auto m = ept.getMessage();
        std::lock_guard<std::mutex> lock(delayTimeLock);
        while (m) {
            ept.send(m->original_source, m->data, currentTime + delayTime);
            m = ept.getMessage();
        }
    }

    void Echo::addEndpoint(const std::string& endpointName, const std::string& endpointType)
    {
        endpoints.emplace_back(fed->registerGlobalEndpoint(endpointName, endpointType));
        endpoints.back().setCallback(
            [this](const Endpoint& ept, Time messageTime) { echoMessage(ept, messageTime); });
    }

    void Echo::loadJsonFile(const std::string& jsonFile)
    {
        loadJsonFileConfiguration("echo", jsonFile);
        auto eptCount = fed->getEndpointCount();
        for (int ii = 0; ii < eptCount; ++ii) {
            endpoints.emplace_back(fed->getEndpoint(ii));
            endpoints.back().setCallback(
                [this](const Endpoint& ept, Time messageTime) { echoMessage(ept, messageTime); });
        }

        auto doc = loadJson(jsonFile);

        if (doc.isMember("echo")) {
            auto echoConfig = doc["echo"];

            if (echoConfig.isMember("delay")) {
                std::lock_guard<std::mutex> lock(delayTimeLock);
                delayTime = loadJsonTime(echoConfig["delay"]);
            }
        }
    }

}  // namespace apps
}  // namespace helics
