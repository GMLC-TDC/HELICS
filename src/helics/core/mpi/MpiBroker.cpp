/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiBroker.h"
#include "MpiComms.h"
#include "helics/common/blocking_queue.h"
#include "helics/config.h"
#include "helics/core/core-data.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "helics/core/argParser.h"

static const std::string DEFAULT_BROKER = "tcp://localhost:5555";

namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs{{"brokerinit"s, "string"s, "the initialization string for the broker"s}};

MpiBroker::MpiBroker (bool rootBroker) noexcept : CoreBroker (rootBroker) {}

MpiBroker::MpiBroker (const std::string &broker_name) : CoreBroker (broker_name) {}

MpiBroker::~MpiBroker ()
{
    haltOperations = true;
    joinAllThreads ();
}

void MpiBroker::InitializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == broker_state_t::created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count ("broker") > 0)
        {
            auto brstring = vm["broker"].as<std::string> ();
            // tbroker = findTestBroker(brstring);
        }

        if (vm.count ("brokerinit") > 0)
        {
            // tbroker->Initialize(vm["brokerinit"].as<std::string>());
        }
        CoreBroker::InitializeFromArgs (argc, argv);
    }
}

bool MpiBroker::brokerConnect ()
{
    comms = std::make_unique<MpiComms> ("", "");
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    // comms->setMessageSize(maxMessageSize, maxMessageCount);
    return comms->connect ();
}

void MpiBroker::brokerDisconnect () { comms->disconnect (); }

void MpiBroker::transmit (int route_id, const ActionMessage &cmd) { comms->transmit (route_id, cmd); }

void MpiBroker::addRoute (int route_id, const std::string &routeInfo) { comms->addRoute (route_id, routeInfo); }

std::string MpiBroker::getAddress () const { return ""; }
}  // namespace helics
