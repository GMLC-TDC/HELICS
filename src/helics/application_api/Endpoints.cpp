/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Endpoints.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "MessageFederate.hpp"

#include <memory>
#include <string>
#include <utility>

namespace helics {

Endpoint::Endpoint(MessageFederate* mFed, std::string_view name, InterfaceHandle hid):
    Interface(mFed, hid, name), fed(mFed)
{
}

Endpoint::Endpoint(MessageFederate* mFed, std::string_view name, std::string_view type):
    Endpoint(mFed->registerEndpoint(name, type))
{
}

Endpoint::Endpoint(InterfaceVisibility locality,
                   MessageFederate* mFed,
                   std::string_view name,
                   std::string_view type)
{
    if (locality == InterfaceVisibility::GLOBAL) {
        operator=(mFed->registerGlobalEndpoint(name, type));
    } else {
        operator=(mFed->registerEndpoint(name, type));
    }
}

void Endpoint::send(const char* data, size_t data_size) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        mCore->send(handle, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::sendTo(const char* data, size_t data_size, std::string_view dest) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        if (dest.empty()) {
            dest = defDest;
        }
        mCore->sendTo(handle, data, data_size, dest);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::sendAt(const char* data, size_t data_size, Time sendTime) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        mCore->sendAt(handle, data, data_size, sendTime);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::sendToAt(const char* data,
                        size_t data_size,
                        std::string_view dest,
                        Time sendTime) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        if (dest.empty()) {
            dest = defDest;
        }
        mCore->sendToAt(handle, data, data_size, dest, sendTime);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}
/** send a data block and length to the target destination
@param data pointer to data location
@param data_size the length of the data
*/
void Endpoint::send(const void* data, size_t data_size) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        mCore->send(handle, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

/** send a pointer to a message object*/
void Endpoint::send(std::unique_ptr<Message> mess) const
{
    if ((fed->getCurrentMode() == Federate::Modes::EXECUTING) ||
        (fed->getCurrentMode() == Federate::Modes::INITIALIZING)) {
        if (mess->dest.empty()) {
            mess->dest = defDest;
        }
        mCore->sendMessage(handle, std::move(mess));
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::setDefaultDestination(std::string_view target)
{
    if (defDest.empty() && fed->getCurrentMode() < Federate::Modes::EXECUTING) {
        addDestinationTarget(target);
    }
    defDest = target;
}

const std::string& Endpoint::getDefaultDestination() const
{
    return (!defDest.empty()) ? defDest : mCore->getDestinationTargets(handle);
}

void Endpoint::subscribe(std::string_view key)
{
    mCore->addSourceTarget(handle, key, helics::InterfaceType::PUBLICATION);
}

std::unique_ptr<Message> Endpoint::getMessage() const
{
    return (fed != nullptr) ? fed->getMessage(*this) : nullptr;
}

/** check if there is a message available*/
bool Endpoint::hasMessage() const
{
    return (fed != nullptr) ? fed->hasMessage(*this) : false;
}

/** check if there is a message available*/
std::uint64_t Endpoint::pendingMessageCount() const
{
    return (fed != nullptr) ? fed->pendingMessageCount(*this) : 0;
}

void Endpoint::setCallback(const std::function<void(const Endpoint&, Time)>& callback)
{
    if (fed != nullptr) {
        fed->setMessageNotificationCallback(*this, callback);
    }
}

/** add a named filter to an endpoint for all messages coming from the endpoint*/
void Endpoint::addSourceFilter(std::string_view filterName)
{
    mCore->addSourceTarget(handle, filterName, InterfaceType::FILTER);
}
/** add a named filter to an endpoint for all messages going to the endpoint*/
void Endpoint::addDestinationFilter(std::string_view filterName)
{
    mCore->addDestinationTarget(handle, filterName, InterfaceType::FILTER);
}

void Endpoint::addSourceEndpoint(std::string_view endpointName)
{
    mCore->addSourceTarget(handle, endpointName, InterfaceType::ENDPOINT);
}
void Endpoint::addDestinationEndpoint(std::string_view endpointName)
{
    mCore->addDestinationTarget(handle, endpointName, InterfaceType::ENDPOINT);
}
}  // namespace helics
