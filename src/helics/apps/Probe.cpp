/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Probe.hpp"

#include "../application_api/queryFunctions.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"

#include <fmt/format.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace helics::apps {

Probe::Probe(int argc, char* argv[]): App("probe_${#}", argc, argv) {}

Probe::Probe(std::vector<std::string> args): App("probe_${#}", std::move(args)) {}

Probe::Probe(std::string_view appName, const FederateInfo& fi): App(appName, fi) {}

Probe::Probe(std::string_view appName, const std::shared_ptr<Core>& core, const FederateInfo& fi):
    App(appName, core, fi)
{
}

Probe::Probe(std::string_view appName, CoreApp& core, const FederateInfo& fi):
    App(appName, core, fi)
{
}

Probe::Probe(std::string_view name, const std::string& configString): App(name, configString) {}

void Probe::initialize()
{
    auto md = fed->getCurrentMode();
    if (md != Federate::Modes::STARTUP) {
        return;
    }
    auto period = fed->getTimeProperty(HELICS_PROPERTY_TIME_PERIOD);
    if (period <= Time::epsilon()) {
        auto td = fed->getTimeProperty(HELICS_PROPERTY_TIME_DELTA);
        if (td <= Time::epsilon()) {
            fed->setProperty(HELICS_PROPERTY_TIME_PERIOD, 1.0);
        }
    }
    if (stopTime == Time::maxVal()) {
        // use default stop time of 10s
        stopTime = 10.0;
    }
    endpoint = fed->registerEndpoint("probePoint", "probe");
    fed->enterInitializingModeIterative();
    auto qres = fed->query("root", "endpoints");
    auto epoints = vectorizeQueryResult(qres);

    const std::string& eptName = endpoint.getName();
    for (const auto& ept : epoints) {
        if (ept == eptName) {
            // do not connect to self
            continue;
        }
        if (ept.find("probe") == std::string::npos) {
            continue;
        }
        endpoint.addDestinationEndpoint(ept);
        ++connections;
    }
    fed->logInfoMessage(
        fmt::format("Probe {} connected to {} endpoints", fed->getName(), connections));
    fed->enterInitializingMode();
}

void Probe::runTo(Time stopTime_input)
{
    auto md = fed->getCurrentMode();
    if (md == Federate::Modes::STARTUP) {
        initialize();
    }

    if (md < Federate::Modes::EXECUTING) {
        fed->enterExecutingMode();
    }

    Time currentTime = fed->getCurrentTime();

    while (currentTime <= stopTime_input) {
        runProbe();
        currentTime = fed->requestNextStep();
    }
}

void Probe::runProbe()
{
    auto ctime = fed->getCurrentTime();
    while (endpoint.hasMessage()) {
        auto m = endpoint.getMessage();
        fed->logInfoMessage(
            fmt::format("Message from {} at Time {}: [{}]", m->source, ctime, m->to_string()));
        ++messagesReceived;
    }
    endpoint.send(fmt::format("message from {},time {}", fed->getName(), ctime));
}

}  // namespace helics::apps
