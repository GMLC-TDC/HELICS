/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "InterfaceInfo.hpp"

namespace helics
{

void InterfaceInfo::createSubscription(Core::handle_id_t handle,
    const std::string &key,
    const std::string &type,
    const std::string &units,
    handle_check_mode check_mode)
{
    auto subHandle = subscriptions.lock();
    subHandle->insert(key, handle, handle, global_id, key, type, units,
        (check_mode == handle_check_mode::required));

    subHandle->back()->only_update_on_change = only_update_on_change;
}

void InterfaceInfo::createControlOutput(Core::handle_id_t handle,
    const std::string &key,
    const std::string &type,
    const std::string &units)
{
    controlOutputs.lock()->insert(key, handle, handle, global_id, key, type, units);
}

void InterfaceInfo::createPublication(Core::handle_id_t handle,
    const std::string &key,
    const std::string &type,
    const std::string &units)
{
    publications.lock()->insert(key, handle, handle, global_id, key, type, units);
}

void InterfaceInfo::createControlInput(Core::handle_id_t handle,
    const std::string &key,
    const std::string &type,
    const std::string &units)
{
    auto ciHandle = controlInputs.lock();
    ciHandle->insert(key, handle, handle, global_id, key, type, units);
    ciHandle->back()->only_update_on_change = only_update_on_change;
}

void InterfaceInfo::createEndpoint(Core::handle_id_t handle,
    const std::string &endpointName,
    const std::string &type)
{
    endpoints.lock()->insert(endpointName, handle, handle, global_id, endpointName, type);
}

void InterfaceInfo::setChangeUpdateFlag(bool updateFlag)
{
    if (updateFlag != only_update_on_change)
    {
        only_update_on_change = updateFlag;
        for (auto &sub : subscriptions.lock())
        {
            sub->only_update_on_change = updateFlag;
        }
    }
    
}

const SubscriptionInfo *InterfaceInfo::getSubscription(const std::string &subName) const
{
    return subscriptions.lock_shared()->find(subName);
}

SubscriptionInfo *InterfaceInfo::getSubscription(const std::string &subName)
{
    return subscriptions.lock()->find(subName);
}

const SubscriptionInfo *InterfaceInfo::getSubscription(Core::handle_id_t handle_) const
{
    return subscriptions.lock_shared()->find(handle_);
}

SubscriptionInfo *InterfaceInfo::getSubscription(Core::handle_id_t handle_)
{
    return subscriptions.lock()->find(handle_);
}

const PublicationInfo *InterfaceInfo::getPublication(const std::string &pubName) const
{
    return publications.lock_shared()->find(pubName);
}

const PublicationInfo *InterfaceInfo::getPublication(Core::handle_id_t handle_) const
{
    return publications.lock()->find(handle_);
}

PublicationInfo *InterfaceInfo::getPublication(const std::string &pubName)
{
    return publications.lock()->find(pubName);
}

PublicationInfo *InterfaceInfo::getPublication(Core::handle_id_t handle_)
{
    return publications.lock()->find(handle_);
}

const ControlOutputInfo *InterfaceInfo::getControlOutput(const std::string &subName) const
{
    return controlOutputs.lock_shared()->find(subName);
}

ControlOutputInfo *InterfaceInfo::getControlOutput(const std::string &subName)
{
    return controlOutputs.lock()->find(subName);
}

const ControlOutputInfo *InterfaceInfo::getControlOutput(Core::handle_id_t handle_) const
{
    return controlOutputs.lock_shared()->find(handle_);
}

ControlOutputInfo *InterfaceInfo::getControlOutput(Core::handle_id_t handle_)
{
    return controlOutputs.lock()->find(handle_);
}

const ControlInputInfo *InterfaceInfo::getControlInput(const std::string &pubName) const
{
    return controlInputs.lock_shared()->find(pubName);
}

const ControlInputInfo *InterfaceInfo::getControlInput(Core::handle_id_t handle_) const
{
    return controlInputs.lock()->find(handle_);
}

ControlInputInfo *InterfaceInfo::getControlInput(const std::string &pubName)
{
    return controlInputs.lock()->find(pubName);
}

ControlInputInfo *InterfaceInfo::getControlInput(Core::handle_id_t handle_)
{
    return controlInputs.lock()->find(handle_);
}

const EndpointInfo *InterfaceInfo::getEndpoint(const std::string &endpointName) const
{
    return endpoints.lock_shared()->find(endpointName);
}

const EndpointInfo *InterfaceInfo::getEndpoint(Core::handle_id_t handle_) const
{
    return endpoints.lock_shared()->find(handle_);
}

EndpointInfo *InterfaceInfo::getEndpoint(const std::string &endpointName)
{
    return endpoints.lock()->find(endpointName);
}

EndpointInfo *InterfaceInfo::getEndpoint(Core::handle_id_t handle_) { return endpoints.lock()->find(handle_); }

}