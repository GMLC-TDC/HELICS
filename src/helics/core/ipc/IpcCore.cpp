/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcCore.h"
#include "../core-data.h"
#include "../core-exceptions.h"
#include "../core.h"
#include "../helics-time.h"
#include "helics/helics-config.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "IpcComms.h"

#include "../../common/argParser.h"
#include <boost/filesystem.hpp>

namespace helics
{
using namespace std::string_literals;
static const ArgDescriptors extraArgs{
  {"queueloc"s, "string"s, "the file location of the shared queue"s},
  {"broker,b"s, "string"s, "identifier for the broker"s},
  {"broker_address", "string"s, "location of the broker i.e network address"},
  {"broker_auto_start"s, "", "automatically start the broker"s},
  {"broker_init"s, "string"s,
   "the init string to pass to the broker upon startup-will only be used if the autostart is activated"s},
  {"brokername"s, "string"s, "identifier for the broker-same as broker"s},
  {"brokerinit"s, "string"s, "the initialization string for the broker"s}};

IpcCore::IpcCore () noexcept {}

IpcCore::IpcCore (const std::string &core_name) : CommsBroker (core_name) {}

IpcCore::~IpcCore () = default;

void IpcCore::initializeFromArgs (int argc, const char *const *argv)
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

        CommonCore::initializeFromArgs (argc, argv);
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

std::string IpcCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    return fileloc;
}

}  // namespace helics
