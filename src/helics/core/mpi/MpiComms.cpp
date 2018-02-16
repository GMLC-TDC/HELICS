/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiComms.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include <memory>

#include <mpi.h>

namespace helics
{

bool MpiComms::mpiCommsExists = false;
std::mutex MpiComms::mpiSerialMutex;
int MpiComms::commRank = -1;

MpiComms::MpiComms ()
{
    if (mpiCommsExists)
    {
        std::cout << "WARNING: MPIComms object already created, unexpected results may occur" << std::endl;
    }
    std::cout << "MpiComms()" << std::endl;
    initMPI();
    mpiCommsExists = true;
}

MpiComms::MpiComms(const int &brokerRank)
    : brokerRank(brokerRank)
{
    if (mpiCommsExists)
    {
        std::cout << "WARNING: MPIComms object already created, unexpected results may occur" << std::endl;
    }
    std::cout << "MpiComms(" << std::to_string(brokerRank) << ")" << std::endl;
    initMPI();
    if (brokerRank == -1)
    {
        setBrokerRank (commRank);
    }
    mpiCommsExists = true;
}

/** destructor*/
MpiComms::~MpiComms ()
{
    std::cout << "Disconnecting MPIComms" << std::endl;
    disconnect ();
    std::cout << "Finished disconnect" << std::endl;

    int mpi_initialized;
    MPI_Initialized(&mpi_initialized);
    if (mpi_initialized)
    {
        std::cout << "Finalizing MPI" << std::endl;
        std::lock_guard<std::mutex> lock(mpiSerialMutex);
        MPI_Finalize();
        shutdown = true;
        std::cout << "MPI Finalized" << std::endl;
    }
}

bool MpiComms::initMPI()
{
    std::lock_guard<std::mutex> lock(mpiSerialMutex);

    // Initialize MPI with MPI_THREAD_SERIALIZED
    int mpi_initialized;
    int mpi_thread_level;

    MPI_Initialized(&mpi_initialized);

    if (!mpi_initialized)
    {
        MPI_Init_thread(nullptr, nullptr, MPI_THREAD_SERIALIZED, &mpi_thread_level);

        MPI_Initialized(&mpi_initialized);
        if (!mpi_initialized)
        {
            std::cerr << "MPI initialization failed" << std::endl;
            return false;
        }
    }

    MPI_Query_thread(&mpi_thread_level);

    if (mpi_thread_level < MPI_THREAD_SERIALIZED)
    {
        std::cerr << "MPI_THREAD_SERIALIZED support required" << std::endl;
        return false;
    }

    return true;
}

void MpiComms::serializeSendMPI(std::vector<char> message, int dest, int tag, MPI_Comm comm) {
    std::cout << "SendMPI from " << getAddress() << " to " << dest << std::endl;
    std::lock_guard<std::mutex> lock(mpiSerialMutex);
    MPI_Request req;
    MPI_Isend(message.data(), message.size(), MPI_CHAR, dest, tag, comm, &req);

    int message_sent = false;
    while (!message_sent)
    {
        MPI_Test(&req, &message_sent, MPI_STATUS_IGNORE);
    }
}

std::vector<char> MpiComms::serializeReceiveMPI(int src, int tag, MPI_Comm comm) {
    int message_waiting = false;
    MPI_Status status;

    while (!message_waiting && !shutdown)
    {
        std::lock_guard<std::mutex> probe_lock(mpiSerialMutex);
        if (!shutdown)
        {
            MPI_Iprobe(src, tag, comm, &message_waiting, &status);
        }

        if (message_waiting == true)
        {
            int recv_size;
            std::vector<char> buffer;
            MPI_Get_count(&status, MPI_CHAR, &recv_size);
            buffer.resize(recv_size);
            MPI_Request req;
            MPI_Irecv(buffer.data(), buffer.capacity(), MPI_CHAR, src, tag, comm, &req);

            int message_received = false;
            while (!message_received)
            {
                MPI_Test(&req, &message_received, MPI_STATUS_IGNORE);
            }
            return buffer;
        }
    }
    return std::vector<char>();
}

int MpiComms::processIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.index)
        {
        case CLOSE_RECEIVER:
            return (-1);
        default:
            break;
        }
    }
    ActionCallback (std::move (M));
    return 0;
}

