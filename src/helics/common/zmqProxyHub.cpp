/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "zmqProxyHub.h"
#include <mutex>

std::vector<std::shared_ptr<zmqProxyHub>> zmqProxyHub::proxies;

std::mutex proxyCreationMutex;

std::shared_ptr<zmqProxyHub>
zmqProxyHub::getProxy (const std::string &proxyName, const std::string &pairType, const std::string &contextName)
{
    std::lock_guard<std::mutex> proxyLock (proxyCreationMutex);
    for (auto &rct : proxies)
    {
        if (rct->getName () == proxyName)
        {
            return rct;
        }
    }
    // if it doesn't find a matching name make a new one with the appropriate name
    auto newProxy = std::shared_ptr<zmqProxyHub> (new zmqProxyHub (proxyName, pairType, contextName));
    proxies.push_back (newProxy);
    return newProxy;
}

zmqProxyHub::~zmqProxyHub () { stopProxy (); }

void zmqProxyHub::startProxy ()
{
    proxyThread = std::thread (&zmqProxyHub::proxyLoop, this);
    int active = 0;
    controllerSocket->recv (&active, 4);
    if (active == 1)
    {
        proxyRunning = true;
    }
}

void zmqProxyHub::stopProxy ()
{
    if (proxyRunning)
    {
        controllerSocket->send ("TERMINATE", 9, 0);
        proxyThread.join ();
    }
    std::lock_guard<std::mutex> proxyLock (proxyCreationMutex);
    for (auto px = proxies.begin (); px != proxies.end (); ++px)
    {
        if ((*px)->name == name)
        {
            proxies.erase (px);
            break;
        }
    }
}

void zmqProxyHub::modifyIncomingConnection (socket_ops op, const std::string &connection)
{
    incoming.addOperation (op, connection);
}
void zmqProxyHub::modifyOutgoingConnection (socket_ops op, const std::string &connection)
{
    outgoing.addOperation (op, connection);
}

zmqProxyHub::zmqProxyHub (const std::string &proxyName, const std::string &pairtype, const std::string &context)
    : name (proxyName)
{
    contextManager = zmqContextManager::getContextPointer (context);
    if ((pairtype == "pubsub") || (pairtype == "sub") || (pairtype == "pub"))
    {
        incoming.type = zmq::socket_type::xsub;
        outgoing.type = zmq::socket_type::xpub;
    }
    else if ((pairtype == "router") || (pairtype == "dealer"))
    {
        incoming.type = zmq::socket_type::router;
        outgoing.type = zmq::socket_type::dealer;
    }
    else if ((pairtype == "pull") || (pairtype == "push") || (pairtype == "pushpull"))
    {
        incoming.type = zmq::socket_type::pull;
        outgoing.type = zmq::socket_type::push;
    }
    controllerSocket = std::make_unique<zmq::socket_t> (contextManager->getContext (), zmq::socket_type::pair);
    controllerSocket->bind (std::string ("inproc://proxy_" + name).c_str ());
}

void zmqProxyHub::proxyLoop ()
{
    zmq::socket_t inputSocket = incoming.makeSocket (contextManager->getContext ());
    zmq::socket_t outputSocket = outgoing.makeSocket (contextManager->getContext ());

    zmq::socket_t control (contextManager->getContext (), zmq::socket_type::pair);

    control.connect (std::string ("inproc://proxy_" + name).c_str ());

    int active = 1;
    control.send (&active, sizeof (int), 0);

    zmq::proxy_steerable (inputSocket, outputSocket, nullptr, control);

    // if we exit the proxy function we are no longer running
    proxyRunning = false;
}