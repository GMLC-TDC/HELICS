/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiComms.h"
#include "MpiService.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include <memory>

namespace helics
{

MpiComms::MpiComms ()
    : shutdown (false)
{
    std::cout << "MpiComms()" << std::endl;

    auto& mpi_service = MpiService::getInstance ();
    commAddress = mpi_service.addMpiComms (this);
    std::cout << "- commAddress = " << commAddress << std::endl;
}

MpiComms::MpiComms(const std::string &broker)
    : brokerAddress(broker), shutdown (false)
{
    std::cout << "MpiComms(" << brokerAddress << ")" << std::endl;

    auto& mpi_service = MpiService::getInstance ();
    commAddress = mpi_service.addMpiComms (this);
    std::cout << "- commAddress = " << commAddress << std::endl;
}

/** destructor*/
MpiComms::~MpiComms ()
{
    std::cout << "Disconnecting MPIComms" << std::endl;
    disconnect ();
    std::cout << "Finished disconnect" << std::endl;
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
    rx_status = connection_status::connected;

    while (true)
    { 
        auto M = rxMessageQueue.try_pop ();

        if (M)
        {
            std::cout << "message received: " << prettyPrintString(M.value ()) << std::endl;
            if (!isValidCommand (M.value ()))
            {
                std::cerr << "invalid command received" << std::endl;
                continue;
            }

            if (isProtocolCommand (M.value ()))
            {
                if (M->index == CLOSE_RECEIVER)
                {
                    goto CLOSE_RX_LOOP;
                }
            }

            auto res = processIncomingMessage (M.value ());
            if (res < 0)
            {
                goto CLOSE_RX_LOOP;
            }
        }
        
        if (shutdown)
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
    tx_status = connection_status::connected;

    std::map<int, std::string> routes;  // for all the other possible routes

    if (brokerAddress != "")
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
                txMessageQueue.emplace (brokerAddress, cmd.to_vector ());
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            // Send to ourself -- may need command line option to enable for openmpi
            std::cout << "send msg to self" << prettyPrintString(cmd) << std::endl;
            rxMessageQueue.push (cmd);
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                // Send using MPI to rank given by route
                std::cout << "send msg to rt: " << prettyPrintString(cmd) << std::endl;
                txMessageQueue.emplace (rt_find->second, cmd.to_vector ());
            }
            else
            {
                if (hasBroker)
                {
                    // Send using MPI to broker
                    std::cout << "send msg to brkr: " << prettyPrintString(cmd) << std::endl;
                    txMessageQueue.emplace (brokerAddress, cmd.to_vector ());
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

}  // namespace helics
