/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "InterfaceInfo.hpp"
#include "../common/fmt_format.h"
#include "helics_definitions.hpp"

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

bool InterfaceInfo::setInputProperty (interface_handle id, int option, bool value)
{
    auto ipt = getInput (id);
    switch (option)
    {
    case defs::options::ignore_interrupts:
        ipt->not_interruptible = value;
        break;
    case defs::options::handle_only_update_on_change:
        ipt->only_update_on_change = value;
        break;
    case defs::options::connection_required:
        ipt->required = value;
        break;
    case defs::options::connection_optional:
        ipt->required = !value;
        break;
    case defs::options::single_connection_only:
        ipt->single_source = value;
        break;
    case defs::options::multiple_connections_allowed:
        ipt->single_source = !value;
        break;
    case defs::options::strict_type_checking:
        ipt->strict_type_matching = value;
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool InterfaceInfo::setPublicationProperty (interface_handle id, int option, bool value)
{
    auto pub = getPublication (id);
    switch (option)
    {
    case defs::options::handle_only_transmit_on_change:
        pub->only_update_on_change = value;
        break;
    case defs::options::connection_required:
        pub->required = value;
        break;
    case defs::options::connection_optional:
        pub->required = !value;
        break;
    case defs::options::single_connection_only:
        pub->single_destination = value;
        break;
    case defs::options::multiple_connections_allowed:
        pub->single_destination = !value;
        break;
    case defs::options::buffer_data:
        pub->buffer_data = value;
        break;
    default:
        return false;
        break;
    }
    return true;
}

bool InterfaceInfo::setEndpointProperty (interface_handle /*id*/, int /*option*/, bool /*value*/)
{
    // auto ept = getEndpoint (id);
    // currently no properties on endpoints
    return false;
}

bool InterfaceInfo::getInputProperty (interface_handle id, int option) const
{
    auto ipt = getInput (id);
    switch (option)
    {
    case defs::options::ignore_interrupts:
        return ipt->not_interruptible;
        break;
    case defs::options::handle_only_update_on_change:
        return ipt->only_update_on_change;
        break;
    case defs::options::connection_required:
        return ipt->required;
        break;
    case defs::options::connection_optional:
        return !ipt->required;
        break;
    case defs::options::single_connection_only:
        return ipt->single_source;
        break;
    case defs::options::multiple_connections_allowed:
        return !ipt->single_source;
        break;
    case defs::options::strict_type_checking:
        return ipt->strict_type_matching;
        break;
    default:
        return false;
        break;
    }
}

bool InterfaceInfo::getPublicationProperty (interface_handle id, int option) const
{
    auto pub = getPublication (id);
    switch (option)
    {
    case defs::options::handle_only_transmit_on_change:
        return pub->only_update_on_change;
        break;
    case defs::options::connection_required:
        return pub->required;
        break;
    case defs::options::connection_optional:
        return !pub->required;
        break;
    case defs::options::single_connection_only:
        return pub->single_destination;
        break;
    case defs::options::multiple_connections_allowed:
        return !pub->single_destination;
        break;
    case defs::options::buffer_data:
        return pub->buffer_data;
        break;
    default:
        return false;
        break;
    }
}

bool InterfaceInfo::getEndpointProperty (interface_handle /*id*/, int /*option*/) const
{
    // auto ept = getEndpoint (id);
    // currently no properties on endpoints
    return false;
}

std::vector<std::pair<int, std::string>> InterfaceInfo::checkInterfacesForIssues ()
{
    std::vector<std::pair<int, std::string>> issues;
    auto ihandle = inputs.lock ();
    for (auto &ipt : ihandle)
    {
        if (ipt->required)
        {
            if (!ipt->has_target)
            {
                issues.emplace_back (helics::defs::errors::connection_failure,
                                     fmt::format ("Input {} is required but has no connection", ipt->key));
            }
        }
        if (ipt->single_source)
        {
            if (ipt->input_sources.size () > 1)
            {
                issues.emplace_back (
                  helics::defs::errors::connection_failure,
                  fmt::format ("Input {} is single source only but has more than one connection", ipt->key));
            }
        }
        for (auto &source : ipt->source_types)
        {
            if (!checkTypeMatch (ipt->type, source.first, ipt->strict_type_matching))
            {
                issues.emplace_back (
                  helics::defs::errors::connection_failure,
                  fmt::format ("Input {} source has mismatched types {} is not compatible with {}", ipt->key,
                               ipt->type, source.first));
            }
        }
    }
    ihandle.unlock ();
    auto phandle = publications.lock ();
    for (auto &pub : phandle)
    {
        if (pub->required)
        {
            if (pub->subscribers.empty ())
            {
                issues.emplace_back (helics::defs::errors::connection_failure,
                                     fmt::format ("Publication {} is required but has no subscribers", pub->key));
            }
        }
        if (pub->single_destination)
        {
            if (pub->subscribers.size () > 1)
            {
                issues.emplace_back (
                  helics::defs::errors::connection_failure,
                  fmt::format ("Publication {} is single source only but has more than one connection", pub->key));
            }
        }
    }
    phandle.unlock ();
    return issues;
}

}  // namespace helics