void MpiComms::queue_rx_function ()
{
    bool isInitializedMPI = initMPI();
    if (!isInitializedMPI)
    {
        rx_status = connection_status::error;
        return;
    }
    rx_status = connection_status::connected;

    std::vector<char> data (10192);
    boost::system::error_code error;
    boost::system::error_code ignored_error;
    while (true)
    {
        // Receive using MPI
        data = serializeReceiveMPI (MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
        if (error)
        {
            rx_status = connection_status::error;
            return;
        }
        if (shutdown)
        {
            goto CLOSE_RX_LOOP;
        }

        ActionMessage M;
        M.from_vector (data);
        std::cout << "message received: " << prettyPrintString(M) << std::endl;
        if (!isValidCommand (M))
        {
            std::cerr << "invalid command received" << std::endl;
            continue;
        }
        if (isProtocolCommand (M))
        {
            if (M.index == CLOSE_RECEIVER)
            {
                goto CLOSE_RX_LOOP;
            }
        }
        auto res = processIncomingMessage (M);
        if (res < 0)
        {
            goto CLOSE_RX_LOOP;
        }
    }
CLOSE_RX_LOOP:
    std::cout << "Shutdown RX Loop" << std::endl;
    shutdown = true;
    rx_status = connection_status::terminated;
}

void MpiComms::queue_tx_function ()
{
    bool isInitializedMPI = initMPI();
    if (!isInitializedMPI)
    {
        tx_status = connection_status::error;
        return;
    }
    tx_status = connection_status::connected;

    std::vector<char> buffer;
    boost::system::error_code error;
    std::map<int, std::string> routes;  // for all the other possible routes

    if (brokerRank != -1)
    {
        hasBroker = true;
    }

    while (true)
    {
        int route_id;
        ActionMessage cmd;

        std::tie (route_id, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (route_id == -1)
            {
                switch (cmd.index)
                {
                case NEW_ROUTE:
                {
                    // cmd.payload would be the MPI rank of the destination
                    routes.emplace(cmd.dest_id, cmd.payload);
                    processed = true;
                }
                break;
                case DISCONNECT:
                    goto CLOSE_TX_LOOP;  // break out of loop
                }
            }
        }
        if (processed)
        {
            continue;
        }

        if (route_id == 0)
        {
            if (hasBroker)
            {
                // Send using MPI to broker
                std::cout << "send msg to brkr rt: " << prettyPrintString(cmd) << std::endl;
                serializeSendMPI(cmd.to_vector(), brokerRank, 0, MPI_COMM_WORLD);
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            // Send to ourself -- may need command line option to enable for openmpi
            std::cout << "send msg to self" << prettyPrintString(cmd) << std::endl;
            serializeSendMPI(cmd.to_vector(), commRank, 0, MPI_COMM_WORLD);
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                // Send using MPI to rank given by route
                std::cout << "send msg to rt: " << prettyPrintString(cmd) << std::endl;
                serializeSendMPI(cmd.to_vector(), std::stoi(rt_find->second), 0, MPI_COMM_WORLD);
            }
            else
            {
                if (hasBroker)
                {
                    // Send using MPI to broker
                    std::cout << "send msg to brkr: " << prettyPrintString(cmd) << std::endl;
                    serializeSendMPI(cmd.to_vector(), brokerRank, 0, MPI_COMM_WORLD);
                }
            }
        }
    }
CLOSE_TX_LOOP:
    std::cout << "Shutdown TX Loop" << std::endl;
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        shutdown = true;
    }

    tx_status = connection_status::terminated;
}

void MpiComms::closeReceiver() {
    shutdown = true;
    disconnect(); 
}

std::string MpiComms::getAddress () {
    if (commRank == -1)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
    }
    return std::to_string(commRank);
}

}  // namespace helics
