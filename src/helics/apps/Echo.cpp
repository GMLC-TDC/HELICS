/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Echo.hpp"
#include "PrecHelper.hpp"

#include <fstream>
#include <iostream>

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/argParser.h"
#include "../core/helicsVersion.hpp"
#include <set>
#include <stdexcept>

namespace helics
{
namespace apps
{
static const ArgDescriptors InfoArgs{{"delay", "the delay with which the echo app will echo message"}};

Echo::Echo (int argc, char *argv[]) : App ("echo", argc, argv)
{
    variable_map vm_map;
    if (!deactivated)
    {
        argumentParser (argc, argv, vm_map, InfoArgs);
        loadArguments (vm_map);
        if (!masterFileName.empty ())
        {
            loadFile (masterFileName);
        }
    }
    else
    {
        argumentParser (argc, argv, vm_map, InfoArgs);
    }
}

Echo::Echo (const std::string &name, const FederateInfo &fi) : App (name, fi) {}

Echo::Echo (const std::string &name, const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : App (name, core, fi)
{
}

Echo::Echo (const std::string &name, const std::string &jsonString) : App (name, jsonString)
{
    Echo::loadJsonFile (jsonString);
}

Echo::Echo (Echo &&other_echo)
    : App (std::move (other_echo)), endpoints (std::move (other_echo.endpoints)), delayTime (other_echo.delayTime)
{
}

Echo &Echo::operator= (Echo &&other_echo)
{
    endpoints = std::move (other_echo.endpoints);
    std::lock_guard<std::mutex> lock (delayTimeLock);
    delayTime = other_echo.delayTime;
    echoCounter = other_echo.echoCounter;
    App::operator= (std::move (other_echo));
    return *this;
}

void Echo::runTo (Time stopTime_input)
{
    auto md = fed->getCurrentMode ();
    if (md == Federate::modes::startup)
    {
        initialize ();
    }
    if (md < Federate::modes::executing)
    {
        fed->enterExecutingMode ();
    }
    else if (md == Federate::modes::finalize)
    {
        return;
    }
    auto ctime = fed->getCurrentTime ();
    while (ctime < stopTime_input)
    {
        ctime = fed->requestTime (stopTime_input);
    }
}

void Echo::setEchoDelay (Time delay)
{
    std::lock_guard<std::mutex> lock (delayTimeLock);
    delayTime = delay;
}

void Echo::echoMessage (const Endpoint &ept, Time currentTime)
{
    auto m = ept.getMessage ();
    std::lock_guard<std::mutex> lock (delayTimeLock);
    while (m)
    {
        ept.send (m->original_source, m->data, currentTime + delayTime);
        m = ept.getMessage ();
    }
}

void Echo::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    endpoints.emplace_back (fed->registerGlobalEndpoint (endpointName, endpointType));
    endpoints.back ().setCallback (
      [this](const Endpoint &ept, Time messageTime) { echoMessage (ept, messageTime); });
}

int Echo::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("delay") > 0)
    {
        std::lock_guard<std::mutex> lock (delayTimeLock);
        delayTime = loadTimeFromString (vm_map["delay"].as<std::string> ());
    }
    return 0;
}

void Echo::loadJsonFile (const std::string &jsonFile)
{
    loadJsonFileConfiguration ("echo", jsonFile);
    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed->getEndpoint (ii));
        endpoints.back ().setCallback (
          [this](const Endpoint &ept, Time messageTime) { echoMessage (ept, messageTime); });
    }

    auto doc = loadJson (jsonFile);

    if (doc.isMember ("echo"))
    {
        auto echoConfig = doc["echo"];

        if (echoConfig.isMember ("delay"))
        {
            std::lock_guard<std::mutex> lock (delayTimeLock);
            delayTime = loadJsonTime (echoConfig["delay"]);
        }
    }
}

}  // namespace apps
}  // namespace helics
