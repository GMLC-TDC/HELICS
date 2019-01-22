/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "CommonCore.hpp"
#include "CoreBroker.hpp"

#include "TimeoutMonitor.h"
#include "loggingHelper.hpp"

namespace helics
{
void TimeoutMonitor::tick (CommonCore *core)
{
    if (waitingForPingReply)
    {
        auto now = std::chrono::steady_clock::now ();
        if (now - lastParentPing > timeout)
        {
            // try to reset the connection to the broker
            // brokerReconnect()
            core->sendToLogger (core->global_broker_id_local, log_level::error, core->getIdentifier (),
                                "core lost connection with broker");
            core->sendErrorToFederates (-5);
            core->processDisconnect ();
            core->brokerState = BrokerBase::broker_state_t::errored;
            core->addActionMessage (CMD_STOP);
        }
        else
        {  // ping again
            ActionMessage png (CMD_PING);
            png.source_id = core->global_broker_id_local;
            png.dest_id = core->higher_broker_id;
            core->transmit (parent_route_id, png);
        }
    }
    else if ((core->isConnected ()) && (core->global_broker_id_local.isValid ()) &&
             (core->global_broker_id_local != parent_broker_id))
    {
        // if (allFedWaiting())
        //{
        if (core->higher_broker_id.isValid ())
        {
            ActionMessage png (CMD_PING);
            png.source_id = core->global_broker_id_local;
            png.dest_id = core->higher_broker_id;
            core->transmit (parent_route_id, png);
            lastParentPing = std::chrono::steady_clock::now ();
            waitingForPingReply = true;
        }
        //}
    }
    else if ((core->isConnected ()) &&
             ((!core->global_broker_id_local.isValid ()) || (core->global_broker_id_local == parent_broker_id)))
    {
        ActionMessage rsend (CMD_RESEND);
        rsend.messageID = static_cast<int32_t> (CMD_REG_BROKER);
        core->processCommand (std::move (rsend));
    }
    else if ((core->brokerState == BrokerBase::broker_state_t::terminated) ||
             (core->brokerState == BrokerBase::broker_state_t::errored))
    {
        if (waitingForConnection)
        {
            auto now = std::chrono::steady_clock::now ();
            if (now - startWaiting > timeout)
            {
                ActionMessage png (CMD_CHECK_CONNECTIONS);
                png.source_id = core->global_broker_id_local;
                core->addActionMessage (png);
            }
        }
        else
        {
            waitingForConnection = true;
            startWaiting = std::chrono::steady_clock::now ();
        }
    }
    else
    {
        if (waitingForConnection)
        {
            auto now = std::chrono::steady_clock::now ();
            if (now - startWaiting > timeout)
            {
                ActionMessage png (CMD_CHECK_CONNECTIONS);
                png.source_id = core->global_broker_id_local;
                core->addActionMessage (png);
            }
        }
        else
        {
            waitingForConnection = true;
            startWaiting = std::chrono::steady_clock::now ();
        }
    }
}

void TimeoutMonitor::tick (CoreBroker *brk)
{
    if (!brk->isRoot ())
    {
        if (waitingForPingReply)
        {
            auto now = std::chrono::steady_clock::now ();
            if (now - lastParentPing > timeout)
            {
                // try to reset the connection to the broker
                // brokerReconnect()
                brk->sendToLogger (brk->global_broker_id_local, log_level::error, brk->getIdentifier (),
                                   "core lost connection with broker");
                brk->sendErrorToImmediateBrokers (-5);
                brk->processDisconnect ();
                brk->brokerState = BrokerBase::broker_state_t::errored;
                brk->addActionMessage (CMD_STOP);
            }
            else
            {  // ping again
                ActionMessage png (CMD_PING);
                png.source_id = brk->global_broker_id_local;
                png.dest_id = brk->higher_broker_id;
                brk->transmit (parent_route_id, png);
            }
        }
        else if ((brk->isConnected ()) && (brk->global_broker_id_local.isValid ()) &&
                 (brk->global_broker_id_local != parent_broker_id))
        {
            // if (allFedWaiting())
            //{
            if (brk->higher_broker_id.isValid ())
            {
                ActionMessage png (CMD_PING);
                png.source_id = brk->global_broker_id_local;
                png.dest_id = brk->higher_broker_id;
                brk->transmit (parent_route_id, png);
                lastParentPing = std::chrono::steady_clock::now ();
                waitingForPingReply = true;
            }
            //}
        }
        else if ((brk->brokerState == BrokerBase::broker_state_t::terminated) ||
                 (brk->brokerState == BrokerBase::broker_state_t::errored))
        {
            if (waitingForConnection)
            {
                auto now = std::chrono::steady_clock::now ();
                if (now - startWaiting > timeout)
                {
                    ActionMessage png (CMD_CHECK_CONNECTIONS);
                    png.source_id = brk->global_broker_id_local;
                    brk->addActionMessage (png);
                }
            }
            else
            {
                waitingForConnection = true;
                startWaiting = std::chrono::steady_clock::now ();
            }
        }
        else
        {
            if (waitingForConnection)
            {
                auto now = std::chrono::steady_clock::now ();
                if (now - startWaiting > timeout)
                {
                    ActionMessage png (CMD_CHECK_CONNECTIONS);
                    png.source_id = brk->global_broker_id_local;
                    brk->addActionMessage (png);
                }
            }
            else
            {
                waitingForConnection = true;
                startWaiting = std::chrono::steady_clock::now ();
            }
        }
    }
}

void TimeoutMonitor::pingReply (const ActionMessage & /*cmd*/)
{
    waitingForPingReply = false;
    waitingForConnection = false;
}

}  // namespace helics
