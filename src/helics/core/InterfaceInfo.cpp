/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InterfaceInfo.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"

#include <fmt/format.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace helics {

void InterfaceInfo::reset()
{
    publications.lock()->clear();
    inputs.lock()->clear();
    endpoints.lock()->clear();
    only_update_on_change = false;
}

void InterfaceInfo::createPublication(InterfaceHandle handle,
                                      std::string_view key,
                                      std::string_view type,
                                      std::string_view units,
                                      std::uint16_t flags)
{
    auto cpHandle = publications.lock();
    cpHandle->insert(std::string(key), handle, GlobalHandle{global_id, handle}, key, type, units);
    if (checkActionFlag(flags, required_flag)) {
        cpHandle->back()->setProperty(defs::Options::CONNECTION_REQUIRED, 1);
    }
    if (checkActionFlag(flags, optional_flag)) {
        cpHandle->back()->setProperty(defs::Options::CONNECTION_OPTIONAL, 1);
    }
    if (checkActionFlag(flags, buffer_data_flag)) {
        cpHandle->back()->setProperty(defs::Options::BUFFER_DATA, 1);
    }
    if (checkActionFlag(flags, only_transmit_on_change_flag)) {
        cpHandle->back()->setProperty(defs::Options::HANDLE_ONLY_TRANSMIT_ON_CHANGE, 1);
    }
    if (checkActionFlag(flags, single_connection_flag)) {
        cpHandle->back()->setProperty(defs::Options::SINGLE_CONNECTION_ONLY, 1);
    }
}

void InterfaceInfo::createInput(InterfaceHandle handle,
                                std::string_view key,
                                std::string_view type,
                                std::string_view units,
                                std::uint16_t flags)
{
    auto ciHandle = inputs.lock();
    ciHandle->insert(std::string(key), handle, GlobalHandle{global_id, handle}, key, type, units);
    ciHandle->back()->only_update_on_change = only_update_on_change;

    if (checkActionFlag(flags, required_flag)) {
        ciHandle->back()->setProperty(defs::Options::CONNECTION_REQUIRED, 1);
    }
    if (checkActionFlag(flags, optional_flag)) {
        ciHandle->back()->setProperty(defs::Options::CONNECTION_OPTIONAL, 1);
    }
    if (checkActionFlag(flags, only_update_on_change_flag)) {
        ciHandle->back()->setProperty(defs::Options::HANDLE_ONLY_UPDATE_ON_CHANGE, 1);
    }
    if (checkActionFlag(flags, single_connection_flag)) {
        ciHandle->back()->setProperty(defs::Options::SINGLE_CONNECTION_ONLY, 1);
    }
}

void InterfaceInfo::createEndpoint(InterfaceHandle handle,
                                   std::string_view endpointName,
                                   std::string_view type,
                                   std::uint16_t flags)
{
    auto ceHandle = endpoints.lock();
    ceHandle->insert(
        std::string(endpointName), handle, GlobalHandle{global_id, handle}, endpointName, type);
    if (checkActionFlag(flags, required_flag)) {
        ceHandle->back()->setProperty(defs::Options::CONNECTION_REQUIRED, 1);
    }
    if (checkActionFlag(flags, optional_flag)) {
        ceHandle->back()->setProperty(defs::Options::CONNECTION_OPTIONAL, 1);
    }
    if (checkActionFlag(flags, targeted_flag)) {
        ceHandle->back()->targetedEndpoint = true;
    }
    if (checkActionFlag(flags, single_connection_flag)) {
        ceHandle->back()->setProperty(defs::Options::SINGLE_CONNECTION_ONLY, 1);
    }
    if (checkActionFlag(flags, source_only_flag)) {
        ceHandle->back()->setProperty(defs::Options::SEND_ONLY, 1);
    }
    if (checkActionFlag(flags, receive_only_flag)) {
        ceHandle->back()->setProperty(defs::Options::RECEIVE_ONLY, 1);
    }
}

