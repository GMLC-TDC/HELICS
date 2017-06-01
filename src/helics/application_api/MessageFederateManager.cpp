/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MessageFederateManager.h"
#include "helics/core/core.h"
namespace helics
{
message_t generateCoreMessage (const Message_view &mv)
{
    message_t m;
    m.src = mv.src.data ();
    m.dst = mv.dest.data ();
    m.origsrc = mv.origsrc.data ();
    m.data = mv.data.data ();
    m.len = mv.data.size ();
    return m;
}

MessageFederateManager::MessageFederateManager (std::shared_ptr<Core> coreOb, Core::federate_id_t id)
    : coreObject (std::move (coreOb)), fedID (id)
{
}
MessageFederateManager::~MessageFederateManager () = default;

endpoint_id_t MessageFederateManager::registerEndpoint (const std::string &name, const std::string &type)
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    endpoint_id_t id = static_cast<identifier_type> (local_endpoints.size ());
    local_endpoints.emplace_back (name, type);
    local_endpoints.back ().id = id;
    local_endpoints.back ().handle = coreObject->registerEndpoint (fedID, name.c_str (), type.c_str ());
    endpointNames.emplace (name, id);
    handleLookup.emplace (local_endpoints.back ().handle, id);
    return id;
}


void MessageFederateManager::registerKnownCommunicationPath (endpoint_id_t localEndpoint,
                                                             const std::string &remoteEndpoint)
{
    if (localEndpoint.value () < local_endpoints.size ())
    {
        coreObject->registerFrequentCommunicationsPair (local_endpoints[localEndpoint.value ()].name.c_str (),
                                                        remoteEndpoint.c_str ());
    }
}

