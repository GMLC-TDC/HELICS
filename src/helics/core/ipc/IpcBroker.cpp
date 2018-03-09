/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcBroker.h"
#include "../../common/blocking_queue.h"
#include "../Core.hpp"
#include "../core-data.hpp"
#include "../helics-time.hpp"
#include "helics/helics-config.h"

#include "IpcComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "../../common/argParser.h"

#include <boost/filesystem.hpp>

constexpr size_t maxMessageSize = 16 * 1024;

constexpr size_t maxMessageCount = 256;

#define CLOSE_IPC 23425
namespace helics
{
namespace ipc
{
using namespace std::string_literals;
static const ArgDescriptors extraArgs{ {"queueloc"s , "the named location of the shared queue"s},
                                      {"broker,b"s, "identifier for the broker"s},
                                      {"broker_address", "location of the broker i.e network address"},
                                      {"brokerinit"s, "the initialization string for the broker"s} };

IpcBroker::IpcBroker(bool rootBroker) noexcept : CommsBroker(rootBroker) {}

IpcBroker::IpcBroker(const std::string &broker_name) : CommsBroker(broker_name) {}

IpcBroker::~IpcBroker() = default;

void IpcBroker::displayHelp(bool local_only)
{
    std::cout << " Help for Interprocess Broker: \n";
    variable_map vm;
    const char *const argV[] = { "", "--help" };
    argumentParser(2, argV, vm, extraArgs);
    if (!local_only)
    {
        CoreBroker::displayHelp();
    }
}

void IpcBroker::initializeFromArgs(int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        variable_map vm;
        argumentParser(argc, argv, vm, extraArgs);

        if (vm.count("broker") > 0)
        {
            brokername = vm["broker"].as<std::string>();
        }

        if (vm.count("broker_address") > 0)
        {
            brokerloc = vm["broker_address"].as<std::string>();
        }

        if (vm.count("fileloc") > 0)
        {
            fileloc = vm["fileloc"].as<std::string>();
        }
        noAutomaticID = true;
        CoreBroker::initializeFromArgs(argc, argv);
        if (getIdentifier().empty())
        {
            setIdentifier("_ipc_broker");
        }
    }
}

bool IpcBroker::brokerConnect()
{
    std::lock_guard<std::mutex> lock(dataMutex);  // mutex protecting the other information in the ipcBroker
    if (fileloc.empty())
    {
        fileloc = getIdentifier() + "_queue.hqf";
    }

    if ((brokerloc.empty()) && (brokername.empty()))
    {
        setAsRoot();
    }
    else if (brokerloc.empty())
    {
        brokerloc = brokername + "_queue.hqf";
    }
    comms = std::make_unique<IpcComms>(fileloc, brokerloc);
    comms->setCallback([this](ActionMessage M) { addActionMessage(std::move(M)); });
    comms->setMessageSize(maxMessageSize, maxMessageCount);
    return comms->connect();
}

std::string IpcBroker::getAddress() const
{
    std::lock_guard<std::mutex> lock(dataMutex);
    return fileloc;
}

} // namespace ipc
}  // namespace helics

