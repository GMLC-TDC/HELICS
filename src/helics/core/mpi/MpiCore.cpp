/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiCore.h"
#include "MpiComms.h"

#include "../../common/argParser.h"

#include <mpi.h>

namespace helics
{
MpiCore::MpiCore () noexcept {}

MpiCore::~MpiCore () = default;

MpiCore::MpiCore (const std::string &core_name) : CommsBroker (core_name) {}

using namespace std::string_literals;
static const ArgDescriptors extraArgs{ { "broker_address", ArgDescriptor::arg_type_t::string_type, "location of a broker using mpi (rank:tag)" } };

void MpiCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        variable_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count("broker_address") > 0)
        {
            brokerAddress = vm["broker_address"].as<std::string>();
        }
        else
        {
            brokerAddress = "0:0";
        }

        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool MpiCore::brokerConnect ()
{
    comms = std::make_unique<MpiComms> (brokerAddress);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());

    auto res = comms->connect ();
    return res;
}

std::string MpiCore::getAddress () const
{
    return comms->getAddress();
}

}  // namespace helics