void MessageFederateManager::subscribe (endpoint_id_t endpoint, const std::string &name, const std::string &type)
{
    if (endpoint.value () < local_endpoints.size ())
    {
        auto h = coreObject->registerSubscription (fedID, name.c_str (), type.c_str (), "", false);
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
    return (id.value () < local_endpoints.size ()) ? (!messageQueues[id.value ()].empty ()) : false;
}


/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederateManager::receiveCount (endpoint_id_t id) const
{
    return (id.value () < local_endpoints.size ()) ? (messageQueues[id.value ()].size ()) : 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multithreaded contexts due to the required locking
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

Message_view MessageFederateManager::getMessage (endpoint_id_t endpoint)
{
    if (endpoint.value () < local_endpoints.size ())
    {
        auto mv = messageQueues[endpoint.value ()].pop ();
        if (mv)
        {
            return *mv;
        }
    }
    return Message_view{};
}

Message_view MessageFederateManager::getMessage ()
{
    // just start with the first endpoint and check until a queue isn't empty
    for (auto &mq : messageQueues)
    {
        if (!mq.empty ())
        {
            auto ms = mq.pop ();
            if (ms)
            {
                return *ms;
            }
        }
    }
    return Message_view{};
}


void MessageFederateManager::sendMessage (endpoint_id_t source, const char *dest, data_view message)
{
    if (source.value () < local_endpoints.size ())
    {
        coreObject->send (local_endpoints[source.value ()].handle, dest, message.data (), message.size ());
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (endpoint_id_t source, const char *dest, data_view message, Time sendTime)
{
    if (source.value () < local_endpoints.size ())
    {
        coreObject->sendEvent (sendTime, local_endpoints[source.value ()].handle, dest, message.data (),
                               message.size ());
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::sendMessage (endpoint_id_t source, Message_view message)
{
    if (source.value () < local_endpoints.size ())
    {
        auto m = generateCoreMessage (message);
        m.src = local_endpoints[source.value ()].name.c_str ();
        coreObject->sendMessage (&m);
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
    for (size_t ii = 0; ii < epCount; ++ii)
    {
        auto msgp = coreObject->receiveAny (fedID);
        if (msgp.second == nullptr)
        {
            break;
        }

        /** find the id*/
        auto fid = handleLookup.find (msgp.first);
        if (fid != handleLookup.end ())
        {  // assign the data

            /** making a shared pointer with custom deleter*/
            auto sd = std::shared_ptr<message_t> (msgp.second, [=](message_t *m) { coreObject->dereference (m); });
            auto localEndpointIndex = fid->second.value ();
            messageQueues[localEndpointIndex].emplace (std::move (sd));
            if (local_endpoints[localEndpointIndex].callbackIndex >= 0)
            {
				auto cb = callbacks[local_endpoints[localEndpointIndex].callbackIndex];
               eplock.unlock ();
                cb(fid->second, CurrentTime);
                eplock.lock ();
            }
            else if (allCallbackIndex >= 0)
            {
				auto ac = callbacks[allCallbackIndex];
                eplock.unlock ();
                ac(fid->second, CurrentTime);
                eplock.lock ();
            }
        }
    }
    if (hasSubscriptions)
    {
        auto handles = coreObject->getValueUpdates (fedID, &epCount);
        for (decltype (epCount) ii = 0; ii < epCount; ++ii)
        {
            auto sfnd = subHandleLookup.find (handles[ii]);
            if (sfnd != subHandleLookup.end ())
            {
                Message_view mv;
                mv.src = sfnd->second.second;
                auto localEndpointIndex = sfnd->second.first.value ();
                mv.dest = local_endpoints[localEndpointIndex].name;
                mv.origsrc = mv.src;
                // get the data value
                auto data = coreObject->getValue (handles[ii]);
                /** making a shared pointer with custom deleter*/
                auto sd = std::shared_ptr<data_t> (data, [=](data_t *v) { coreObject->dereference (v); });

                mv.data = data_view (std::move (sd));
                mv.time = CurrentTime;
                messageQueues[localEndpointIndex].push (std::move (mv));
                if (local_endpoints[localEndpointIndex].callbackIndex >= 0)
                {
					auto cb = callbacks[local_endpoints[localEndpointIndex].callbackIndex];
                    eplock.unlock ();
                    cb(sfnd->second.first, newTime);
                    eplock.lock ();
                }
                else if (allCallbackIndex >= 0)
                {
					auto ac = callbacks[allCallbackIndex];
                    eplock.unlock ();
                    ac(sfnd->second.first, CurrentTime);
                    eplock.lock ();
                }
            }
        }
    }
}

void MessageFederateManager::StartupToInitializeStateTransition ()
{
    messageQueues.resize (local_endpoints.size ());
}

void MessageFederateManager::InitializeToExecuteStateTransition () {}


static const std::string nullStr;

std::string MessageFederateManager::getEndpointName (endpoint_id_t id) const
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    return (id.value () < local_endpoints.size ()) ? local_endpoints[id.value ()].name : nullStr;
}


endpoint_id_t MessageFederateManager::getEndpointId (const std::string &name) const
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    auto sub = endpointNames.find (name);
    return (sub != endpointNames.end ()) ? sub->second : 0;
}

std::string MessageFederateManager::getEndpointType (endpoint_id_t id) const
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    return (id.value () < local_endpoints.size ()) ? local_endpoints[id.value ()].type : nullStr;
}

void MessageFederateManager::registerCallback (std::function<void(endpoint_id_t, Time)> callback)
{
    std::lock_guard<std::mutex> eLock (endpointLock);
    if (allCallbackIndex < 0)
    {
        allCallbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
    else
    {
        callbacks[allCallbackIndex] = std::move (callback);
    }
}

void MessageFederateManager::registerCallback (endpoint_id_t id, std::function<void(endpoint_id_t, Time)> callback)
{
    if (id.value () < local_endpoints.size ())
    {
        std::lock_guard<std::mutex> eLock (endpointLock);
        local_endpoints[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
    else
    {
        throw (std::invalid_argument ("endpoint id is invalid"));
    }
}

void MessageFederateManager::registerCallback (const std::vector<endpoint_id_t> &ids,
                                               std::function<void(endpoint_id_t, Time)> callback)
{
    std::lock_guard<std::mutex> eLock (endpointLock);

    int ind = static_cast<int> (callbacks.size ());
    callbacks.emplace_back (std::move (callback));
    for (auto id : ids)
    {
        if (id.value () < local_endpoints.size ())
        {
            local_endpoints[id.value ()].callbackIndex = ind;
        }
    }
}

void MessageFederateManager::removeOrderedMessage (unsigned int index)
{
    std::lock_guard<std::mutex> mLock (morderMutex);
    if (index == messageOrder.back ())
    {
        messageOrder.pop_back ();
    }
    else
    {
        auto term = messageOrder.rend ();
        for (auto ri = messageOrder.rbegin () + 1; ri != term; ++ri)
        {
            if (*ri == index)
            {
                messageOrder.erase (ri.base ());
                break;
            }
        }
    }
}
}
