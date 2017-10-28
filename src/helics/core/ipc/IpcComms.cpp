/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "IpcComms.h"
#include "IpcQueueHelper.h"
#include "helics/core/ActionMessage.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#define SET_TO_OPERATING 135111

using ipc_queue = boost::interprocess::message_queue;
using ipc_state = boost::interprocess::shared_memory_object;

namespace helics
{
IpcComms::IpcComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
}
/** destructor*/
IpcComms::~IpcComms () { disconnect (); }

void IpcComms::queue_rx_function ()
{
    ownedQueue rxQueue;
    bool connected = rxQueue.connect (localTarget_, maxMessageCount_, maxMessageSize_);
    if (!connected)
    {
        disconnecting = true;
        ActionMessage err (CMD_ERROR);
        err.payload = rxQueue.getError ();
        ActionCallback (std::move (err));
        rx_status = connection_status::error;  // the connection has failed
        rxQueue.changeState (queue_state_t::closing);
        return;
    }
    rx_status = connection_status::connected;  // this is a atomic indicator that the rx queue is ready
    bool operating = false;
    while (true)
    {
        ActionMessage cmd = rxQueue.getMessage ();
        if (isProtocolCommand(cmd))
        {
            if (cmd.index == CLOSE_RECEIVER)
            {
                disconnecting = true;
                break;
            }
            if (cmd.index == SET_TO_OPERATING)
            {
                if (!operating)
                {
                    rxQueue.changeState (queue_state_t::operating);
                    operating = true;
                }
            }
            continue;
        }
        if (cmd.action () == CMD_INIT_GRANT)
        {
            if (!operating)
            {
                rxQueue.changeState (queue_state_t::operating);
                operating = true;
            }
        }
        ActionCallback (std::move (cmd));
    }

    rxQueue.changeState (queue_state_t::closing);
    rx_status = connection_status::terminated;
}

void IpcComms::queue_tx_function ()
{
    sendToQueue brokerQueue;  //!< the queue of the broker
    sendToQueue rxQueue;
    std::map<int, sendToQueue> routes;  //!< table of the routes to other brokers
    bool hasBroker = false;

    int sleep_counter = 50;
    if (!brokerTarget_.empty ())
    {
        bool conn = brokerQueue.connect (brokerTarget_, true, 20);
        if (!conn)
        {
            ActionMessage err (CMD_ERROR);
            err.payload = std::string ("Unable to open broker connection ->") + brokerQueue.getError ();
            ActionCallback (std::move (err));
            tx_status = connection_status::error;
            return;
        }
        hasBroker = true;
    }
    sleep_counter = 50;
    // wait for the receiver to startup
    while (rx_status == connection_status::startup)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (sleep_counter));
        sleep_counter *= 2;
        if (sleep_counter > 1700)
        {
            ActionMessage err (CMD_ERROR);
            err.payload = "Unable to link with receiver";
            ActionCallback (std::move (err));
            tx_status = connection_status::error;
            return;
        }
    }
    if (rx_status == connection_status::error)
    {
        tx_status = connection_status::error;
        return;
    }
    bool conn = rxQueue.connect (localTarget_, false, 0);
    if (!conn)
    {
        ActionMessage err (CMD_ERROR);
        err.payload = std::string ("Unable to open receiver connection ->") + brokerQueue.getError ();
        ActionCallback (std::move (err));
        tx_status = connection_status::error;
        return;
    }

    tx_status = connection_status::connected;
    bool operating = false;
    while (true)
    {
        int route_id;
        ActionMessage cmd;
        std::tie (route_id, cmd) = txQueue.pop ();
        if (isProtocolCommand(cmd))
        {
            if (route_id == -1)
            {
                switch (cmd.index)
                {
                case NEW_ROUTE:
                {
                    sendToQueue newQueue;
                    bool newQconnected = newQueue.connect (cmd.payload, false, 3);
                    if (newQconnected)
                    {
                        routes.emplace (cmd.dest_id, std::move (newQueue));
                    }
                    continue;
                }
                case DISCONNECT:
                    goto DISCONNECT_TX_QUEUE;
                }
            }
        }
        if (cmd.action () == CMD_INIT_GRANT)
        {
            if (!operating)
            {
                ActionMessage op (CMD_PROTOCOL);
                op.index = SET_TO_OPERATING;
                rxQueue.sendMessage (op, 3);
            }
        }
        std::string buffer = cmd.to_string ();
        int priority = isPriorityCommand (cmd) ? 3 : 1;
        if (route_id == 0)
        {
            if (hasBroker)
            {
                brokerQueue.sendMessage (cmd, priority);
            }
        }
        else if (route_id == -1)
        {
            rxQueue.sendMessage (cmd, priority);
        }
        else
        {
            auto routeFnd = routes.find (route_id);
            if (routeFnd != routes.end ())
            {
                routeFnd->second.sendMessage (cmd, priority);
            }
            else
            {
                if (hasBroker)
                {
                    brokerQueue.sendMessage (cmd, priority);
                }
            }
        }
    }
DISCONNECT_TX_QUEUE:
    tx_status = connection_status::terminated;
}

void IpcComms::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void IpcComms::closeReceiver ()
{
    if ((rx_status == connection_status::error) || (rx_status == connection_status::terminated))
    {
        return;
    }
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.index = CLOSE_RECEIVER;
    if (tx_status == connection_status::connected)
    {
        transmit (-1, cmd);
    }
    else if (!disconnecting)
    {
        try
        {
            auto rxQueue = std::make_unique<ipc_queue> (boost::interprocess::open_only,
                                                        stringTranslateToCppName (localTarget_).c_str ());
            std::string buffer = cmd.to_string ();
            rxQueue->send (buffer.data (), buffer.size (), 3);
        }
        catch (boost::interprocess::interprocess_exception const &ipe)
        {
            if (!disconnecting)
            {
                std::cerr << "unable to send close message\n";
            }
        }
    }
}
}  // namespace helics