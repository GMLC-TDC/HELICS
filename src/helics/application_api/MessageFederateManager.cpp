/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MessageFederateManager.hpp"

#include "../core/Core.hpp"
#include "../core/EmptyCore.hpp"
#include "../core/queryHelpers.hpp"
#include "helics/core/core-exceptions.hpp"

#include <cassert>
#include <string>

namespace helics {
MessageFederateManager::MessageFederateManager(Core* coreOb,
                                               MessageFederate* fed,
                                               LocalFederateId id):
    coreObject(coreOb),
    mFed(fed), fedID(id)
{
}
MessageFederateManager::~MessageFederateManager() = default;

static EmptyCore eCore;

void MessageFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = &eCore;
}

Endpoint& MessageFederateManager::registerEndpoint(std::string_view name, std::string_view type)
{
    auto handle = coreObject->registerEndpoint(fedID, name, type);
    if (handle.isValid()) {

        auto eptHandle = local_endpoints.lock();
        auto loc = eptHandle->insert(name, handle, mFed, name, handle);
        if (loc) {
            auto& ref = eptHandle->back();
            auto datHandle = eptData.lock();
            auto& edat = datHandle->emplace_back();

            // non-owning pointer
            ref.dataReference = &edat;
            datHandle.unlock();
            ref.referenceIndex = static_cast<int>(*loc);

            return ref;
        }
    }
    throw(RegistrationFailure("Unable to register Endpoint"));
}

Endpoint& MessageFederateManager::registerTargetedEndpoint(std::string_view name,
                                                           std::string_view type)
{
    auto handle = coreObject->registerTargetedEndpoint(fedID, name, type);
    if (handle.isValid()) {
        auto eptHandle = local_endpoints.lock();
        auto loc = eptHandle->insert(name, handle, mFed, name, handle);
        if (loc) {
            auto& ref = eptHandle->back();
            auto datHandle = eptData.lock();
            auto& edat = datHandle->emplace_back();

            // non-owning pointer
            ref.dataReference = &edat;
            datHandle.unlock();
            ref.referenceIndex = static_cast<int>(*loc);

            return ref;
        }
    }
    throw(RegistrationFailure("Unable to register Targeted Endpoint"));
}
bool MessageFederateManager::hasMessage() const
{
    auto eptDat = eptData.lock_shared();
    for (const auto& mq : eptDat) {
        if (!mq.messages.empty()) {
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
uint64_t MessageFederateManager::pendingMessageCount(const Endpoint& ept)
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
uint64_t MessageFederateManager::pendingMessageCount() const
{
    auto eptDat = eptData.lock_shared();
    uint64_t sz = 0;
    for (const auto& mq : eptDat) {
        sz += mq.messages.size();
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
        if (!edat.messages.empty()) {
            auto ms = edat.messages.pop();
            if (ms) {
                return std::move(*ms);
            }
        }
    }
    return nullptr;
}

void MessageFederateManager::updateTime(Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto epCount = coreObject->receiveCountAny(fedID);
    if (epCount == 0) {
        return;
    }
    InterfaceHandle endpoint_id;
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
            auto* eData = static_cast<EndpointData*>(fid->dataReference);

            eData->messages.emplace(std::move(message));

            if (eData->callback) {
                // need to be copied otherwise there is a potential race condition on lock removal
                epts.unlock();
                eData->callback(currentEpt, CurrentTime);
                epts = local_endpoints.lock();
            } else if (mcall) {
                epts.unlock();
                mcall(currentEpt, CurrentTime);
                epts = local_endpoints.lock();
            }
        }
    }
}

void MessageFederateManager::startupToInitializeStateTransition() {}

void MessageFederateManager::initializeToExecuteStateTransition(IterationResult result)
{
    Time ctime = result == IterationResult::NEXT_STEP ? timeZero : initializationTime;
    updateTime(ctime, initializationTime);
}

std::string MessageFederateManager::localQuery(std::string_view queryStr) const
{
    std::string ret;
    if (queryStr == "endpoints") {
        ret = generateStringVector_if(
            local_endpoints.lock_shared(),
            [](const auto& info) { return info.getName(); },
            [](const auto& info) { return (!info.getName().empty()); });
    }
    return ret;
}

static const Endpoint invalidEpt{};
static Endpoint invalidEptNC{};

Endpoint& MessageFederateManager::getEndpoint(std::string_view name)
{
    auto sharedEpt = local_endpoints.lock();
    auto ept = sharedEpt->find(name);
    return (ept != sharedEpt.end()) ? (*ept) : invalidEptNC;
}
const Endpoint& MessageFederateManager::getEndpoint(std::string_view name) const
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
