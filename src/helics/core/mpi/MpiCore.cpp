/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MpiCore.h"
#include "MpiComms.h"

#include "../../common/argParser.h"

#include <mpi.h>

namespace helics
{
namespace mpi
{
MpiCore::MpiCore () noexcept {}

// MpiCore::~MpiCore () = default;
MpiCore::~MpiCore () { /*std::cout << "MpiCore destructor for " << MpiCore::getAddress () << std::endl;*/ }
MpiCore::MpiCore (const std::string &core_name) : CommsBroker (core_name) {}

using namespace std::string_literals;
static const ArgDescriptors extraArgs{
  {"broker_address", ArgDescriptor::arg_type_t::string_type, "location of a broker using mpi (rank:tag)"},
  {"broker_rank", ArgDescriptor::arg_type_t::int_type, "mpi rank of a broker using mpi"},
  {"broker_tag", ArgDescriptor::arg_type_t::int_type, "mpi tag of a broker using mpi"}};

void MpiCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        variable_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        brokerRank = 0;
        brokerTag = 0;

        if (vm.count ("broker_address") > 0)
        {
            auto addr = vm["broker_address"].as<std::string> ();
            auto delim_pos = addr.find_first_of (":", 1);
            brokerRank = std::stoi (addr.substr (0, delim_pos));
            brokerTag = std::stoi (addr.substr (delim_pos + 1, addr.length ()));
        }

        if (vm.count ("broker_rank") > 0)
        {
            brokerRank = vm["broker_rank"].as<int> ();
        }

        if (vm.count ("broker_tag") > 0)
        {
            brokerTag = vm["broker_tag"].as<int> ();
        }

        brokerAddress = std::to_string (brokerRank) + ":" + std::to_string (brokerTag);

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

std::string MpiCore::generateLocalAddressString () const
{
    return (comms) ? (comms->getAddress ()) : brokerAddress;
}

}  // namespace mpi
}  // namespace helics
