/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InterfaceInfo.hpp"

#include "../common/fmt_format.h"
#include "helics_definitions.hpp"

#include <sstream>

namespace helics {
void InterfaceInfo::createPublication(
    interface_handle handle,
    const std::string& key,
    const std::string& type,
    const std::string& units)
{
    publications.lock()->insert(key, handle, global_handle{global_id, handle}, key, type, units);
}

void InterfaceInfo::createInput(
    interface_handle handle,
    const std::string& key,
    const std::string& type,
    const std::string& units)
{
    auto ciHandle = inputs.lock();
    ciHandle->insert(key, handle, global_handle{global_id, handle}, key, type, units);
    ciHandle->back()->only_update_on_change = only_update_on_change;
}

void InterfaceInfo::createEndpoint(
    interface_handle handle,
    const std::string& endpointName,
    const std::string& type)
{
    endpoints.lock()->insert(
        endpointName, handle, global_handle{global_id, handle}, endpointName, type);
}

void InterfaceInfo::setChangeUpdateFlag(bool updateFlag)
{
    if (updateFlag != only_update_on_change) {
        only_update_on_change = updateFlag;
        for (auto& ip : inputs.lock()) {
            ip->only_update_on_change = updateFlag;
        }
    }
}

const PublicationInfo* InterfaceInfo::getPublication(const std::string& pubName) const
{
    return publications.lock_shared()->find(pubName);
}

const PublicationInfo* InterfaceInfo::getPublication(interface_handle handle) const
{
    return publications.lock()->find(handle);
}

PublicationInfo* InterfaceInfo::getPublication(const std::string& pubName)
{
    return publications.lock()->find(pubName);
}

PublicationInfo* InterfaceInfo::getPublication(interface_handle handle)
{
    return publications.lock()->find(handle);
}

const NamedInputInfo* InterfaceInfo::getInput(const std::string& inputName) const
{
    return inputs.lock_shared()->find(inputName);
}

const NamedInputInfo* InterfaceInfo::getInput(interface_handle handle) const
{
    return inputs.lock()->find(handle);
}

NamedInputInfo* InterfaceInfo::getInput(const std::string& inputName)
{
    return inputs.lock()->find(inputName);
}

NamedInputInfo* InterfaceInfo::getInput(interface_handle handle)
{
    return inputs.lock()->find(handle);
}

const EndpointInfo* InterfaceInfo::getEndpoint(const std::string& endpointName) const
{
    return endpoints.lock_shared()->find(endpointName);
}

const EndpointInfo* InterfaceInfo::getEndpoint(interface_handle handle) const
{
    return endpoints.lock_shared()->find(handle);
}

EndpointInfo* InterfaceInfo::getEndpoint(const std::string& endpointName)
{
    return endpoints.lock()->find(endpointName);
}

EndpointInfo* InterfaceInfo::getEndpoint(interface_handle handle)
{
    return endpoints.lock()->find(handle);
}

bool InterfaceInfo::setInputProperty(interface_handle id, int option, bool value)
{
    auto ipt = getInput(id);
    if (ipt == nullptr) {
        return false;
    }
    switch (option) {
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
        case defs::options::ignore_unit_mismatch:
            ipt->ignore_unit_mismatch = value;
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool InterfaceInfo::setPublicationProperty(interface_handle id, int option, bool value)
{
    auto pub = getPublication(id);
    if (pub == nullptr) {
        return false;
    }
    switch (option) {
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

bool InterfaceInfo::setEndpointProperty(interface_handle /*id*/, int /*option*/, bool /*value*/)
{
    // auto ept = getEndpoint (id);
    // currently no properties on endpoints
    return false;
}

bool InterfaceInfo::getInputProperty(interface_handle id, int option) const
{
    auto ipt = getInput(id);
    if (ipt == nullptr) {
        return false;
    }
    switch (option) {
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

bool InterfaceInfo::getPublicationProperty(interface_handle id, int option) const
{
    auto pub = getPublication(id);
    if (pub == nullptr) {
        return false;
    }
    switch (option) {
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

bool InterfaceInfo::getEndpointProperty(interface_handle /*id*/, int /*option*/) const
{
    // auto ept = getEndpoint (id);
    // currently no properties on endpoints
    return false;
}

std::vector<std::pair<int, std::string>> InterfaceInfo::checkInterfacesForIssues()
{
    std::vector<std::pair<int, std::string>> issues;
    auto ihandle = inputs.lock();
    for (auto& ipt : ihandle) {
        if (ipt->required) {
            if (!ipt->has_target) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format("Input {} is required but has no connection", ipt->key));
            }
        }
        if (ipt->single_source) {
            if (ipt->input_sources.size() > 1) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Input {} is single source only but has more than one connection",
                        ipt->key));
            }
        }
        for (auto& source : ipt->source_info) {
            if (!checkTypeMatch(ipt->type, std::get<1>(source), ipt->strict_type_matching)) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Input \"{}\" source has mismatched types: {} is not compatible with {}",
                        ipt->key,
                        ipt->type,
                        std::get<1>(source)));
            }
            if ((!ipt->ignore_unit_mismatch) &&
                (!checkUnitMatch(ipt->units, std::get<2>(source), false))) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Input \"{}\" source has incompatible unit: {} is not convertible to {}",
                        ipt->key,
                        std::get<2>(source),
                        ipt->units));
            }
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    for (auto& pub : phandle) {
        if (pub->required) {
            if (pub->subscribers.empty()) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format("Publication {} is required but has no subscribers", pub->key));
            }
        }
        if (pub->single_destination) {
            if (pub->subscribers.size() > 1) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Publication {} is single source only but has more than one connection",
                        pub->key));
            }
        }
    }
    phandle.unlock();
    return issues;
}

std::string InterfaceInfo::generateInferfaceConfig() const
{
    std::ostringstream s;

    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        s << "\"inputs\":[";
        bool first = true;
        for (auto& ipt : ihandle) {
            if (!ipt->key.empty()) {
                if (!first) {
                    s << ',';
                }
                first = false;
                s << "{\n \"key\":\"" << ipt->key << "\"";
                if (!ipt->type.empty()) {
                    s << ",\n \"type\":\"" << ipt->type << "\"";
                }
                if (!ipt->units.empty()) {
                    s << ",\n \"units\":\"" << ipt->units << "\"";
                }
                s << "\n}";
            }
        }
        s << "],";
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        s << "\n\"publications\":[";
        bool first = true;
        for (auto& pub : phandle) {
            if (!first) {
                s << ',';
            }
            first = false;
            s << "{\n \"key\":\"" << pub->key << "\"";
            if (!pub->type.empty()) {
                s << ",\n \"type\":\"" << pub->type << "\"";
            }
            if (!pub->units.empty()) {
                s << ",\n \"units\":\"" << pub->units << "\"";
            }
            s << "\n}";
        }
        s << "],";
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        s << "\n\"endpoints\":[";
        bool first = true;
        for (auto& ept : ehandle) {
            if (!first) {
                s << ',';
            }
            first = false;

            s << "{\n \"key\":\"" << ept->key << "\"";
            if (!ept->type.empty()) {
                s << ",\n \"type\":\"" << ept->type << "\"";
            }
            s << "\n}";
        }
        s << "\n],";
    }
    phandle.unlock();
    s << "\n\"extra\":\"configuration\"";
    return s.str();
}
} // namespace helics
