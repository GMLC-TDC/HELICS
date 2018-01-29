/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiBroker.h"
#include "MpiComms.h"

#include <mpi.h>

namespace helics
{
MpiBroker::MpiBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

MpiBroker::MpiBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

MpiBroker::~MpiBroker () = default;

void MpiBroker::displayHelp (bool local_only)
{
    std::cout << " Help for MPI Broker: \n";
    // TODO add displayHelp function for MPI, similar to NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void MpiBroker::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        // TODO parse args for rank of broker

        CoreBroker::initializeFromArgs (argc, argv);
    }
}

bool MpiBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (brokerRank == -1)
    {
        setAsRoot ();
    }
    comms = std::make_unique<MpiComms> (brokerRank);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());

    auto res = comms->connect ();
    return res;
}

std::string MpiBroker::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    return std::to_string(world_rank);
}
}  // namespace helics
