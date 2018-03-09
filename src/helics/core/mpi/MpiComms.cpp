/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MpiComms.h"
#include "MpiService.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include <memory>

namespace helics
{
namespace mpi
{
MpiComms::MpiComms ()
    : shutdown (false)
{
    auto& mpi_service = MpiService::getInstance ();
    commAddress = mpi_service.addMpiComms (this);
    std::cout << "MpiComms() - commAddress = " << commAddress << std::endl;
}

MpiComms::MpiComms(const std::string &broker)
    : brokerAddress(broker), shutdown (false)
{
    auto& mpi_service = MpiService::getInstance ();
    commAddress = mpi_service.addMpiComms (this);
    std::cout << "MpiComms(" << brokerAddress << ")" << " - commAddress " << commAddress << std::endl;
}

/** destructor*/
MpiComms::~MpiComms ()
{
    disconnect ();
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
    std::cout << "Shutdown RX Loop for " << commAddress << std::endl;
    shutdown = true;
    rx_status = connection_status::terminated;
}

void MpiComms::queue_tx_function ()
{
    tx_status = connection_status::connected;

    auto & mpi_service = MpiService::getInstance ();

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
                    rxMessageQueue.push (cmd);
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
                //std::cout << "send msg to brkr rt: " << prettyPrintString(cmd) << std::endl;
                mpi_service.sendMessage (brokerAddress, cmd.to_vector ());
            }
        }
        else if (route_id == -1)
        {  // send to rx thread loop
            // Send to ourself -- may need command line option to enable for openmpi
            //std::cout << "send msg to self" << prettyPrintString(cmd) << std::endl;
            rxMessageQueue.push (cmd);
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                // Send using MPI to rank given by route
                //std::cout << "send msg to rt: " << prettyPrintString(cmd) << std::endl;
                mpi_service.sendMessage (rt_find->second, cmd.to_vector ());
            }
            else
            {
                if (hasBroker)
                {
                    // Send using MPI to broker
                    //std::cout << "send msg to brkr: " << prettyPrintString(cmd) << std::endl;
                    mpi_service.sendMessage (brokerAddress, cmd.to_vector ());
                }
            }
        }
    }
CLOSE_TX_LOOP:
    std::cout << "Shutdown TX Loop for " << commAddress  << std::endl;
    routes.clear ();
    if (rx_status == connection_status::connected)
    {
        shutdown = true;
    }
    tx_status = connection_status::terminated;
    mpi_service.removeMpiComms (this);
}

void MpiComms::closeReceiver() {
    shutdown = true;
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.index = CLOSE_RECEIVER;
    rxMessageQueue.push (cmd);
}
} // namespace mpi
}  // namespace helics