void InterfaceInfo::setChangeUpdateFlag(bool updateFlag)
{
    if (updateFlag != only_update_on_change) {
        only_update_on_change = updateFlag;
        // input is a reference to a unique_ptr
        for (auto& input : inputs.lock()) {  // NOLINT(readability-qualified-auto)
            input->only_update_on_change = updateFlag;
        }
    }
}

const PublicationInfo* InterfaceInfo::getPublication(const std::string& pubName) const
{
    return publications.lock_shared()->find(pubName);
}

const PublicationInfo* InterfaceInfo::getPublication(InterfaceHandle handle) const
{
    return publications.lock()->find(handle);
}

PublicationInfo* InterfaceInfo::getPublication(const std::string& pubName)
{
    return publications.lock()->find(pubName);
}

PublicationInfo* InterfaceInfo::getPublication(InterfaceHandle handle)
{
    return publications.lock()->find(handle);
}

const InputInfo* InterfaceInfo::getInput(const std::string& inputName) const
{
    return inputs.lock_shared()->find(inputName);
}

const InputInfo* InterfaceInfo::getInput(InterfaceHandle handle) const
{
    return inputs.lock()->find(handle);
}

InputInfo* InterfaceInfo::getInput(const std::string& inputName)
{
    return inputs.lock()->find(inputName);
}

InputInfo* InterfaceInfo::getInput(InterfaceHandle handle)
{
    return inputs.lock()->find(handle);
}

const EndpointInfo* InterfaceInfo::getEndpoint(const std::string& endpointName) const
{
    return endpoints.lock_shared()->find(endpointName);
}

const EndpointInfo* InterfaceInfo::getEndpoint(InterfaceHandle handle) const
{
    return endpoints.lock_shared()->find(handle);
}

EndpointInfo* InterfaceInfo::getEndpoint(const std::string& endpointName)
{
    return endpoints.lock()->find(endpointName);
}

EndpointInfo* InterfaceInfo::getEndpoint(InterfaceHandle handle)
{
    return endpoints.lock()->find(handle);
}

bool InterfaceInfo::setInputProperty(InterfaceHandle hid, int32_t option, int32_t value)
{
    auto* ipt = getInput(hid);
    if (ipt == nullptr) {
        return false;
    }
    ipt->setProperty(option, value);
    return true;
}

bool InterfaceInfo::setPublicationProperty(InterfaceHandle hid, int32_t option, int32_t value)
{
    auto* pub = getPublication(hid);
    if (pub == nullptr) {
        return false;
    }
    pub->setProperty(option, value);
    return true;
}

bool InterfaceInfo::setEndpointProperty(InterfaceHandle hid, int32_t option, int32_t value)
{
    auto* ept = getEndpoint(hid);
    if (ept == nullptr) {
        return false;
    }
    ept->setProperty(option, value);
    ept->setProperty(option, value);
    return true;
}

int32_t InterfaceInfo::getInputProperty(InterfaceHandle hid, int32_t option) const
{
    const auto* ipt = getInput(hid);
    if (ipt == nullptr) {
        return 0;
    }
    return ipt->getProperty(option);
}

int32_t InterfaceInfo::getPublicationProperty(InterfaceHandle hid, int32_t option) const
{
    const auto* pub = getPublication(hid);
    if (pub == nullptr) {
        return 0;
    }
    return pub->getProperty(option);
}

int32_t InterfaceInfo::getEndpointProperty(InterfaceHandle hid, int32_t option) const
{
    const auto* ept = getEndpoint(hid);
    if (ept == nullptr) {
        return 0;
    }
    return ept->getProperty(option);
}

