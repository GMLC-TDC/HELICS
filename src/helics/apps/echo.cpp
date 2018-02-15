/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "echo.h"
#include "PrecHelper.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.h"
#include "../common/JsonProcessingFunctions.hpp"

#include "../common/base64.h"
#include "../common/stringOps.h"

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
        std::cout << helics::versionString() << '\n';
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

    fed->registerInterfaces (filename);

    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed.get (), ii);
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
    if (state != Federate::op_states::execution)
    {
     
        fed->enterExecutionState ();
        // send the stuff at timeZero
        
    }
    else
    {
        //auto ctime = fed->getCurrentTime ();
       
    }

    helics::Time nextPrintTime = 10.0;
    bool moreToSend = true;
    Time nextSendTime = timeZero;
    while (moreToSend)
    {
        nextSendTime = Time::maxVal ();
      
        if (nextSendTime > stopTime_input)
        {
            break;
        }
        if (nextSendTime == Time::maxVal ())
        {
            moreToSend = false;
            continue;
        }
        auto newTime = fed->requestTime (nextSendTime);
        
        if (newTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (newTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
}


void Echo::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    endpoints.push_back (Endpoint (GLOBAL, fed.get (), endpointName, endpointType));
}

int Echo::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("input") == 0)
    {
        return (-1);
    }

    if (!filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        std::cerr << vm_map["input"].as<std::string> () << " does not exist \n";
        return -3;
    }
    loadFile (vm_map["input"].as<std::string> ());

    if (vm_map.count ("stop") > 0)
    {
        stopTime = loadTimeFromString(vm_map["stop"].as<std::string> ());
    }
    if (vm_map.count("delay") > 0)
    {
        delayTime = loadTimeFromString(vm_map["delay"].as<std::string>());
    }
    return 0;
}

}  // namespace helics
