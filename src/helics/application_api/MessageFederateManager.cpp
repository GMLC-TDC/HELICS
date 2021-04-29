/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MessageFederateManager.hpp"

#include "../core/Core.hpp"
#include "../core/queryHelpers.hpp"
#include "helics/core/core-exceptions.hpp"

#include <cassert>
#include <string>

namespace helics {
MessageFederateManager::MessageFederateManager(Core* coreOb,
                                               MessageFederate* fed,
                                               local_federate_id id):
    coreObject(coreOb),
    mFed(fed), fedID(id)
{
}
MessageFederateManager::~MessageFederateManager() = default;

void MessageFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = nullptr;
}

Endpoint& MessageFederateManager::registerEndpoint(const std::string& name, const std::string& type)
{
    auto handle = coreObject->registerEndpoint(fedID, name, type);
    if (handle.isValid()) {
        auto edat = std::make_unique<EndpointData>();

        auto eptHandle = local_endpoints.lock();
        auto loc = eptHandle->insert(name, handle, mFed, name, handle, edat.get());
        if (loc) {
            auto& ref = eptHandle->back();
            ref.referenceIndex = static_cast<int>(*loc);
            eptHandle.unlock();

            //** now insert the data into the appropriate location in the data array
            auto datHandle = eptData.lock();
            if (datHandle->size() == loc) {
                datHandle->push_back(std::move(edat));
            } else if (datHandle->size() < loc) {
                datHandle->resize(*loc + 1);
                (*datHandle)[*loc] = std::move(edat);
            } else {
                (*datHandle)[*loc] = std::move(edat);
            }

            return ref;
        }
    }
    throw(RegistrationFailure("Unable to register Endpoint"));
}

void MessageFederateManager::registerKnownCommunicationPath(const Endpoint& localEndpoint,
                                                            const std::string& remoteEndpoint)
{
    coreObject->registerFrequentCommunicationsPair(localEndpoint.getName(), remoteEndpoint);
}

void MessageFederateManager::subscribe(const Endpoint& ept, const std::string& name)
{
    coreObject->addSourceTarget(ept.handle, name);
}

bool MessageFederateManager::hasMessage() const
{
    auto eptDat = eptData.lock_shared();
    for (const auto& mq : eptDat) {
        if (!mq->messages.empty()) {
            return true;
        }
    }
    return false;
}

