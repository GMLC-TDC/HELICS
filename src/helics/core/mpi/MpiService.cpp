/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "MpiService.h"

namespace helics {

MPI_Comm MpiService::mpiCommunicator = MPI_COMM_NULL;

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

MpiService::MpiService ()
    : commRank (-1)
{
    stop_service = false;
    startup_flag = false;
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
        startup_flag = true;
        service_thread = std::make_unique<std::thread> (&MpiService::serviceLoop, this);
        while (startup_flag && !stop_service);
    }
}

void MpiService::serviceLoop ()
{
    // Startup/teardown for MPI can all go in separate functions
    // For integrating helics MPI calls with existing MPI applications, user may want to call from their own thread

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

    // signal that we have finished starting
    startup_flag = false;

    std::cout << "Started MPI service loop for rank " << commRank << std::endl;


    while (!stop_service)
    {
        // send/receive MPI messages
        sendAndReceiveMessages ();
    }

    std::cout << "End receive loop for rank " << commRank << std::endl;


    // Post receives for any waiting sends
    int message_waiting = 1;
    MPI_Status status;
    while (message_waiting)
    {
        MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, mpiCommunicator, &message_waiting, &status);
        if (message_waiting)
        {
            std::cout << "Unfinished receive to rank " << commRank << std::endl;
            // Get the size of the message waiting to be received
            int recv_size;
            std::vector<char> buffer;
            MPI_Get_count (&status, MPI_CHAR, &recv_size);
            buffer.resize (recv_size);
            
            // Receive the message
            MPI_Recv (buffer.data (), buffer.capacity (), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, mpiCommunicator, &status);
        }
    }

    // Finalize MPI
    int mpi_initialized;
    MPI_Initialized(&mpi_initialized);
    
    // MPI may have already been finalized if HELICS is used by a program that also uses MPI
    if (mpi_initialized)
    {
        std::cout << "About to finalize MPI for rank " << commRank << std::endl;
        MPI_Finalize();
        std::cout << "MPI Finalized for rank " << commRank << std::endl;
    }
}

std::string MpiService::addMpiComms (MpiComms *comm)
{
    comms.push_back (comm);

    // If somehow this gets called while MPI is still initializing, wait until MPI initialization completes
    while (startup_flag && !stop_service);

    // return the rank:tag for the MpiComms object
    return std::to_string(commRank) + ":" + std::to_string(comms.size()-1);
}

std::string MpiService::getAddress (MpiComms *comm)
{
    for (unsigned int i = 0; i < comms.size (); i++)
    {
        if (comms[i] == comm)
        {
            // If somehow this gets called while MPI is still initializing, wait until MPI initialization completes
            while (startup_flag && !stop_service);

            return std::to_string(commRank) + ":" + std::to_string(i);
        }
    }

    // Comm not found
    return "";
}

int MpiService::getRank ()
{
    // If somehow this gets called while MPI is still initializing, wait until MPI initialization completes
    while (startup_flag && !stop_service);

    return commRank;
}


int MpiService::getTag (MpiComms *comm)
{
    for (unsigned int i = 0; i < comms.size (); i++)
    {
        if (comms[i] == comm)
        {
            return i;
        }
    }

    return -1;
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

void MpiService::sendAndReceiveMessages ()
{
    // Eventually this entire loop should be redone
    // Using fixed size chunks for sending messages would allow posting blocks of irecv requests
    // If we know that a message will get received, a blocking MPI_Wait_any could be used for send requests
    // Also, a method of doing time synchronization using MPI reductions should be added
    for (unsigned int i = 0; i < comms.size(); i++)
    {
        // Handle receives for the MpiComms object
        int message_waiting = 1;
        MPI_Status status;

        while (message_waiting && !stop_service)
        {
            // Check if there is a messages waiting
            MPI_Iprobe (MPI_ANY_SOURCE, i, mpiCommunicator, &message_waiting, &status);

            if (message_waiting)
            {
                // Get the size of the message waiting to be received
                int recv_size;
                std::vector<char> buffer;
                MPI_Get_count (&status, MPI_CHAR, &recv_size);
                buffer.resize (recv_size);

                // Post an asynchronous receive
                MPI_Request req;
                MPI_Irecv (buffer.data (), buffer.capacity (), MPI_CHAR, MPI_ANY_SOURCE, i, mpiCommunicator, &req);

                // Wait until the asynchronous receive request has finished
                int message_received = false;
                while (!message_received && !stop_service)
                {
                    MPI_Test (&req, &message_received, MPI_STATUS_IGNORE);
                }

                // Deserialized the received action message
                ActionMessage M (buffer);

                // Add to the received message queue for the MpiComms object
                comms[i]->getRxMessageQueue ().push (M);
            }
        }

        // Send messages for the MpiComms object
        auto sendMsg = comms[i]->getTxMessageQueue ().try_pop ();
        while (sendMsg && !stop_service)
        {
            std::vector<char> msg;
            std::string address;
            std::tie (address, msg) = sendMsg.value ();

            MPI_Request req;
            auto sendRequestData = std::make_pair (req, msg);

            int addr_delim_pos = address.find (":");
            int destRank = std::stoi (address.substr (0, addr_delim_pos));;
            int destTag = std::stoi (address.substr (addr_delim_pos+1, address.length ()));

            if (destRank != commRank)
            {
                // Send the message using asynchronous send
                MPI_Isend(sendRequestData.second.data (), sendRequestData.second.size (), MPI_CHAR, destRank, destTag, mpiCommunicator, &sendRequestData.first);
                send_requests.push_back (sendRequestData);
            }
            else
            {
                // Add the message directly to the destination rx queue (same process)
                ActionMessage M (msg);
                comms[destTag]->getRxMessageQueue ().push (M);
            }
            sendMsg = comms[i]->getTxMessageQueue ().try_pop ();
        }
    }

    send_requests.remove_if (
            [](std::pair<MPI_Request,std::vector<char>> req)
            {
                int send_finished;
                MPI_Test (&req.first, &send_finished, MPI_STATUS_IGNORE);

                if (send_finished == 1)
                {
                    // Any cleanup needed here? Freeing the vector or MPI_Request?
                }

                return send_finished == 1;
            });
}

} // namespace helics
