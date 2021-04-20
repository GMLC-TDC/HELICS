/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InterfaceInfo.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
#include "helics_definitions.hpp"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace helics {
void InterfaceInfo::createPublication(interface_handle handle,
                                      const std::string& key,
                                      const std::string& type,
                                      const std::string& units)
{
    publications.lock()->insert(key, handle, global_handle{global_id, handle}, key, type, units);
}

void InterfaceInfo::createInput(interface_handle handle,
                                const std::string& key,
                                const std::string& type,
                                const std::string& units)
{
    auto ciHandle = inputs.lock();
    ciHandle->insert(key, handle, global_handle{global_id, handle}, key, type, units);
    ciHandle->back()->only_update_on_change = only_update_on_change;
}

void InterfaceInfo::createEndpoint(interface_handle handle,
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
        // ip is a reference to a unique_ptr
        for (auto& ip : inputs.lock()) {  // NOLINT(readability-qualified-auto)
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

const InputInfo* InterfaceInfo::getInput(const std::string& inputName) const
{
    return inputs.lock_shared()->find(inputName);
}

const InputInfo* InterfaceInfo::getInput(interface_handle handle) const
{
    return inputs.lock()->find(handle);
}

InputInfo* InterfaceInfo::getInput(const std::string& inputName)
{
    return inputs.lock()->find(inputName);
}

InputInfo* InterfaceInfo::getInput(interface_handle handle)
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

bool InterfaceInfo::setInputProperty(interface_handle id, int32_t option, int32_t value)
{
    auto* ipt = getInput(id);
    if (ipt == nullptr) {
        return false;
    }
    bool bvalue = (value != 0);
    switch (option) {
        case defs::options::ignore_interrupts:
            ipt->not_interruptible = bvalue;
            break;
        case defs::options::handle_only_update_on_change:
            ipt->only_update_on_change = bvalue;
            break;
        case defs::options::connection_required:
            ipt->required = bvalue;
            break;
        case defs::options::connection_optional:
            ipt->required = !bvalue;
            break;
        case defs::options::single_connection_only:
            ipt->required_connnections = bvalue ? 1 : 0;
            break;
        case defs::options::multiple_connections_allowed:
            ipt->required_connnections = bvalue ? 0 : 1;
            break;
        case defs::options::strict_type_checking:
            ipt->strict_type_matching = bvalue;
            break;
        case defs::options::ignore_unit_mismatch:
            ipt->ignore_unit_mismatch = bvalue;
            break;
        case defs::options::connections:
            ipt->required_connnections = value;
            break;
        case defs::options::input_priority_location:
            ipt->priority_sources.push_back(value);
            break;
        case defs::options::clear_priority_list:
            ipt->priority_sources.clear();
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool InterfaceInfo::setPublicationProperty(interface_handle id, int32_t option, int32_t value)
{
    auto* pub = getPublication(id);
    if (pub == nullptr) {
        return false;
    }
    bool bvalue = (value != 0);
    switch (option) {
        case defs::options::handle_only_transmit_on_change:
            pub->only_update_on_change = bvalue;
            break;
        case defs::options::connection_required:
            pub->required = bvalue;
            break;
        case defs::options::connection_optional:
            pub->required = !bvalue;
            break;
        case defs::options::single_connection_only:
            pub->required_connections = bvalue ? 1 : 0;
            break;
        case defs::options::multiple_connections_allowed:
            pub->required_connections = !bvalue ? 0 : 1;
            break;
        case defs::options::buffer_data:
            pub->buffer_data = bvalue;
            break;
        case defs::options::connections:
            pub->required_connections = value;
            break;
        default:
            return false;
            break;
    }
    return true;
}

bool InterfaceInfo::setEndpointProperty(interface_handle id, int32_t option, int32_t value)
{
    auto* ept = getEndpoint(id);
    if (ept == nullptr) {
        return false;
    }
    bool bvalue = (value != 0);
    switch (option) {
        case defs::options::connection_required:
            ept->required = bvalue;
            break;
        case defs::options::connection_optional:
            ept->required = !bvalue;
            break;
        default:
            return false;
            break;
    }
    return true;
}

int32_t InterfaceInfo::getInputProperty(interface_handle id, int32_t option) const
{
    const auto* ipt = getInput(id);
    if (ipt == nullptr) {
        return 0;
    }
    bool flagval = false;
    switch (option) {
        case defs::options::ignore_interrupts:
            flagval = ipt->not_interruptible;
            break;
        case defs::options::handle_only_update_on_change:
            flagval = ipt->only_update_on_change;
            break;
        case defs::options::connection_required:
            flagval = ipt->required;
            break;
        case defs::options::connection_optional:
            flagval = !ipt->required;
            break;
        case defs::options::single_connection_only:
            flagval = (ipt->required_connnections == 1);
            break;
        case defs::options::multiple_connections_allowed:
            flagval = (ipt->required_connnections != 1);
            break;
        case defs::options::strict_type_checking:
            flagval = ipt->strict_type_matching;
            break;
        case defs::options::connections:
            return static_cast<int32_t>(ipt->input_sources.size());
        case defs::options::input_priority_location:
            return ipt->priority_sources.empty() ? -1 : ipt->priority_sources.back();
        case defs::options::clear_priority_list:
            flagval = ipt->priority_sources.empty();
            break;
        default:
            break;
    }
    return flagval ? 1 : 0;
}

int32_t InterfaceInfo::getPublicationProperty(interface_handle id, int32_t option) const
{
    const auto* pub = getPublication(id);
    if (pub == nullptr) {
        return 0;
    }
    bool flagval = false;
    switch (option) {
        case defs::options::handle_only_transmit_on_change:
            flagval = pub->only_update_on_change;
            break;
        case defs::options::connection_required:
            flagval = pub->required;
            break;
        case defs::options::connection_optional:
            flagval = !pub->required;
            break;
        case defs::options::single_connection_only:
            flagval = (pub->required_connections == 1);
            break;
        case defs::options::multiple_connections_allowed:
            flagval = pub->required_connections != 1;
            break;
        case defs::options::buffer_data:
            flagval = pub->buffer_data;
            break;
        case defs::options::connections:
            return static_cast<int32_t>(pub->subscribers.size());
        default:
            break;
    }
    return flagval ? 1 : 0;
}

int32_t InterfaceInfo::getEndpointProperty(interface_handle id, int32_t option) const
{
    const auto* ept = getEndpoint(id);
    if (ept == nullptr) {
        return 0;
    }
    bool flagval = false;
    switch (option) {
        case defs::options::connection_required:
            flagval = ept->required;
            break;
        case defs::options::connection_optional:
            flagval = !ept->required;
            break;
        default:
            break;
    }
    return flagval ? 1 : 0;
}

std::vector<std::pair<int, std::string>> InterfaceInfo::checkInterfacesForIssues()
{
    std::vector<std::pair<int, std::string>> issues;
    auto ihandle = inputs.lock();
    for (const auto& ipt : ihandle) {
        if (ipt->required) {
            if (!ipt->has_target) {
                issues.emplace_back(helics::defs::errors::connection_failure,
                                    fmt::format("Input {} is required but has no connection",
                                                ipt->key));
            }
        }
        if (ipt->required_connnections > 0) {
            if (ipt->input_sources.size() != static_cast<size_t>(ipt->required_connnections)) {
                if (ipt->required_connnections == 1) {
                    issues.emplace_back(
                        helics::defs::errors::connection_failure,
                        fmt::format(
                            "Input {} is single source only but has more than one connection",
                            ipt->key));
                } else {
                    issues.emplace_back(
                        helics::defs::errors::connection_failure,
                        fmt::format("Input {} requires {} connections but{} {} were made",
                                    ipt->key,
                                    ipt->required_connnections,
                                    (ipt->input_sources.size() <
                                     static_cast<size_t>(ipt->required_connnections)) ?
                                        " only" :
                                        "",
                                    ipt->input_sources.size()));
                }
            }
        }
        for (auto& source : ipt->source_info) {
            if (!checkTypeMatch(ipt->type, source.type, ipt->strict_type_matching)) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Input \"{}\" source has mismatched types: {} is not compatible with {}",
                        ipt->key,
                        ipt->type,
                        source.type));
            }
            if ((!ipt->ignore_unit_mismatch) &&
                (!checkUnitMatch(ipt->units, source.units, false))) {
                issues.emplace_back(
                    helics::defs::errors::connection_failure,
                    fmt::format(
                        "Input \"{}\" source has incompatible unit: {} is not convertible to {}",
                        ipt->key,
                        source.units,
                        ipt->units));
            }
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    for (const auto& pub : phandle) {
        if (pub->required) {
            if (pub->subscribers.empty()) {
                issues.emplace_back(helics::defs::errors::connection_failure,
                                    fmt::format("Publication {} is required but has no subscribers",
                                                pub->key));
            }
        }
        if (pub->required_connections > 0) {
            if (pub->subscribers.size() != static_cast<size_t>(pub->required_connections)) {
                if (pub->required_connections == 1) {
                    issues.emplace_back(
                        helics::defs::errors::connection_failure,
                        fmt::format(
                            "Publication {} is single source only but has more than one connection",
                            pub->key));
                } else {
                    issues.emplace_back(
                        helics::defs::errors::connection_failure,
                        fmt::format("Publication {} requires {} connections but only {} are made",
                                    pub->key,
                                    pub->required_connections,
                                    pub->subscribers.size()));
                }
            }
        }
    }
    phandle.unlock();
    return issues;
}

void InterfaceInfo::generateInferfaceConfig(Json::Value& base) const
{
    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        base["inputs"] = Json::arrayValue;
        for (const auto& ipt : ihandle) {
            if (!ipt->key.empty()) {
                Json::Value ibase;
                ibase["key"] = ipt->key;
                if (!ipt->type.empty()) {
                    ibase["type"] = ipt->type;
                }
                if (!ipt->units.empty()) {
                    ibase["units"] = ipt->units;
                }
                base["inputs"].append(std::move(ibase));
            }
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        base["publications"] = Json::arrayValue;
        for (const auto& pub : phandle) {
            if (!pub->key.empty()) {
                Json::Value pbase;
                pbase["key"] = pub->key;
                if (!pub->type.empty()) {
                    pbase["type"] = pub->type;
                }
                if (!pub->units.empty()) {
                    pbase["units"] = pub->units;
                }
                base["publications"].append(std::move(pbase));
            }
        }
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        base["endpoints"] = Json::arrayValue;
        for (const auto& ept : ehandle) {
            if (!ept->key.empty()) {
                Json::Value ebase;
                ebase["key"] = ept->key;
                if (!ept->type.empty()) {
                    ebase["type"] = ept->type;
                }
                base["endpoints"].append(std::move(ebase));
            }
        }
    }
    phandle.unlock();
    base["extra"] = "configuration";
}

void InterfaceInfo::GenerateDataFlowGraph(Json::Value& base) const
{
    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        base["inputs"] = Json::arrayValue;
        for (const auto& ipt : ihandle) {
            Json::Value ibase;
            if (!ipt->key.empty()) {
                ibase["key"] = ipt->key;
            }
            ibase["federate"] = ipt->id.fed_id.baseValue();
            ibase["handle"] = ipt->id.handle.baseValue();
            if (!ipt->input_sources.empty()) {
                ibase["sources"] = Json::arrayValue;
                for (auto& source : ipt->input_sources) {
                    Json::Value sid;
                    sid["federate"] = source.fed_id.baseValue();
                    sid["handle"] = source.handle.baseValue();
                    ibase["sources"].append(sid);
                }
            }
            base["inputs"].append(std::move(ibase));
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        base["publications"] = Json::arrayValue;
        for (const auto& pub : phandle) {
            Json::Value pbase;
            if (!pub->key.empty()) {
                pbase["key"] = pub->key;
            }
            pbase["federate"] = pub->id.fed_id.baseValue();
            pbase["handle"] = pub->id.handle.baseValue();
            if (!pub->subscribers.empty()) {
                pbase["targets"] = Json::arrayValue;
                for (auto& target : pub->subscribers) {
                    Json::Value sid;
                    sid["federate"] = target.fed_id.baseValue();
                    sid["handle"] = target.handle.baseValue();
                    pbase["targets"].append(sid);
                }
            }
            base["publications"].append(std::move(pbase));
        }
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        base["endpoints"] = Json::arrayValue;
        for (const auto& ept : ehandle) {
            Json::Value ebase;
            ebase["federate"] = ept->id.fed_id.baseValue();
            ebase["handle"] = ept->id.handle.baseValue();
            if (!ept->key.empty()) {
                ebase["key"] = ept->key;
            }
            base["endpoints"].append(std::move(ebase));
        }
    }
    ehandle.unlock();
}

}  // namespace helics
