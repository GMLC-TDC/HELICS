/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "InterfaceInfo.hpp"

namespace helics
{
void InterfaceInfo::createPublication (interface_handle handle,
                                       const std::string &key,
                                       const std::string &type,
                                       const std::string &units)
{
    publications.lock ()->insert (key, handle, global_handle{global_id, handle}, key, type, units);
}

void InterfaceInfo::createInput (interface_handle handle,
                                 const std::string &key,
                                 const std::string &type,
                                 const std::string &units)
{
    auto ciHandle = inputs.lock ();
    ciHandle->insert (key, handle, global_handle{global_id, handle}, key, type, units);
    ciHandle->back ()->only_update_on_change = only_update_on_change;
}

void InterfaceInfo::createEndpoint (interface_handle handle,
                                    const std::string &endpointName,
                                    const std::string &type)
{
    endpoints.lock ()->insert (endpointName, handle, global_handle{global_id, handle}, endpointName, type);
}

void InterfaceInfo::setChangeUpdateFlag (bool updateFlag)
{
    if (updateFlag != only_update_on_change)
    {
        only_update_on_change = updateFlag;
        for (auto &ip : inputs.lock ())
        {
            ip->only_update_on_change = updateFlag;
        }
    }
}

const PublicationInfo *InterfaceInfo::getPublication (const std::string &pubName) const
{
    return publications.lock_shared ()->find (pubName);
}

const PublicationInfo *InterfaceInfo::getPublication (interface_handle handle_) const
{
    return publications.lock ()->find (handle_);
}

PublicationInfo *InterfaceInfo::getPublication (const std::string &pubName)
{
    return publications.lock ()->find (pubName);
}

PublicationInfo *InterfaceInfo::getPublication (interface_handle handle_)
{
    return publications.lock ()->find (handle_);
}

const NamedInputInfo *InterfaceInfo::getInput (const std::string &pubName) const
{
    return inputs.lock_shared ()->find (pubName);
}

const NamedInputInfo *InterfaceInfo::getInput (interface_handle handle_) const
{
    return inputs.lock ()->find (handle_);
}

NamedInputInfo *InterfaceInfo::getInput (const std::string &pubName) { return inputs.lock ()->find (pubName); }

NamedInputInfo *InterfaceInfo::getInput (interface_handle handle_) { return inputs.lock ()->find (handle_); }

const EndpointInfo *InterfaceInfo::getEndpoint (const std::string &endpointName) const
{
    return endpoints.lock_shared ()->find (endpointName);
}

const EndpointInfo *InterfaceInfo::getEndpoint (interface_handle handle_) const
{
    return endpoints.lock_shared ()->find (handle_);
}

EndpointInfo *InterfaceInfo::getEndpoint (const std::string &endpointName)
{
    return endpoints.lock ()->find (endpointName);
}

EndpointInfo *InterfaceInfo::getEndpoint (interface_handle handle_) { return endpoints.lock ()->find (handle_); }
}  // namespace helics
