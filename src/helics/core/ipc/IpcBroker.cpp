/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcBroker.h"
#include "helics/common/blocking_queue.h"
#include "helics/config.h"
#include "helics/core/core-data.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"

#include "IpcComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "helics/core/argParser.h"

#include <boost/filesystem.hpp>

constexpr size_t maxMessageSize = 16 * 1024;

constexpr size_t maxMessageCount = 256;

#define CLOSE_IPC 23425
namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs{{"queueloc"s, "string"s, "the named location of the shared queue"s},
                                      {"brokerinit"s, "string"s, "the initialization string for the broker"s}};

IpcBroker::IpcBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

IpcBroker::IpcBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

IpcBroker::~IpcBroker () {}

void IpcBroker::displayHelp (bool localOnly)
{
    std::cout << " Help for Interprocess Broker: \n";
    namespace po = boost::program_options;
    po::variables_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    if (!localOnly)
    {
        CoreBroker::displayHelp ();
    }
}

void IpcBroker::InitializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == broker_state_t::created)
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
        noAutomaticID = true;
        CoreBroker::InitializeFromArgs (argc, argv);
        if (getIdentifier ().empty ())
        {
            setIdentifier ("_ipc_broker");
        }
    }
}

bool IpcBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);  // mutex protecting the other information in the ipcBroker
    if (fileloc.empty ())
    {
        fileloc = getIdentifier () + "_queue.hqf";
    }

    if ((brokerloc.empty ()) && (brokername.empty ()))
    {
        setAsRoot ();
    }
    else if (brokerloc.empty ())
    {
        brokerloc = brokername + "_queue.hqf";
    }
    comms = std::make_unique<IpcComms> (fileloc, brokerloc);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setMessageSize (maxMessageSize, maxMessageCount);
    return comms->connect ();
}

std::string IpcBroker::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    return fileloc;
}

}  // namespace helics
