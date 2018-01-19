/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was modified by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute;
the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence
Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
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

#include "zmqReactor.h"
#include "zmqContextManager.h"

std::vector<std::shared_ptr<zmqReactor>> zmqReactor::reactors;

using namespace zmq;

static std::mutex reactorCreationLock;

zmqReactor::zmqReactor (const std::string &reactorName, const std::string &context) : name (reactorName)
{
    contextManager = zmqContextManager::getContextPointer (context);
    notifier = std::make_unique<socket_t> (contextManager->getContext (), socket_type::pair);
    std::string constring = "inproc://reactor_" + name;
    notifier->bind (constring.c_str ());

    loopThread = std::thread (&zmqReactor::reactorLoop, this);
}

static const int zero (0);

zmqReactor::~zmqReactor () { terminateReactor (); }

void zmqReactor::terminateReactor ()
{
    if (reactorLoopRunning)
    {
        updates.emplace (reactorInstruction::terminate, std::string ());
        notifier->send (&zero, sizeof (int));
        loopThread.join ();
    }
    std::lock_guard<std::mutex> creationLock (reactorCreationLock);
    for (auto rct = reactors.begin (); rct != reactors.end (); ++rct)
    {
        if ((*rct)->name == name)
        {
            reactors.erase (rct);
            break;
        }
    }
}

std::shared_ptr<zmqReactor>
zmqReactor::getReactorInstance (const std::string &reactorName, const std::string &contextName)
{
    std::lock_guard<std::mutex> creationLock (reactorCreationLock);
    for (auto &rct : reactors)
    {
        if (rct->getName () == reactorName)
        {
            return rct;
        }
    }
    // if it doesn't find a matching name make a new one with the appropriate name
    // can't use make_shared since constructor is private
    auto newReactor = std::shared_ptr<zmqReactor> (new zmqReactor (reactorName, contextName));
    reactors.push_back (newReactor);
    return newReactor;
}

void zmqReactor::addSocket (const zmqSocketDescriptor &desc)
{
    updates.emplace (reactorInstruction::newSocket, desc);
    notifier->send (&zero, sizeof (int));
}

void zmqReactor::modifySocket (const zmqSocketDescriptor &desc)
{
    updates.emplace (reactorInstruction::modify, desc);
    notifier->send (&zero, sizeof (int));
}

void zmqReactor::closeSocket (const std::string &socketName)
{
    updates.emplace (reactorInstruction::close, socketName);
    notifier->send (&zero, sizeof (int));
}

void zmqReactor::addSocketBlocking (const zmqSocketDescriptor &desc)
{
    unsigned int one (1);
    updates.emplace (reactorInstruction::newSocket, desc);
    notifier->send (&one, sizeof (int));
    notifier->recv (&one, 4, 0);
}

void zmqReactor::modifySocketBlocking (const zmqSocketDescriptor &desc)
{
    unsigned int one (1);

    updates.emplace (reactorInstruction::modify, desc);
    notifier->send (&one, sizeof (int));

    notifier->recv (&one, 4, 0);
}

void zmqReactor::closeSocketBlocking (const std::string &socketName)
{
    unsigned int one (1);
    updates.emplace (reactorInstruction::close, socketName);
    notifier->send (&one, sizeof (int));
    notifier->recv (&one, 4, 0);
}

// this is not a member function but a helper function for the reactorLoop
auto findSocketByName (const std::string &socketName, const std::vector<std::string> &socketNames)
{
    int index = 0;
    for (auto &nm : socketNames)
    {
        if (nm == socketName)
        {
            return index;
        }
        ++index;
    }
    return index;
}

void zmqReactor::reactorLoop ()
{
    // make the signaling socket
    // use mostly local variables
    std::vector<socket_t> sockets;
    unsigned int messageCode = 0;

    sockets.emplace_back (contextManager->getContext (), socket_type::pair);
    sockets[0].connect (std::string ("inproc://reactor_" + name).c_str ());

    std::vector<zmq_pollitem_t> socketPolls;
    socketPolls.reserve (4);
    socketPolls.resize (1);
    std::vector<std::string> socketNames{name};
    std::vector<std::function<void(zmq::multipart_t res)>> socketCallbacks (1);
    int socketCount = 1;

    socketPolls[0].socket = sockets[0];
    socketPolls[0].fd = 0;
    socketPolls[0].events = ZMQ_POLLIN;
    socketPolls[0].revents = 0;
    reactorLoopRunning = true;
    while (true)
    {
        int val = poll (socketPolls);
        if (val > 0)
        {
            // do the callbacks for any socket with a message received
            for (int kk = 1; kk < socketCount; ++kk)
            {
                if ((socketPolls[kk].revents & ZMQ_POLLIN) != 0)
                {
                    socketCallbacks[kk](multipart_t (sockets[kk]));
                }
            }
            // deal with any socket updates as triggered by a message on socket 0
            if ((socketPolls[0].revents & ZMQ_POLLIN) != 0)
            {
                sockets[0].recv (&messageCode, sizeof (unsigned int), 0);  // clear the message
                auto socketop = updates.pop ();
                while (socketop)
                {
                    int index;
                    auto instruction = (*socketop).first;
                    auto &descriptor = (*socketop).second;
                    switch (instruction)
                    {
                    case reactorInstruction::close:
                        index = findSocketByName (descriptor.name, socketNames);
                        if (index < static_cast<int> (sockets.size ()))
                        {
                            sockets[index].close ();
                            sockets.erase (sockets.begin () + index);
                            socketNames.erase (socketNames.begin () + index);
                            socketCallbacks.erase (socketCallbacks.begin () + index);
                            socketPolls.erase (socketPolls.begin () + index);
                            --socketCount;
                        }
                        break;
                    case reactorInstruction::newSocket:
                        sockets.push_back (descriptor.makeSocket (contextManager->getContext ()));
                        socketNames.push_back (descriptor.name);
                        socketCallbacks.emplace_back (descriptor.callback);
                        socketPolls.resize (socketPolls.size () + 1);
                        socketPolls.back ().socket = sockets.back ();
                        socketPolls.back ().fd = 0;
                        socketPolls.back ().events = ZMQ_POLLIN;
                        socketPolls.back ().revents = 0;
                        ++socketCount;
                        break;
                    case reactorInstruction::modify:
                        index = findSocketByName (descriptor.name, socketNames);
                        if (index < static_cast<int> (sockets.size ()))
                        {
                            descriptor.modifySocket (sockets[index]);
                        }
                        // replace the callback if needed
                        if (descriptor.callback)
                        {
                            socketCallbacks[index] = descriptor.callback;
                        }
                        break;
                    case reactorInstruction::terminate:
                        // jumpt out of the loop
                        goto REACTOR_HALT;
                    default:
                        break;
                    }
                    socketop = updates.pop ();
                }
                if (messageCode > 0)
                {
                    sockets[0].send (&messageCode, sizeof (unsigned int), 0);
                }
            }
        }
    }
REACTOR_HALT:
    reactorLoopRunning = false;
}