std::vector<std::pair<int, std::string>> InterfaceInfo::checkInterfacesForIssues()
{
    std::vector<std::pair<int, std::string>> issues;
    auto ihandle = inputs.lock();
    for (const auto& ipt : ihandle) {
        if (ipt->required) {
            if (!ipt->has_target) {
                issues.emplace_back(helics::defs::Errors::CONNECTION_FAILURE,
                                    fmt::format("Input {} is required but has no connection",
                                                ipt->key));
            }
        }
        if (ipt->required_connnections > 0) {
            if (ipt->input_sources.size() != static_cast<size_t>(ipt->required_connnections)) {
                if (ipt->required_connnections == 1) {
                    issues.emplace_back(
                        helics::defs::Errors::CONNECTION_FAILURE,
                        fmt::format(
                            "Input {} is single source only but has more than one connection",
                            ipt->key));
                } else {
                    issues.emplace_back(
                        helics::defs::Errors::CONNECTION_FAILURE,
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
                    helics::defs::Errors::CONNECTION_FAILURE,
                    fmt::format(
                        "Input \"{}\" source has mismatched types: {} is not compatible with {}",
                        ipt->key,
                        ipt->type,
                        source.type));
            }
            if ((!ipt->ignore_unit_mismatch) &&
                (!checkUnitMatch(ipt->units, source.units, false))) {
                issues.emplace_back(
                    helics::defs::Errors::CONNECTION_FAILURE,
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
                issues.emplace_back(helics::defs::Errors::CONNECTION_FAILURE,
                                    fmt::format("Publication {} is required but has no subscribers",
                                                pub->key));
            }
        }
        if (pub->requiredConnections > 0) {
            if (pub->subscribers.size() != static_cast<size_t>(pub->requiredConnections)) {
                if (pub->requiredConnections == 1) {
                    issues.emplace_back(
                        helics::defs::Errors::CONNECTION_FAILURE,
                        fmt::format(
                            "Publication {} is single source only but has more than one connection",
                            pub->key));
                } else {
                    issues.emplace_back(
                        helics::defs::Errors::CONNECTION_FAILURE,
                        fmt::format("Publication {} requires {} connections but only {} are made",
                                    pub->key,
                                    pub->requiredConnections,
                                    pub->subscribers.size()));
                }
            }
        }
    }
    phandle.unlock();
    auto ehandle = endpoints.lock();
    for (const auto& ept : ehandle) {
        ept->checkInterfacesForIssues(issues);
    }
    ehandle.unlock();
    return issues;
}

void InterfaceInfo::getUnconnectedInterfaces(nlohmann::json& base) const
{
    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        base["unconnected_inputs"] = nlohmann::json::array();
        base["connected_inputs"] = nlohmann::json::array();
        for (const auto& ipt : ihandle) {
            if (!ipt->key.empty()) {
                if (!ipt->has_target) {
                    base["unconnected_inputs"].push_back(ipt->key);
                } else {
                    base["connected_inputs"].push_back(ipt->key);
                }
            }
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        base["unconnected_publications"] = nlohmann::json::array();
        base["connected_publications"] = nlohmann::json::array();
        for (const auto& pub : phandle) {
            if (!pub->key.empty()) {
                if (pub->subscribers.empty()) {
                    base["unconnected_publications"].push_back(pub->key);
                } else {
                    base["connected_publications"].push_back(pub->key);
                }
            }
        }
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        base["unconnected_source_endpoints"] = nlohmann::json::array();
        base["unconnected_destination_endpoints"] = nlohmann::json::array();
        base["connected_endpoints"] = nlohmann::json::array();
        for (const auto& ept : ehandle) {
            if (!ept->key.empty()) {
                if (ept->targetedEndpoint) {
                    if (!ept->hasSource()) {
                        base["unconnected_target_endpoints"].push_back(ept->key);
                    }
                    if (!ept->hasTarget()) {
                        base["unconnected_source_endpoints"].push_back(ept->key);
                    }
                    if (ept->hasConnection()) {
                        base["connected_endpoints"].push_back(ept->key);
                    }
                } else {
                    base["connected_endpoints"].push_back(ept->key);
                }
            }
        }
    }
    ehandle.unlock();
}

void InterfaceInfo::generateInferfaceConfig(nlohmann::json& base) const
{
    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        base["inputs"] = nlohmann::json::array();
        for (const auto& ipt : ihandle) {
            if (!ipt->key.empty()) {
                nlohmann::json ibase;
                ibase["key"] = ipt->key;
                if (!ipt->type.empty()) {
                    ibase["type"] = ipt->type;
                }
                if (!ipt->units.empty()) {
                    ibase["units"] = ipt->units;
                }
                base["inputs"].push_back(std::move(ibase));
            }
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        base["publications"] = nlohmann::json::array();
        for (const auto& pub : phandle) {
            if (!pub->key.empty()) {
                nlohmann::json pbase;
                pbase["key"] = pub->key;
                if (!pub->type.empty()) {
                    pbase["type"] = pub->type;
                }
                if (!pub->units.empty()) {
                    pbase["units"] = pub->units;
                }
                base["publications"].push_back(std::move(pbase));
            }
        }
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        base["endpoints"] = nlohmann::json::array();
        for (const auto& ept : ehandle) {
            if (!ept->key.empty()) {
                nlohmann::json ebase;
                ebase["key"] = ept->key;
                if (!ept->type.empty()) {
                    ebase["type"] = ept->type;
                }
                base["endpoints"].push_back(std::move(ebase));
            }
        }
    }
    ehandle.unlock();
    base["extra"] = "configuration";
}

void InterfaceInfo::generateDataFlowGraph(nlohmann::json& base) const
{
    auto ihandle = inputs.lock_shared();
    if (ihandle->size() > 0) {
        for (const auto& ipt : ihandle) {
            nlohmann::json ibase;
            if (!ipt->key.empty()) {
                ibase["key"] = ipt->key;
            }
            ibase["federate"] = ipt->id.fed_id.baseValue();
            ibase["handle"] = ipt->id.handle.baseValue();
            if (!ipt->input_sources.empty()) {
                ibase["sources"] = nlohmann::json::array();
                for (auto& source : ipt->input_sources) {
                    nlohmann::json sid;
                    sid["federate"] = source.fed_id.baseValue();
                    sid["handle"] = source.handle.baseValue();

                    ibase["sources"].push_back(sid);
                }
            }
            base["inputs"].push_back(std::move(ibase));
        }
    }
    ihandle.unlock();
    auto phandle = publications.lock();
    if (phandle->size() > 0) {
        base["publications"] = nlohmann::json::array();
        for (const auto& pub : phandle) {
            nlohmann::json pbase;
            if (!pub->key.empty()) {
                pbase["key"] = pub->key;
            }
            pbase["federate"] = pub->id.fed_id.baseValue();
            pbase["handle"] = pub->id.handle.baseValue();
            if (!pub->subscribers.empty()) {
                pbase["targets"] = nlohmann::json::array();
                for (auto& target : pub->subscribers) {
                    nlohmann::json sid;
                    sid["federate"] = target.id.fed_id.baseValue();
                    sid["handle"] = target.id.handle.baseValue();
                    if (!target.key.empty()) {
                        sid["key"] = target.key;
                    }
                    pbase["targets"].push_back(sid);
                }
            }
            base["publications"].push_back(std::move(pbase));
        }
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();
    if (ehandle->size() > 0) {
        base["endpoints"] = nlohmann::json::array();
        for (const auto& ept : ehandle) {
            nlohmann::json ebase;
            ebase["federate"] = ept->id.fed_id.baseValue();
            ebase["handle"] = ept->id.handle.baseValue();
            if (!ept->key.empty()) {
                ebase["key"] = ept->key;
            }
            base["endpoints"].push_back(std::move(ebase));
        }
    }
    ehandle.unlock();
}

void InterfaceInfo::disconnectFederate(GlobalFederateId fedToDisconnect, Time disconnectTime)
{
    if (disconnectTime < cBigTime) {
        auto ihandle = inputs.lock_shared();
        for (auto& ipt : ihandle) {
            ipt->disconnectFederate(fedToDisconnect, disconnectTime);
        }
        ihandle.unlock();
    }

    auto phandle = publications.lock();

    for (auto& pub : phandle) {
        pub->disconnectFederate(fedToDisconnect);
    }
    phandle.unlock();

    auto ehandle = endpoints.lock_shared();

    for (auto& ept : ehandle) {
        ept->disconnectFederate(fedToDisconnect);
    }
    ehandle.unlock();
}

}  // namespace helics
