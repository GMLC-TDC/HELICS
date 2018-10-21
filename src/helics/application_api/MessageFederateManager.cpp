/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageFederateManager.hpp"
#include "../core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "../core/queryHelpers.hpp"
#include <cassert>

namespace helics
{
MessageFederateManager::MessageFederateManager (Core *coreOb, federate_id_t id)
    : coreObject (coreOb), fedID (id)
{
}
MessageFederateManager::~MessageFederateManager () = default;

void MessageFederateManager::disconnect ()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = nullptr;
}
Endpoint & MessageFederateManager::registerEndpoint (const std::string &name, const std::string &type)
{
    auto handle = coreObject->registerEndpoint (fedID, name, type);
    auto eptHandle = local_endpoints.lock ();
    return *(eptHandle->insert (name, handle, name, handle));
}

void MessageFederateManager::registerKnownCommunicationPath (Endpoint &localEndpoint,
                                                             const std::string &remoteEndpoint)
{
    coreObject->registerFrequentCommunicationsPair (localEndpoint.getName(),
                                                        remoteEndpoint);
}

void MessageFederateManager::subscribe (Endpoint &ept, const std::string &name)
{
   coreObject->addSourceTarget (ept.handle, name);
}

bool MessageFederateManager::hasMessage () const
{
    std::lock_guard<std::mutex> lock (endpointLock);
    for (auto &mq : messageQueues)
    {
        if (!mq.empty ())
        {
            return true;
        }
    }
    return false;
}

