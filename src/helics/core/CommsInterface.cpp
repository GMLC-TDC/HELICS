/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "CommsInterface.hpp"
#include "NetworkBrokerData.hpp"

namespace helics
{
CommsInterface::CommsInterface (const std::string &localTarget, const std::string &brokerTarget)
    : localTarget_ (localTarget), brokerTarget_ (brokerTarget)
{
}

CommsInterface::CommsInterface (const NetworkBrokerData &netInfo)
    : localTarget_ (netInfo.localInterface), brokerTarget_ (netInfo.brokerAddress),
      brokerName_ (netInfo.brokerName)
{
}
/** destructor*/
CommsInterface::~CommsInterface ()
{
    std::lock_guard<std::mutex> syncLock (threadSyncLock);
    if (queue_watcher.joinable ())
    {
        queue_watcher.join ();
    }
    if (queue_transmitter.joinable ())
    {
        queue_transmitter.join ();
    }
}

void CommsInterface::transmit (int route_id, const ActionMessage &cmd)
{
    if (isPriorityCommand (cmd))
    {
        txQueue.emplacePriority (route_id, cmd);
    }
    else
    {
        txQueue.emplace (route_id, cmd);
    }
}

void CommsInterface::addRoute (int route_id, const std::string &routeInfo)
{
    ActionMessage rt (CMD_PROTOCOL_PRIORITY);
    rt.payload = routeInfo;
    rt.index = NEW_ROUTE;
    rt.dest_id = route_id;
    transmit (-1, rt);
}

bool CommsInterface::connect ()
{
    if (isConnected ())
    {
        return true;
    }
    if (rx_status != connection_status::startup)
    {
        return false;
    }
    if (tx_status != connection_status::startup)
    {
        return false;
    }
    // bool exp = false;
    if (!ActionCallback)
    {
        std::cerr << "no callback specified, the receiver cannot start\n";
        return false;
    }
    std::lock_guard<std::mutex> syncLock (threadSyncLock);
    if (name.empty ())
    {
        name = localTarget_;
    }
    if (localTarget_.empty ())
    {
        localTarget_ = name;
    }
    queue_watcher = std::thread ([this] { queue_rx_function (); });
    queue_transmitter = std::thread ([this] { queue_tx_function (); });
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    while (rx_status == connection_status::startup)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
    }
    while (tx_status == connection_status::startup)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
    }
    if (rx_status != connection_status::connected)
    {
        // std::cerr << "receiver connection failure" << std::endl;
        if (tx_status == connection_status::connected)
        {
            if (queue_transmitter.joinable ())
            {
                closeTransmitter ();
                queue_transmitter.join ();
            }
        }
        queue_watcher.join ();
        return false;
    }

    if (tx_status != connection_status::connected)
    {
        std::cerr << "transmitter connection failure" << std::endl;
        if (rx_status == connection_status::connected)
        {
            if (queue_watcher.joinable ())
            {
                closeReceiver ();
                queue_watcher.join ();
            }
        }
        queue_transmitter.join ();
        return false;
    }
    return true;
}

void CommsInterface::setName (const std::string &name_) { name = name_; }
void CommsInterface::disconnect ()
{
    if (tripDetector.isTripped ())
    {
        rx_status = connection_status::terminated;
        tx_status = connection_status::terminated;
        return;
    }
    if (rx_status.load () <= connection_status::connected)
    {
        closeReceiver ();
    }
    if (tx_status.load () <= connection_status::connected)
    {
        closeTransmitter ();
    }
    int cnt = 0;
    while (rx_status.load () <= connection_status::connected)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if ((cnt & 31) == 0)  // call this every 32*50 milliseconds
        {
            // try calling closeReceiver again
            closeReceiver ();
        }
        if (cnt == 400)  // Eventually give up
        {
            std::cerr << "unable to terminate connection\n";
            break;
        }
        // check the trip detector
        if (tripDetector.isTripped ())
        {
            rx_status = connection_status::terminated;
            tx_status = connection_status::terminated;
            return;
        }
    }
    cnt = 0;
    while (tx_status.load () <= connection_status::connected)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if ((cnt & 31) == 0)
        {
            // try calling closeTransmitter again
            closeTransmitter ();
        }
        if (cnt == 400)
        {
            std::cerr << "unable to terminate connection\n";
            break;
        }
        // check the trip detector
        if (tripDetector.isTripped ())
        {
            rx_status = connection_status::terminated;
            tx_status = connection_status::terminated;
            return;
        }
    }
}

bool CommsInterface::reconnect ()
{
    rx_status = connection_status::reconnecting;
    tx_status = connection_status::reconnecting;
    reconnectReceiver ();
    reconnectTransmitter ();
    int cnt = 0;
    while (rx_status.load () == connection_status::reconnecting)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if (cnt == 400)  // Eventually give up
        {
            std::cerr << "unable to reconnect\n";
            break;
        }
    }
    cnt = 0;
    while (tx_status.load () == connection_status::reconnecting)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if (cnt == 400)
        {
            std::cerr << "unable to reconnect\n";
            break;
        }
    }

    return ((rx_status.load () == connection_status::connected) &&
            (tx_status.load () == connection_status::connected));
}

void CommsInterface::setCallback (std::function<void(ActionMessage &&)> callback)
{
    ActionCallback = std::move (callback);
}

void CommsInterface::setMessageSize (int maxMessageSize, int maxMessageCount)
{
    if (maxMessageSize > 0)
    {
        maxMessageSize_ = maxMessageSize;
    }
    if (maxMessageCount > 0)
    {
        maxMessageCount_ = maxMessageCount;
    }
}

bool CommsInterface::isConnected () const
{
    return ((tx_status == connection_status::connected) && (rx_status == connection_status::connected));
}

void CommsInterface::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void CommsInterface::reconnectTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = RECONNECT;
    transmit (-1, rt);
}

void CommsInterface::reconnectReceiver ()
{
    ActionMessage cmd (CMD_PROTOCOL);
    cmd.index = RECONNECT_RECEIVER;
    transmit (-1, cmd);
}

}  // namespace helics
