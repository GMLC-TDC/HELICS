/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MpiComms.h"
//#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include "MpiService.h"
#include <iostream>
#include <map>
#include <memory>

namespace helics
{
namespace mpi
{
MpiComms::MpiComms ()
{
    auto &mpi_service = MpiService::getInstance ();
    localTarget_ = mpi_service.addMpiComms (this);
    std::cout << "MpiComms() - commAddress = " << localTarget_ << std::endl;
}

/** destructor*/
MpiComms::~MpiComms () { disconnect (); }

void MpiComms::setBrokerAddress (const std::string &address)
{
    if (propertyLock ())
    {
        brokerTarget_ = address;
        propertyUnLock ();
    }
}

int MpiComms::processIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
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
    setRxStatus (connection_status::connected);

    while (true)
    {
        auto M = rxMessageQueue.pop (std::chrono::milliseconds (2000));

        if (M)
        {
            if (!isValidCommand (M.value ()))
            {
                logError ("invalid command received");
                continue;
            }

            if (isProtocolCommand (M.value ()))
            {
                if (M->messageID == CLOSE_RECEIVER)
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
    std::cout << "Shutdown RX Loop for " << localTarget_ << std::endl;
    shutdown = true;
    setRxStatus (connection_status::terminated);
}

void MpiComms::queue_tx_function ()
{
    setTxStatus (connection_status::connected);

    auto &mpi_service = MpiService::getInstance ();

    std::map<route_id, std::pair<int, int>> routes;  // for all the other possible routes

    std::pair<int, int> brokerLocation;
    if (!brokerTarget_.empty ())
    {
        hasBroker = true;
        auto addr_delim_pos = brokerTarget_.find (":");
        brokerLocation.first = std::stoi (brokerTarget_.substr (0, addr_delim_pos));
        brokerLocation.second = std::stoi (brokerTarget_.substr (addr_delim_pos + 1, brokerTarget_.length ()));
    }

    while (true)
    {
        route_id rid;
        ActionMessage cmd;

        std::tie (rid, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (control_route == rid)
            {
                switch (cmd.messageID)
                {
                case NEW_ROUTE:
                {
                    // cmd.payload would be the MPI rank of the destination
                    std::pair<int, int> routeLoc;
                    auto addr_delim_pos = cmd.payload.find (":");
                    routeLoc.first = std::stoi (cmd.payload.substr (0, addr_delim_pos));
                    routeLoc.second = std::stoi (cmd.payload.substr (addr_delim_pos + 1, cmd.payload.length ()));

                    routes.emplace (route_id{cmd.getExtraData ()}, routeLoc);
                    processed = true;
                }
                break;
                case REMOVE_ROUTE:
                    routes.erase (route_id (cmd.getExtraData ()));
                    processed = true;
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

        if (rid == parent_route_id)
        {
            if (hasBroker)
            {
                // Send using MPI to broker
                // std::cout << "send msg to brkr rt: " << prettyPrintString(cmd) << std::endl;
                mpi_service.sendMessage (brokerLocation, cmd.to_vector ());
            }
        }
        else if (rid == control_route)
        {  // send to rx thread loop
            // Send to ourself -- may need command line option to enable for openmpi
            // std::cout << "send msg to self" << prettyPrintString(cmd) << std::endl;
            rxMessageQueue.push (cmd);
        }
        else
        {
            auto rt_find = routes.find (rid);
            if (rt_find != routes.end ())
            {
                // Send using MPI to rank given by route
                // std::cout << "send msg to rt: " << prettyPrintString(cmd) << std::endl;
                mpi_service.sendMessage (rt_find->second, cmd.to_vector ());
            }
            else
            {
                if (hasBroker)
                {
                    // Send using MPI to broker
                    // std::cout << "send msg to brkr: " << prettyPrintString(cmd) << std::endl;
                    mpi_service.sendMessage (brokerLocation, cmd.to_vector ());
                }
            }
        }
    }
CLOSE_TX_LOOP:
    std::cout << "Shutdown TX Loop for " << localTarget_ << std::endl;
    routes.clear ();
    if (getRxStatus () == connection_status::connected)
    {
        shutdown = true;
    }
    setTxStatus (connection_status::terminated);
    mpi_service.removeMpiComms (this);
}

void MpiComms::closeReceiver ()
{
    shutdown = true;
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    rxMessageQueue.push (cmd);
}
}  // namespace mpi
}  // namespace helics
