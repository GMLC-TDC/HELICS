/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once

#include "MpiService.h"

namespace helics {

std::atomic<int> MpiService::commRank = -1;
MPI_Comm MpiService::mpiCommunicator = MPI_COMM_NULL;
std::atomic<bool> MpiService::stop_service = false;

MpiService& MpiService::getInstance ()
{
    static MpiService instance;
    instance.startService ();
    return instance;
}

void MpiService::setMpiCommunicator (MPI_Comm communicator)
{
    mpiCommunicator = communicator;
}

MpiService::MpiService () : stop_service (false)
{
}

MpiService::~MpiService ()
{
    // Stop the service thread
    stop_service = true;
    if (service_thread->joinable ())
    {
        service_thread->join ();
    }
}

void MpiService::startService ()
{
    if (service_thread == nullptr)
    {
        service_thread = make_unique<std::thread> (&MpiService::serviceLoop, this);
    }
}

void MpiService::serviceLoop ()
{
    std::cout << "Starting MPI service loop" << std::endl;

    // Initialize MPI
    if (initMPI ())
    {
        // if mpiCommunicator isn't set by user, make it a duplicate of MPI_COMM_WORLD
        if (mpiCommunicator == MPI_COMM_NULL)
        {
            MPI_Comm_dup (MPI_COMM_WORLD, &mpiCommunicator);
        }

        // set commRank to our process rank
        MPI_Comm_rank (mpiCommunicator, &commRank);
    }

    while (!stop_service)
    {
        // send/receive MPI messages
    }

    // Finalize MPI
    int mpi_initialized;
    MPI_Initialized(&mpi_initialized);
    
    // MPI may have already been finalized if HELICS is used by a program that also uses MPI
    if (mpi_initialized)
    {
        std::cout << "Finalizing MPI" << std::endl;
        MPI_Finalize();
        std::cout << "MPI Finalized" << std::endl;
    }
}

int MpiService::getRank ()
{
    return commRank;
}

int MpiService::addMpiComms (MpiComms *comm)
{
    comms.push_back (comm);
    
    // return the tag for the MpiComms object
    return comms.size()-1;
}

void MpiService::removeMpiComms (MpiComms *comm)
{
    for (int i = 0; i < comms.size (); i++)
    {
        if (comms[i] == comm)
        {
            comms.erase(i);
            break;
        }
    }
}

void MpiService::getTag (MpiComms *comm)
{
    for (int i = 0; i < comms.size (); i++)
    {
        if (comms[i] == comm)
        {
            return i;
        }
    }

}

bool MpiService::initMPI()
{
    // Initialize MPI with MPI_THREAD_FUNNELED
    int mpi_initialized;
    int mpi_thread_level;

    MPI_Initialized(&mpi_initialized);

    if (!mpi_initialized)
    {
        MPI_Init_thread(nullptr, nullptr, MPI_THREAD_FUNNELED, &mpi_thread_level);

        MPI_Initialized(&mpi_initialized);
        if (!mpi_initialized)
        {
            std::cerr << "MPI initialization failed" << std::endl;
            return false;
        }
    }

    MPI_Query_thread(&mpi_thread_level);

    if (mpi_thread_level < MPI_THREAD_FUNNELED)
    {
        std::cerr << "MPI_THREAD_FUNNELED support required" << std::endl;
        return false;
    }

    return true;
}

} // namespace helics
