/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CommsInterface.h"

namespace helics
{
CommsInterface::CommsInterface (const std::string &localTarget, const std::string &brokerTarget)
    : localTarget_ (localTarget), brokerTarget_ (brokerTarget)
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

void CommsInterface::transmit (int route_id, const ActionMessage &cmd) { txQueue.emplace (route_id, cmd); }

void CommsInterface::addRoute (int route_id, const std::string &routeInfo)
{
    ActionMessage rt (CMD_PROTOCOL);
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
    if (rx_status == connection_status::connected)
    {
        closeReceiver ();
    }
    if (tx_status == connection_status::connected)
    {
        closeTransmitter ();
    }
    int cnt = 0;
    while (rx_status != connection_status::terminated)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if ((cnt & 31) == 0)  // call this every 32*50 seconds
        {
            // try calling closeReceiver again
            closeReceiver ();
        }
        if (cnt == 400)  // Eventually give up
        {
            std::cerr << "unable to terminate connection\n";
            break;
        }
    }
    cnt = 0;
    while (tx_status != connection_status::terminated)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        ++cnt;
        if ((cnt & 31) == 0)
        {
            // try calling closeReceiver again
            closeTransmitter ();
        }
        if (cnt == 400)
        {
            std::cerr << "unable to terminate connection\n";
            break;
        }
    }
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
}  // namespace helics