bool MessageFederateManager::hasMessage(const Endpoint& ept)
{
    if (ept.dataReference != nullptr) {
        auto* eptDat = reinterpret_cast<EndpointData*>(ept.dataReference);
        return (!eptDat->messages.empty());
    }
    return false;
}

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederateManager::pendingMessages(const Endpoint& ept)
{
    if (ept.dataReference != nullptr) {
        auto* eptDat = reinterpret_cast<EndpointData*>(ept.dataReference);
        return eptDat->messages.size();
    }
    return 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multi-threaded contexts due to the required locking
prefer to just use getMessage until it returns an invalid Message.
*/
uint64_t MessageFederateManager::pendingMessages() const
{
    auto eptDat = eptData.lock_shared();
    uint64_t sz = 0;
    for (const auto& mq : eptDat) {
        sz += mq->messages.size();
    }
    return sz;
}

std::unique_ptr<Message> MessageFederateManager::getMessage(const Endpoint& ept)
{
    if (ept.dataReference != nullptr) {
        auto* eptDat = reinterpret_cast<EndpointData*>(ept.dataReference);
        auto mv = eptDat->messages.pop();
        if (mv) {
            return std::move(*mv);
        }
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederateManager::getMessage()
{
    // just start with the first endpoint and check until a queue isn't empty
    auto eptDat = eptData.lock();
    for (auto& edat : eptDat) {
        if (!edat->messages.empty()) {
            auto ms = edat->messages.pop();
            if (ms) {
                return std::move(*ms);
            }
        }
    }
    return nullptr;
}

void MessageFederateManager::sendMessage(const Endpoint& source,
                                         const std::string& dest,
                                         const data_view& message)
{
    coreObject->send(source.handle, dest, message.data(), message.size());
}

void MessageFederateManager::sendMessage(const Endpoint& source,
                                         const std::string& dest,
                                         const data_view& message,
                                         Time sendTime)
{
    coreObject->sendEvent(sendTime, source.handle, dest, message.data(), message.size());
}

void MessageFederateManager::sendMessage(const Endpoint& source, std::unique_ptr<Message> message)
{
    coreObject->sendMessage(source.handle, std::move(message));
}

void MessageFederateManager::updateTime(Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto epCount = coreObject->receiveCountAny(fedID);
    if (epCount == 0) {
        return;
    }
    // lock the data updates
    auto eptDat = eptData.lock();

    interface_handle endpoint_id;
    auto epts = local_endpoints.lock();
    auto mcall = allCallback.load();
    for (size_t ii = 0; ii < epCount; ++ii) {
        auto message = coreObject->receiveAny(fedID, endpoint_id);
        if (!message) {
            break;
        }

        /** find the id*/
        auto fid = epts->find(endpoint_id);
        if (fid != epts->end()) {  // assign the data

            Endpoint& currentEpt = *fid;
            auto localEndpointIndex = fid->referenceIndex;
            (*eptDat)[localEndpointIndex]->messages.emplace(std::move(message));
            auto cb = (*eptDat)[localEndpointIndex]->callback.load();
            if (cb) {
                // need to be copied otherwise there is a potential race condition on lock removal
                eptDat.unlock();
                epts.unlock();
                cb(currentEpt, CurrentTime);
                eptDat = eptData.lock();
                epts = local_endpoints.lock();
            } else if (mcall) {
                eptDat.unlock();
                epts.unlock();
                mcall(currentEpt, CurrentTime);
                eptDat = eptData.lock();
                epts = local_endpoints.lock();
            }
        }
    }
}

void MessageFederateManager::startupToInitializeStateTransition() {}

void MessageFederateManager::initializeToExecuteStateTransition(iteration_result result)
{
    Time ctime = result == iteration_result::next_step ? timeZero : initializationTime;
    updateTime(ctime, initializationTime);
}

std::string MessageFederateManager::localQuery(const std::string& queryStr) const
{
    std::string ret;
    if (queryStr == "endpoints") {
        ret = generateStringVector_if(
            local_endpoints.lock_shared(),
            [](const auto& info) { return info.actualName; },
            [](const auto& info) { return (!info.actualName.empty()); });
    }
    return ret;
}

static const Endpoint invalidEpt{};
static Endpoint invalidEptNC{};

Endpoint& MessageFederateManager::getEndpoint(const std::string& name)
{
    auto sharedEpt = local_endpoints.lock();
    auto ept = sharedEpt->find(name);
    return (ept != sharedEpt.end()) ? (*ept) : invalidEptNC;
}
const Endpoint& MessageFederateManager::getEndpoint(const std::string& name) const
{
    auto sharedEpt = local_endpoints.lock_shared();
    auto ept = sharedEpt->find(name);
    return (ept != sharedEpt.end()) ? (*ept) : invalidEpt;
}

Endpoint& MessageFederateManager::getEndpoint(int index)
{
    auto sharedEpt = local_endpoints.lock();
    if (isValidIndex(index, *sharedEpt)) {
        return (*sharedEpt)[index];
    }
    return invalidEptNC;
}
const Endpoint& MessageFederateManager::getEndpoint(int index) const
{
    auto sharedEpt = local_endpoints.lock_shared();
    if (isValidIndex(index, *sharedEpt)) {
        return (*sharedEpt)[index];
    }
    return invalidEpt;
}

int MessageFederateManager::getEndpointCount() const
{
    return static_cast<int>(local_endpoints.lock_shared()->size());
}

void MessageFederateManager::addSourceFilter(const Endpoint& ept, const std::string& filterName)
{
    coreObject->addSourceTarget(ept.handle, filterName);
}

/** add a named filter to an endpoint for all message going to the endpoint*/
void MessageFederateManager::addDestinationFilter(const Endpoint& ept,
                                                  const std::string& filterName)
{
    coreObject->addDestinationTarget(ept.handle, filterName);
}

void MessageFederateManager::setEndpointNotificationCallback(
    const std::function<void(Endpoint&, Time)>& callback)
{
    allCallback.store(callback);
}

void MessageFederateManager::setEndpointNotificationCallback(
    const Endpoint& ept,
    const std::function<void(Endpoint&, Time)>& callback)
{
    if (ept.dataReference != nullptr) {
        auto* eptDat = reinterpret_cast<EndpointData*>(ept.dataReference);
        eptDat->callback = callback;
    }
}

void MessageFederateManager::removeOrderedMessage(unsigned int index)
{
    auto handle = messageOrder.lock();
    if (index == handle->back()) {
        handle->pop_back();
    } else {
        auto term = handle->rend();
        for (auto ri = handle->rbegin() + 1; ri != term; ++ri) {
            if (*ri == index) {
                handle->erase(ri.base());
                break;
            }
        }
    }
}
}  // namespace helics
