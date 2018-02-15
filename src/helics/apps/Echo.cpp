/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "Echo.hpp"
#include "PrecHelper.hpp"

#include <fstream>
#include <iostream>

#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"
#include "../core/helicsVersion.hpp"

#include "../common/JsonProcessingFunctions.hpp"

namespace filesystem = boost::filesystem;

namespace helics
{

static const ArgDescriptors InfoArgs{
    {"delay", "the delay with which the echo app will echo message" },
    {"stop", "the time to stop the echo"}
};

Echo::Echo (int argc, char *argv[])
{
    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs,"input");
    if (res == versionReturn)
    {
        std::cout << helics::helicsVersionString() << '\n';
    }
    if (res < 0)
    {
        deactivated = true;
        return;
    }

    FederateInfo fi ("echo");
    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<MessageFederate> (fi);
    
    loadArguments (vm_map);
}

Echo::Echo (const FederateInfo &fi) : fed (std::make_shared<MessageFederate> (fi))
{

}

Echo::Echo (std::shared_ptr<Core> core, const FederateInfo &fi)
    : fed (std::make_shared<MessageFederate> (std::move (core), fi))
{

}

Echo::Echo (const std::string &jsonString) : fed (std::make_shared<MessageFederate> (jsonString))
{
    loadFile (jsonString);
}

Echo::~Echo () = default;

void Echo::loadFile (const std::string &filename)
{

    auto ext = filesystem::path(filename).extension().string();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        loadJsonFile(filename);
    }
    else
    {
        //loadTextFile(filename);
    }

}


void Echo::initialize ()
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        fed->enterInitializationState ();
    }
}


/*run the Echo*/
void Echo::run ()
{
    run (stopTime);
    fed->finalize ();
}

void Echo::run (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state < Federate::op_states::execution)
    {
     
        fed->enterExecutionState ();
        
    }
    else if (state == Federate::op_states::finalize)
    {
        return;
    }
    auto ctime = fed->getCurrentTime();
    while (ctime < stopTime_input)
    {
        ctime = fed->requestTime(stopTime_input);
    }

}


void Echo::setEchoDelay(Time delay)
{
    std::lock_guard<std::mutex> lock(delayTimeLock);
    delayTime = delay;
}

void Echo::echoMessage(Endpoint *ept, Time currentTime)
{
    auto m = ept->getMessage();
    std::lock_guard<std::mutex> lock(delayTimeLock);
    while (m)
    {
        ept->send(m->original_source, m->data, currentTime + delayTime);
        m = ept->getMessage();
    }
}

void Echo::finalize()
{
    fed->finalize();
}

void Echo::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    endpoints.emplace_back (GLOBAL, fed.get (), endpointName, endpointType);
    endpoints.back().setCallback([this](Endpoint *ept, Time messageTime) {echoMessage(ept, messageTime); });
}

int Echo::loadArguments (boost::program_options::variables_map &vm_map)
{
    std::string file;
    if (vm_map.count("input") == 0)
    {
        if (!fileLoaded)
        {
            if (filesystem::exists("helics.json"))
            {
                file = "helics.json";
            }
        }
    }
    if (filesystem::exists(vm_map["input"].as<std::string>()))
    {
        file = vm_map["input"].as<std::string>();
    }
    if (!file.empty())
    {
        loadFile(file);
    }

    if (vm_map.count ("stop") > 0)
    {
        stopTime = loadTimeFromString(vm_map["stop"].as<std::string> ());
    }
    if (vm_map.count("delay") > 0)
    {
        std::lock_guard<std::mutex> lock(delayTimeLock);
        delayTime = loadTimeFromString(vm_map["delay"].as<std::string>());
    }
    return 0;
}



void Echo::loadJsonFile(const std::string &jsonFile)
{
    fed->registerInterfaces(jsonFile);

    auto eptCount = fed->getEndpointCount();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back(fed.get(), ii);
        endpoints.back().setCallback([this](Endpoint *ept, Time messageTime) {echoMessage(ept, messageTime); });
    }

    auto doc = loadJsonString(jsonFile);


    if (doc.isMember("echo"))
    {
        auto playerConfig = doc["echo"];
        if (playerConfig.isMember("stop"))
        {
            stopTime = loadJsonTime(playerConfig["stop"]);
        }
        if (playerConfig.isMember("separator"))
        {
            auto sep = playerConfig["separator"].asString();
            if (sep.size() > 0)
            {
                fed->setSeparator(sep[0]);
            }

        }
        if (playerConfig.isMember("file"))
        {
            if (playerConfig["file"].isArray())
            {
                for (decltype(playerConfig.size()) ii = 0; ii<playerConfig.size(); ++ii)
                {
                    loadFile(playerConfig["file"][ii].asString());
                }
            }
            else
            {
                loadFile(playerConfig["file"].asString());
            }
        }
        if (playerConfig.isMember("delay"))
        {
            std::lock_guard<std::mutex> lock(delayTimeLock);
            delayTime = loadJsonTime(playerConfig["local"]);
        }
    }
}

}  // namespace helics
