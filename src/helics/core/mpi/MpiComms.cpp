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
MpiComms::MpiComms ()
{
}

MpiComms::MpiComms(const int &brokerRank)
    : brokerRank(brokerRank)
{
}

/** destructor*/
MpiComms::~MpiComms () { disconnect (); }

bool MpiComms::initMPI()
{
    // Initialize MPI with MPI_THREAD_SERIALIZED
    int mpi_initialized;
    int mpi_thread_level;

    MPI_Initialized(&mpi_initialized);

    if (!mpi_initialized)
        MPI_Init_thread(nullptr, nullptr, MPI_THREAD_SERIALIZED, &mpi_thread_level);

    if (!mpi_initialized)
    {
        std::cerr << "MPI initialization failed" << std::endl;
        return false;
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
    std::lock_guard<std::mutex> lock(mpiSerialMutex);
    MPI_Send(message.data(), message.size(), MPI_CHAR, dest, tag, comm);
}

std::vector<char> MpiComms::serializeReceiveMPI(int src, int tag, MPI_Comm comm) {
    std::lock_guard<std::mutex> lock(mpiSerialMutex);
    int recv_size;
    std::vector<char> buffer;
    MPI_Status status;
    MPI_Probe(src, tag, comm, &status);
    MPI_Get_count(&status, MPI_CHAR, &recv_size);
    buffer.resize(recv_size);
    MPI_Recv(buffer.data(), buffer.capacity(), MPI_CHAR, src, tag, comm, MPI_STATUS_IGNORE);
    return buffer;
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

ActionMessage MpiComms::generateReplyToIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.index)
        {
        case CLOSE_RECEIVER:
            return M;
        default:
            M.index = NULL_REPLY;
            return M;
        }
    }
    ActionCallback (std::move (M));
    ActionMessage resp (CMD_PRIORITY_ACK);
    return resp;
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
        auto len = 0; //socket.receive_from (boost::asio::buffer (data), remote_endp, 0, error);
        data = serializeReceiveMPI (MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD);
        if (error)
        {
            rx_status = connection_status::error;
            return;
        }
        if (len == 5)
        {
            std::string str (data.data (), len);
            if (str == "close")
            {
                goto CLOSE_RX_LOOP;
            }
        }
        ActionMessage M (data.data (), len);
        if (!isValidCommand (M))
        {
            std::cerr << "invalid command received mpi" << std::endl;
            continue;
        }
        if (isProtocolCommand (M))
        {
            if (M.index == CLOSE_RECEIVER)
            {
                goto CLOSE_RX_LOOP;
            }
        }
        if (isPriorityCommand (M))
        {
            auto reply = generateReplyToIncomingMessage (M);
            if (isProtocolCommand (reply))
            {
                if (reply.index == DISCONNECT)
                {
                    goto CLOSE_RX_LOOP;
                }
            }
            // Send using MPI
            //socket.send_to (boost::asio::buffer (reply.to_string ()), remote_endp, 0, ignored_error);
            serializeSendMPI(reply.to_vector(), brokerRank, 0, MPI_COMM_WORLD);
        }
        else
        {
            auto res = processIncomingMessage (M);
            if (res < 0)
            {
                goto CLOSE_RX_LOOP;
            }
        }
    }
CLOSE_RX_LOOP:
    // TODO tell tx loop to shut down
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
                //transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), broker_endpoint, 0, error);
                serializeSendMPI(cmd.to_vector(), brokerRank, 0, MPI_COMM_WORLD);
                if (error)
                {
                    std::cerr << "transmit failure to broker " << error.message () << '\n';
                }
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            // Send using MPI to ourself? Can it just get added directly to a receive queue?
            //transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), rxEndpoint, 0, error);
            serializeSendMPI(cmd.to_vector(), std::stoi(getAddress()), 0, MPI_COMM_WORLD);
            if (error)
            {
                std::cerr << "transmit failure to receiver " << error.message () << '\n';
            }
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                // Send using MPI to rank given by route
                //transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), rt_find->second, 0, error);
                serializeSendMPI(cmd.to_vector(), std::stoi(rt_find->second), 0, MPI_COMM_WORLD);
                if (error)
                {
                    std::cerr << "transmit failure to route to " << route_id << " " << error.message () << '\n';
                }
            }
            else
            {
                if (hasBroker)
                {
                    // Send using MPI to broker
                    //transmitSocket.send_to (boost::asio::buffer (cmd.to_string ()), broker_endpoint, 0, error);
                    serializeSendMPI(cmd.to_vector(), brokerRank, 0, MPI_COMM_WORLD);
                    if (error)
                    {
                        std::cerr << "transmit failure to broker " << error.message () << '\n';
                    }
                }
            }
        }
    }
CLOSE_TX_LOOP:
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        closeReceiver();
    }

    tx_status = connection_status::terminated;
}

void MpiComms::closeReceiver ()
{
    ActionMessage cmd(CMD_PROTOCOL);
    cmd.index = CLOSE_RECEIVER;
    // TODO somehow send message over to rx loop
}

std::string MpiComms::getAddress () const {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    return std::to_string(world_rank);
}

}  // namespace helics