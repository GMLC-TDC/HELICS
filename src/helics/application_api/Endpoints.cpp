/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Endpoints.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "MessageFederate.hpp"

namespace helics {

Endpoint::Endpoint(MessageFederate* mFed, const std::string& name, InterfaceHandle id, void* data):
    Interface(mFed, id, name), fed(mFed), dataReference(data)
{
}

Endpoint::Endpoint(MessageFederate* mFed, const std::string& name, const std::string& type):
    Endpoint(mFed->registerEndpoint(name, type))
{
}

Endpoint::Endpoint(interface_visibility locality,
                   MessageFederate* mFed,
                   const std::string& name,
                   const std::string& type)
{
    if (locality == interface_visibility::global) {
        operator=(mFed->registerGlobalEndpoint(name, type));
    } else {
        operator=(mFed->registerEndpoint(name, type));
    }
}

void Endpoint::send(const char* data, size_t data_size) const
{
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        cr->send(handle, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::sendTo(std::string_view dest, const char* data, size_t data_size) const
{
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        if (dest.empty()) {
            dest = defDest;
        }
        cr->sendTo(handle, dest, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void Endpoint::sendAt(Time sendTime, const char* data, size_t data_size) const
{
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        cr->sendAt(handle, sendTime, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

/** send a data block and length to a destination at particular time
@param dest string name of the destination
@param data pointer to data location
@param data_size the length of the data
@param sendTime the time to send the message
*/
void Endpoint::sendToAt(std::string_view dest,
                        Time sendTime,
                        const char* data,
                        size_t data_size) const
{
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        if (dest.empty()) {
            dest = defDest;
        }
        cr->sendToAt(handle, dest, sendTime, data, data_size);
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
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        cr->send(handle, data, data_size);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

/** send a pointer to a message object*/
void Endpoint::send(std::unique_ptr<Message> mess) const
{
    if ((cr != nullptr) &&
        ((fed->getCurrentMode() == Federate::modes::executing) ||
         (fed->getCurrentMode() == Federate::modes::initializing))) {
        if (mess->dest.empty()) {
            mess->dest = defDest;
        }
        cr->sendMessage(handle, std::move(mess));
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

static const std::string emptyStr;

void Endpoint::setDefaultDestination(std::string_view target)
{
    if (defDest.empty() && fed->getCurrentMode() < Federate::modes::executing) {
        addDestinationTarget(target);
    }
    defDest = target;
}

const std::string& Endpoint::getDefaultDestination() const
{
    return (!defDest.empty()) ? defDest :
                                ((cr != nullptr) ? cr->getDestinationTargets(handle) : emptyStr);
}

void Endpoint::subscribe(const std::string& key)
{
    if (cr != nullptr) {
        cr->addSourceTarget(handle, key);
    }
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
std::uint64_t Endpoint::pendingMessages() const
{
    return (fed != nullptr) ? fed->pendingMessages(*this) : 0;
}

void Endpoint::setCallback(const std::function<void(const Endpoint&, Time)>& callback)
{
    if (fed != nullptr) {
        fed->setMessageNotificationCallback(*this, callback);
    }
}

/** add a named filter to an endpoint for all messages coming from the endpoint*/
void Endpoint::addSourceFilter(const std::string& filterName)
{
    if (cr != nullptr) {
        cr->addSourceTarget(handle, filterName, handle_type::filter);
    }
}
/** add a named filter to an endpoint for all messages going to the endpoint*/
void Endpoint::addDestinationFilter(const std::string& filterName)
{
    if (cr != nullptr) {
        cr->addDestinationTarget(handle, filterName, handle_type::filter);
    }
}
}  // namespace helics