bool MessageFederateManager::hasMessage (const Endpoint &ept) const
{
    return (id.value () < endpointCount) ? (!messageQueues[id.value ()].empty ()) : false;
}

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederateManager::pendingMessages (const Endpoint &ept) const
{
    return (id.value () < endpointCount) ? (messageQueues[id.value ()].size ()) : 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multi-threaded contexts due to the required locking
prefer to just use getMessage until it returns an invalid Message.
*/
uint64_t MessageFederateManager::pendingMessages () const
{
    uint64_t sz = 0;
    for (auto &mq : messageQueues)
    {
        sz += mq.size ();
    }
    return sz;
}

std::unique_ptr<Message> MessageFederateManager::getMessage (Endpoint &ept)
{
    if (endpoint.value () < endpointCount)
    {
        auto mv = messageQueues[endpoint.value ()].pop ();
        if (mv)
        {
            return std::move (*mv);
        }
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederateManager::getMessage ()
{
    // just start with the first endpoint and check until a queue isn't empty
    for (auto &mq : messageQueues)
    {
        if (!mq.empty ())
        {
            auto ms = mq.pop ();
            if (ms)
            {
                return std::move (*ms);
            }
        }
    }
    return nullptr;
}

void MessageFederateManager::sendMessage (Endpoint &source, const std::string &dest, data_view message)
{
    if (source.value () < endpointCount)
    {
        coreObject->send ((*local_endpoints.lock_shared ())[source.value ()].handle, dest, message.data (),
                          message.size ());
    }
    else
    {
        throw (InvalidIdentifier("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (Endpoint &source,
                                          const std::string &dest,
                                          data_view message,
                                          Time sendTime)
{
    if (source.value () < endpointCount)
    {
        coreObject->sendEvent (sendTime, (*local_endpoints.lock_shared ())[source.value ()].handle, dest,
                               message.data (), message.size ());
    }
    else
    {
        throw (InvalidIdentifier("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (Endpoint &source, std::unique_ptr<Message> message)
{
    if (source.value () < endpointCount)
    {
        coreObject->sendMessage ((*local_endpoints.lock_shared ())[source.value ()].handle, std::move (message));
    }
    else
    {
        throw (InvalidIdentifier ("endpoint id is invalid"));
    }
}

void MessageFederateManager::updateTime (Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto epCount = coreObject->receiveCountAny (fedID);
    // lock the data updates
    std::unique_lock<std::mutex> eplock (endpointLock);

    interface_handle endpoint_id;
    for (size_t ii = 0; ii < epCount; ++ii)
    {
        auto message = coreObject->receiveAny (fedID, endpoint_id);
        if (!message)
        {
            break;
        }

        /** find the id*/
        auto epts = local_endpoints.lock();
        auto fid = epts->find (endpoint_id);
        if (fid != epts->end())
        {  // assign the data

            auto localEndpointIndex = fid->id.value ();
            messageQueues[localEndpointIndex].emplace (std::move (message));
            if (fid->callbackIndex >= 0)
            {
                // need to be copied otherwise there is a potential race condition on lock removal
                auto cb = callbacks[fid->callbackIndex];
                eplock.unlock ();
                cb (fid->id, CurrentTime);
                eplock.lock ();
            }
            else if (allCallbackIndex >= 0)
            {
                // need to be copied otherwise there is a potential race condition on lock removal
                auto ac = callbacks[allCallbackIndex];
                eplock.unlock ();
                ac (fid->id, CurrentTime);
                eplock.lock ();
            }
        }
    }
}

void MessageFederateManager::startupToInitializeStateTransition () { messageQueues.resize (endpointCount); }

void MessageFederateManager::initializeToExecuteStateTransition () {}


std::string MessageFederateManager::localQuery(const std::string &queryStr) const
{
    std::string ret;
    if (queryStr == "endpoints")
    {
        ret=generateStringVector_if(local_endpoints.lock_shared(), [](const auto &info) { return info.name; },
            [](const auto &info) {
            return (!info.name.empty());
        });
    }
    return ret;
}

static const std::string nullStr;

const std::string &MessageFederateManager::getEndpointName (const Endpoint &ept) const
{
    return ept.actualName;
}

Endpoint &MessageFederateManager::getEndpoint (const std::string &name) const
{
    auto sharedEpt = local_endpoints.lock_shared ();
    auto ept= sharedEpt->find (name);
    return (ept != sharedEpt.end()) ? *ept : Endpoint();
}

const std::string &MessageFederateManager::getEndpointType (const Endpoint &ept) const
{
    return coreObject->getType(ept.handle);
}

int MessageFederateManager::getEndpointCount () const { return static_cast<int>(local_endpoints.lock_shared ()->size ()); }

void MessageFederateManager::setEndpointOption(Endpoint &ept, int32_t option, bool option_value)
{
	coreObject->setHandleOption(ept.handle, option, option_value);
}


void MessageFederateManager::addSourceFilter (Endpoint &ept, const std::string &filterName)
{
    coreObject->addSourceTarget (ept.handle, filterName);
 }

/** add a named filter to an endpoint for all message going to the endpoint*/
 void MessageFederateManager::addDestinationFilter (Endpoint &ept, const std::string &filterName)
 {
     coreObject->addDestinationTarget (ept.handle, filterName);
 }

void MessageFederateManager::registerCallback (const std::function<void(Endpoint &, Time)> &callback)
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    if (allCallbackIndex < 0)
    {
        allCallbackIndex = static_cast<int> (callbacks.size ());
        callbacks.push_back (callback);
    }
    else
    {
        callbacks[allCallbackIndex] = callback;
    }
}

void MessageFederateManager::registerCallback (Endpoint &ept,
                                               const std::function<void(Endpoint &, Time)> &callback)
{
        auto eplock = local_endpoints.lock ();
		if (eplock)
		{
            (*eplock)[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
            callbacks.push_back (callback);
		}
      
}

void MessageFederateManager::removeOrderedMessage (unsigned int index)
{
    auto handle = messageOrder.lock ();
    if (index == handle->back ())
    {
        handle->pop_back ();
    }
    else
    {
        auto term = handle->rend ();
        for (auto ri = handle->rbegin () + 1; ri != term; ++ri)
        {
            if (*ri == index)
            {
                handle->erase (ri.base ());
                break;
            }
        }
    }
}
}  // namespace helics
