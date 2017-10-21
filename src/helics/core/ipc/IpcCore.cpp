/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcCore.h"
#include "helics/config.h"
#include "helics/core/core-data.h"
#include "helics/core/core-exceptions.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "IpcComms.h"

#include "helics/core/argParser.h"
#include <boost/filesystem.hpp>

namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs{
  {"queueloc"s, "string"s, "the file location of the shared queue"s},
  {"broker_auto_start"s, "", "automatically start the broker"s},
  {"broker_init"s, "string"s,
   "the init string to pass to the broker upon startup-will only be used if the autostart is activated"s},
  {"brokername"s, "string"s, "identifier for the broker-same as broker"s},
  {"brokerinit"s, "string"s, "the initialization string for the broker"s}};

IpcCore::IpcCore () noexcept {}

IpcCore::IpcCore (const std::string &core_name) : CommonCore (core_name) {}

IpcCore::~IpcCore ()
{
    haltOperations = true;
    std::unique_lock<std::mutex> lock (dataMutex);
    comms = nullptr;  // need to ensure the comms are deleted before the callbacks become invalid
    lock.unlock ();
    joinAllThreads ();
}

void IpcCore::InitializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count ("broker") > 0)
        {
            brokername = vm["broker"].as<std::string> ();
        }

        if (vm.count ("broker_address") > 0)
        {
            brokerloc = vm["broker_address"].as<std::string> ();
        }

        if (vm.count ("fileloc") > 0)
        {
            fileloc = vm["fileloc"].as<std::string> ();
        }

        CommonCore::InitializeFromArgs (argc, argv);
    }
}

bool IpcCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (fileloc.empty ())
    {
        fileloc = getIdentifier () + "_queue.hqf";
    }

    if (brokerloc.empty ())
    {
        if (brokername.empty ())
        {
            brokername = "_ipc_broker";
        }
        brokerloc = brokername + "_queue.hqf";
    }

    comms = std::make_unique<IpcComms> (fileloc, brokerloc);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    return comms->connect ();
}

void IpcCore::brokerDisconnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        comms->disconnect ();
    }
}

void IpcCore::transmit (int route_id, const ActionMessage &cmd)
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        comms->transmit (route_id, cmd);
    }
}

void IpcCore::addRoute (int route_id, const std::string &routeInfo)
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        comms->addRoute (route_id, routeInfo);
    }
}

std::string IpcCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    return fileloc;
}

}  // namespace helics
