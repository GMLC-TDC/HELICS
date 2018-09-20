/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../../flag-definitions.h"
#include "../ActionMessage.hpp"
#include "IpcComms.h"
#include "IpcQueueHelper.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "../ActionMessage.hpp"
#include "IpcComms.h"
#include "IpcQueueHelper.h"
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
namespace ipc
{

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
        err.messageID = ERROR_CODE_CONNECTION_FAILURE;
        err.payload = rxQueue.getError ();
        ActionCallback (std::move (err));
        setRxStatus (connection_status::error);  // the connection has failed
        rxQueue.changeState (queue_state_t::closing);
        return;
    }
    setRxStatus (connection_status::connected);  // this is a atomic indicator that the rx queue is ready
    bool IPCoperating = false;
    while (true)
    {
        auto bc = ipcbackchannel.load ();

        switch (bc)
        {
        case IPC_BACKCHANNEL_DISCONNECT:
            ipcbackchannel = 0;
            goto DISCONNECT_RX_QUEUE;
        case IPC_BACKCHANNEL_TRY_RESET:
            connected = rxQueue.connect (localTarget_, maxMessageCount_, maxMessageSize_);
            if (!connected)
            {
                disconnecting = true;
                ActionMessage err (CMD_ERROR);
                err.messageID = ERROR_CODE_CONNECTION_FAILURE;
                err.payload = rxQueue.getError ();
                ActionCallback (std::move (err));
                setRxStatus (connection_status::error);  // the connection has failed
                rxQueue.changeState (queue_state_t::closing);
                ipcbackchannel = 0;
                return;
            }
            ipcbackchannel = 0;
            break;
        }
        auto cmdopt = rxQueue.getMessage (2000);
        if (!cmdopt)
        {
            continue;
        }
        if (isProtocolCommand (*cmdopt))
        {
            if (cmdopt->messageID == CLOSE_RECEIVER)
            {
                disconnecting = true;
                break;
            }
            if (cmdopt->messageID == SET_TO_OPERATING)
            {
                if (!IPCoperating)
                {
                    rxQueue.changeState (queue_state_t::operating);
                    IPCoperating = true;
                }
            }
            continue;
        }
        if (cmdopt->action () == CMD_INIT_GRANT)
        {
            if (!IPCoperating)
            {
                rxQueue.changeState (queue_state_t::operating);
                IPCoperating = true;
            }
        }
        ActionCallback (std::move (*cmdopt));
    }
DISCONNECT_RX_QUEUE:
    try
    {
        rxQueue.changeState (queue_state_t::closing);
    }
    catch (boost::interprocess::interprocess_exception const &ipe)
    {
        std::cerr << "error changing states" << std::endl;
    }
    setRxStatus (connection_status::terminated);
}

void IpcComms::queue_tx_function ()
{
    sendToQueue brokerQueue;  //!< the queue of the broker
    sendToQueue rxQueue;
    std::map<int, sendToQueue> routes;  //!< table of the routes to other brokers
    bool hasBroker = false;

    if (!brokerTarget_.empty ())
    {
        bool conn = brokerQueue.connect (brokerTarget_, true, 20);
        if (!conn)
        {
            ActionMessage err (CMD_ERROR);
            err.messageID = ERROR_CODE_CONNECTION_FAILURE;
            err.payload =
              std::string ("Unable to open broker connection ->") + brokerQueue.getError ();
            ActionCallback (std::move (err));
            setTxStatus (connection_status::error);
            return;
        }
        hasBroker = true;
    }
    // wait for the receiver to startup
    if (!rxTrigger.wait_forActivation (std::chrono::milliseconds (3000)))
    {
        ActionMessage err (CMD_ERROR);
        err.messageID = ERROR_CODE_CONNECTION_FAILURE;
        err.payload = "Unable to link with receiver";
        ActionCallback (std::move (err));
        setTxStatus (connection_status::error);
        return;
    }
    if (getRxStatus () == connection_status::error)
    {
        setTxStatus (connection_status::error);
        return;
    }
    bool conn = rxQueue.connect (localTarget_, false, 0);
    if (!conn)
    {
        /** lets try a reset of the receiver*/
        ipcbackchannel = IPC_BACKCHANNEL_TRY_RESET;
        while (ipcbackchannel != 0)
        {
            if (getRxStatus () != connection_status::connected)
            {
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
        if (getRxStatus () == connection_status::connected)
        {
            conn = rxQueue.connect (localTarget_, false, 0);
        }
        if (!conn)
        {
            ActionMessage err (CMD_ERROR);
            err.messageID = ERROR_CODE_CONNECTION_FAILURE;
            err.payload = std::string ("Unable to open receiver connection ->") + brokerQueue.getError ();
            ActionCallback (std::move (err));
            setRxStatus (connection_status::error);
            return;
        }
    }

    setTxStatus (connection_status::connected);
    bool IPCoperating = false;
    while (true)
    {
        int route_id;
        ActionMessage cmd;
        std::tie (route_id, cmd) = txQueue.pop ();
        if (isProtocolCommand (cmd))
        {
            if (route_id == -1)
            {
                switch (cmd.messageID)
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
            if (!IPCoperating)
            {
                ActionMessage op (CMD_PROTOCOL);
                op.messageID = SET_TO_OPERATING;
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
    setTxStatus (connection_status::terminated);
}

void IpcComms::closeReceiver ()
{
    if ((getRxStatus () == connection_status::error) || (getRxStatus () == connection_status::terminated))
    {
        return;
    }
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.messageID = CLOSE_RECEIVER;
    if (getTxStatus () == connection_status::connected)
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
                ipcbackchannel.store (IPC_BACKCHANNEL_DISCONNECT);
                //  std::cerr << "unable to send close message::" << ipe.what () << std::endl;
            }
        }
    }
}

}  // namespace ipc
}  // namespace helics
