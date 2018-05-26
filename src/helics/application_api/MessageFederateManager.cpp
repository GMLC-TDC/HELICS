/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageFederateManager.hpp"
#include "../core/Core.hpp"
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
endpoint_id_t MessageFederateManager::registerEndpoint (const std::string &name, const std::string &type)
{
    auto handle = coreObject->registerEndpoint (fedID, name, type);
    auto eptHandle = local_endpoints.lock ();
    endpoint_id_t id = static_cast<identifier_type> (eptHandle->size ());
    ++endpointCount;
    eptHandle->insert (name, handle, name, type, id, handle);

    return id;
}

void MessageFederateManager::registerKnownCommunicationPath (endpoint_id_t localEndpoint,
                                                             const std::string &remoteEndpoint)
{
    auto sharedElock = local_endpoints.lock_shared ();
    if (localEndpoint.value () < endpointCount)
    {
        coreObject->registerFrequentCommunicationsPair ((*sharedElock)[localEndpoint.value ()]->name,
                                                        remoteEndpoint);
    }
}

void MessageFederateManager::subscribe (endpoint_id_t endpoint, const std::string &name, const std::string &type)
{
    if (endpoint.value () < endpointCount)
    {
        auto h = coreObject->registerSubscription (fedID, name, type, std::string (), handle_check_mode::optional);
        std::lock_guard<std::mutex> eLock (endpointLock);
        subHandleLookup.emplace (h, std::make_pair (endpoint, name));
        hasSubscriptions = true;
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

bool MessageFederateManager::hasMessage () const
{
    for (auto &mq : messageQueues)
    {
        if (!mq.empty ())
        {
            return true;
        }
    }
    return false;
}

bool MessageFederateManager::hasMessage (endpoint_id_t id) const
{
    return (id.value () < endpointCount) ? (!messageQueues[id.value ()].empty ()) : false;
}

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederateManager::receiveCount (endpoint_id_t id) const
{
    return (id.value () < endpointCount) ? (messageQueues[id.value ()].size ()) : 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multi-threaded contexts due to the required locking
prefer to just use getMessage until it returns an invalid Message.
*/
uint64_t MessageFederateManager::receiveCount () const
{
    uint64_t sz = 0;
    for (auto &mq : messageQueues)
    {
        sz += mq.size ();
    }
    return sz;
}

std::unique_ptr<Message> MessageFederateManager::getMessage (endpoint_id_t endpoint)
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

void MessageFederateManager::sendMessage (endpoint_id_t source, const std::string &dest, data_view message)
{
    if (source.value () < endpointCount)
    {
        coreObject->send ((*local_endpoints.lock_shared ())[source.value ()]->handle, dest, message.data (),
                          message.size ());
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (endpoint_id_t source,
                                          const std::string &dest,
                                          data_view message,
                                          Time sendTime)
{
    if (source.value () < endpointCount)
    {
        coreObject->sendEvent (sendTime, (*local_endpoints.lock_shared ())[source.value ()]->handle, dest,
                               message.data (), message.size ());
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (endpoint_id_t source, std::unique_ptr<Message> message)
{
    if (source.value () < endpointCount)
    {
        coreObject->sendMessage ((*local_endpoints.lock_shared ())[source.value ()]->handle, std::move (message));
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::updateTime (Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto epCount = coreObject->receiveCountAny (fedID);
    // lock the data updates
    std::unique_lock<std::mutex> eplock (endpointLock);

    handle_id_t endpoint_id;
    for (size_t ii = 0; ii < epCount; ++ii)
    {
        auto message = coreObject->receiveAny (fedID, endpoint_id);
        if (!message)
        {
            break;
        }

        /** find the id*/
        auto fid = (local_endpoints.lock ())->find (endpoint_id);
        if (fid != nullptr)
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
    if (hasSubscriptions)
    {
        auto handles = coreObject->getValueUpdates (fedID);
        for (auto handle : handles)
        {
            auto sfnd = subHandleLookup.find (handle);
            if (sfnd != subHandleLookup.end ())
            {
                auto mv = std::make_unique<Message> ();
                mv->source = sfnd->second.second;
                auto localEndpointIndex = sfnd->second.first.value ();
                auto eptInfo = (*local_endpoints.lock ())[localEndpointIndex];
                mv->dest = eptInfo->name;
                mv->original_source = mv->source;
                // get the data value
                auto data = coreObject->getValue (handle);

                mv->data = *data;
                mv->time = CurrentTime;
                messageQueues[localEndpointIndex].push (std::move (mv));
                if (eptInfo->callbackIndex >= 0)
                {
                    // make sure the lock is not engaged for the callback
                    auto cb = callbacks[eptInfo->callbackIndex];
                    eplock.unlock ();
                    cb (sfnd->second.first, newTime);
                    eplock.lock ();
                }
                else if (allCallbackIndex >= 0)
                {
                    // make sure the lock is not engaged for the callback
                    auto ac = callbacks[allCallbackIndex];
                    eplock.unlock ();
                    ac (sfnd->second.first, CurrentTime);
                    eplock.lock ();
                }
            }
        }
    }
}

void MessageFederateManager::startupToInitializeStateTransition () { messageQueues.resize (endpointCount); }

void MessageFederateManager::initializeToExecuteStateTransition () {}

static const std::string nullStr;

std::string MessageFederateManager::getEndpointName (endpoint_id_t id) const
{
    return (id.value () < endpointCount) ? (*local_endpoints.lock_shared ())[id.value ()]->name : nullStr;
}

endpoint_id_t MessageFederateManager::getEndpointId (const std::string &name) const
{
    auto sharedEpt = local_endpoints.lock_shared ();
    auto sub = sharedEpt->find (name);
    return (sub != nullptr) ? sub->id : 0;
}

std::string MessageFederateManager::getEndpointType (endpoint_id_t id) const
{
    return (id.value () < endpointCount) ? (*local_endpoints.lock_shared ())[id.value ()]->type : nullStr;
}

int MessageFederateManager::getEndpointCount () const
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    return static_cast<int> (endpointCount);
}

void MessageFederateManager::registerCallback (const std::function<void(endpoint_id_t, Time)> &callback)
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

void MessageFederateManager::registerCallback (endpoint_id_t id,
                                               const std::function<void(endpoint_id_t, Time)> &callback)
{
    if (id.value () < endpointCount)
    {
        auto eplock = local_endpoints.lock ();
        assert (eplock);
        (*eplock)[id.value ()]->callbackIndex = static_cast<int> (callbacks.size ());
        callbacks.push_back (callback);
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::registerCallback (const std::vector<endpoint_id_t> &ids,
                                               const std::function<void(endpoint_id_t, Time)> &callback)
{
    int ind = static_cast<int> (callbacks.size ());
    callbacks.push_back (callback);
    auto cnt = endpointCount.load ();
    auto eptLock = local_endpoints.lock ();
    assert (eptLock);
    for (auto id : ids)
    {
        if (id.value () < cnt)
        {
            (*eptLock)[id.value ()]->callbackIndex = ind;
        }
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
