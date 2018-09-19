/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "IpcCore.h"
#include "../Core.hpp"
#include "../core-data.hpp"
#include "../core-exceptions.hpp"
#include "../helics-time.hpp"
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
namespace ipc
{
using namespace std::string_literals;
static const ArgDescriptors extraArgs{
  {"queueloc"s, "the file location of the shared queue"s},
  {"broker,b"s, "identifier for the broker"s},
  {"broker_address", "location of the broker i.e network address"},
  {"broker_auto_start"s, ArgDescriptor::arg_type_t::flag_type, "automatically start the broker"s},
  {"broker_init"s,
   "the init string to pass to the broker upon startup-will only be used if the autostart is activated"s},
  {"brokername"s, "identifier for the broker-same as broker"s},
  {"brokerinit"s, "the initialization string for the broker"s}};

IpcCore::IpcCore () noexcept {}

IpcCore::IpcCore (const std::string &core_name) : CommsBroker (core_name) {}

void IpcCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock (dataMutex);
        if (brokerState == created)
        {
        variable_map vm;
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
    comms->loadTargetInfo (fileloc, brokerloc);
    comms->setTimeout (networkTimeout);
    return comms->connect ();
}

std::string IpcCore::generateLocalAddressString () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    return fileloc;
}

}  // namespace ipc
}  // namespace helics
