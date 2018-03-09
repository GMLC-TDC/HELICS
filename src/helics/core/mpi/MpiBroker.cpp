/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MpiBroker.h"
#include "MpiComms.h"

#include "../../common/argParser.h"

#include <mpi.h>

namespace helics
{
namespace mpi
{
MpiBroker::MpiBroker(bool rootBroker) noexcept : CommsBroker(rootBroker) {}

MpiBroker::MpiBroker(const std::string &broker_name) : CommsBroker(broker_name) {}

//MpiBroker::~MpiBroker() = default;
MpiBroker::~MpiBroker() { std::cout << "MpiBroker destructor for " << getAddress() << std::endl; }

using namespace std::string_literals;
static const ArgDescriptors extraArgs{
    { "broker_address", ArgDescriptor::arg_type_t::string_type, "location of a broker using mpi (rank:tag)" },
    { "broker_rank", ArgDescriptor::arg_type_t::int_type, "mpi rank of a broker using mpi"},
    { "broker_tag", ArgDescriptor::arg_type_t::int_type, "mpi tag of a broker using mpi"}
};


void MpiBroker::displayHelp(bool local_only)
{
    std::cout << " Help for MPI Broker: \n";
    variable_map vm;
    const char *const argV[] = { "", "--help" };
    argumentParser(2, argV, vm, extraArgs);
    if (!local_only)
    {
        CoreBroker::displayHelp();
    }
}

void MpiBroker::initializeFromArgs(int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        variable_map vm;
        argumentParser(argc, argv, vm, extraArgs);

        if (vm.count("broker_address") > 0)
        {
            auto addr = vm["broker_address"].as<std::string>();
            auto delim_pos = addr.find_first_of(":", 1);

            brokerRank = std::stoi(addr.substr(0, delim_pos));
            brokerTag = std::stoi(addr.substr(delim_pos + 1, addr.length()));
            brokerAddress = addr;
        }
        else if ((vm.count("broker_rank") > 0) || (vm.count("broker_tag") > 0))
        {
            brokerRank = 0;
            brokerTag = 0;

            if (vm.count("broker_rank") > 0)
            {
                brokerRank = vm["broker_rank"].as<int>();
            }

            if (vm.count("broker_tag") > 0)
            {
                brokerTag = vm["broker_tag"].as<int>();
            }
        }

        CoreBroker::initializeFromArgs(argc, argv);
    }
}

bool MpiBroker::brokerConnect()
{
    comms = std::make_unique<MpiComms>(brokerAddress);
    comms->setCallback([this](ActionMessage M) { addActionMessage(std::move(M)); });
    comms->setName(getIdentifier());

    if (brokerAddress == "")
    {
        setAsRoot();
    }

    auto res = comms->connect();
    return res;
}

std::string MpiBroker::getAddress() const
{
    return comms->getAddress();
}
} // namespace mpi
}  // namespace helics

