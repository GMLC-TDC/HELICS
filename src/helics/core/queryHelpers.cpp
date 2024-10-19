/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "queryHelpers.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "FederateState.hpp"
#include "HandleManager.hpp"
#include "InterfaceInfo.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace helics {

static void addTags(nlohmann::json& v, const BasicHandleInfo& bhi)
{
    if (bhi.tagCount() > 0) {
        v["tags"] = nlohmann::json::array();
        for (size_t ii = 0; ii < bhi.tagCount(); ++ii) {
            nlohmann::json tagBlock = nlohmann::json::object();
            const auto& tg = bhi.getTagByIndex(ii);
            tagBlock["name"] = tg.first;
            tagBlock["value"] = tg.second;
            v["tags"].push_back(tagBlock);
        }
    }
}

void addFederateTags(nlohmann::json& v, const FederateState* fed)
{
    if (fed->tagCount() > 0) {
        v["tags"] = nlohmann::json::array();

        for (size_t ii = 0; ii < fed->tagCount(); ++ii) {
            nlohmann::json tagBlock = nlohmann::json::array();
            const auto& tg = fed->getTagByIndex(ii);
            tagBlock["name"] = tg.first;
            tagBlock["value"] = tg.second;
            v["tags"].push_back(tagBlock);
        }
    }
}

static void
    storeEndpoint(const BasicHandleInfo& handle, nlohmann::json& block, bool includeID = false)
{
    nlohmann::json ept = nlohmann::json::object();
    ept["name"] = handle.key;
    if (includeID) {
        ept["parent"] = handle.getFederateId().baseValue();
        ept["handle"] = handle.getInterfaceHandle().baseValue();
    }
    ept["type"] = handle.type;
    addTags(ept, handle);
    block["endpoints"].push_back(ept);
}

static void storeEndpoint(const EndpointInfo& handle, nlohmann::json& block)
{
    nlohmann::json ept = nlohmann::json::object();
    ept["name"] = handle.key;
    ept["type"] = handle.type;
    block["endpoints"].push_back(ept);
}

static void storeInput(const BasicHandleInfo& handle, nlohmann::json& block, bool includeID = false)
{
    nlohmann::json ipt = nlohmann::json::object();
    ipt["name"] = handle.key;
    if (includeID) {
        ipt["parent"] = handle.getFederateId().baseValue();
        ipt["handle"] = handle.getInterfaceHandle().baseValue();
    }

    ipt["units"] = handle.units;
    ipt["type"] = handle.type;
    addTags(ipt, handle);
    block["inputs"].push_back(ipt);
}

static void storeInput(const InputInfo& handle, nlohmann::json& block)
{
    nlohmann::json ipt = nlohmann::json::object();
    ipt["name"] = handle.key;
    ipt["units"] = handle.units;
    ipt["type"] = handle.type;
    block["inputs"].push_back(ipt);
}

static void
    storePublication(const BasicHandleInfo& handle, nlohmann::json& block, bool includeID = false)
{
    nlohmann::json pub = nlohmann::json::object();
    pub["name"] = handle.key;
    if (includeID) {
        pub["parent"] = handle.getFederateId().baseValue();
        pub["handle"] = handle.getInterfaceHandle().baseValue();
    }
    pub["units"] = handle.units;
    pub["type"] = handle.type;
    addTags(pub, handle);
    block["publications"].push_back(pub);
}

static void storePublication(const PublicationInfo& handle, nlohmann::json& block)
{
    nlohmann::json pub = nlohmann::json::object();
    pub["name"] = handle.key;
    pub["units"] = handle.units;
    pub["type"] = handle.type;
    block["publications"].push_back(pub);
}

static void
    storeTranslator(const BasicHandleInfo& handle, nlohmann::json& block, bool includeID = false)
{
    nlohmann::json trans = nlohmann::json::object();
    trans["name"] = handle.key;
    if (includeID) {
        trans["parent"] = handle.getFederateId().baseValue();
        trans["handle"] = handle.getInterfaceHandle().baseValue();
    }
    trans["units"] = handle.units;
    trans["type"] = handle.type;
    addTags(trans, handle);
    block["translators"].push_back(trans);
}

static void
    storeFilter(const BasicHandleInfo& handle, nlohmann::json& block, bool includeID = false)
{
    nlohmann::json filt = nlohmann::json::object();
    filt["name"] = handle.key;
    if (includeID) {
        filt["parent"] = handle.getFederateId().baseValue();
        filt["handle"] = handle.getInterfaceHandle().baseValue();
    }
    filt["type_in"] = handle.type_in;
    filt["type_out"] = handle.type_out;
    addTags(filt, handle);
    block["filters"].push_back(filt);
}

nlohmann::json generateInterfaceConfig(const helics::HandleManager& hm,
                                       const helics::GlobalFederateId& fed)
{
    nlohmann::json iblock;
    generateInterfaceConfig(iblock, hm, fed);
    return iblock;
}

void generateInterfaceConfig(nlohmann::json& iblock,
                             const helics::HandleManager& hm,
                             const helics::GlobalFederateId& fed)

{
    bool hasPubs{false};
    bool hasEpts{false};
    bool hasInputs{false};
    bool hasFilt{false};
    bool hasTranslators{false};
    bool allInfo = !fed.isValid();
    for (const auto& handle : hm) {
        if (handle.handle.fed_id == fed || allInfo) {
            switch (handle.handleType) {
                case InterfaceType::ENDPOINT:
                    if (!hasEpts) {
                        iblock["endpoints"] = nlohmann::json::array();
                        hasEpts = true;
                    }
                    storeEndpoint(handle, iblock, allInfo);
                    break;
                case InterfaceType::INPUT:
                    if (!hasInputs) {
                        iblock["inputs"] = nlohmann::json::array();
                        hasInputs = true;
                    }
                    storeInput(handle, iblock, allInfo);
                    break;
                case InterfaceType::TRANSLATOR:
                    if (!hasTranslators) {
                        iblock["translators"] = nlohmann::json::array();
                        hasTranslators = true;
                    }
                    storeTranslator(handle, iblock, allInfo);
                    break;
                case InterfaceType::PUBLICATION:
                    if (!hasPubs) {
                        iblock["publications"] = nlohmann::json::array();
                        hasPubs = true;
                    }
                    storePublication(handle, iblock, allInfo);
                    break;
                case InterfaceType::FILTER:
                    if (!hasFilt) {
                        iblock["filters"] = nlohmann::json::array();
                        hasFilt = true;
                    }
                    storeFilter(handle, iblock, allInfo);
                    break;
                default:
                    break;
            }
        }
    }
}

std::string generateInterfaceQueryResults(std::string_view request,
                                          const HandleManager& handles,
                                          const GlobalFederateId fed,
                                          const std::function<void(nlohmann::json&)>& addHeaderInfo)
{
    if (request == "inputs") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [fed](auto& handle) {
                return ((!fed.isValid() || handle.handle.fed_id == fed) &&
                        (handle.handleType == InterfaceType::INPUT ||
                         handle.handleType == InterfaceType::TRANSLATOR) &&
                        !handle.key.empty());
            });
    }
    if (request == "input_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["inputs"] = nlohmann::json::array();
        for (const auto& handle : handles) {
            if ((!fed.isValid() || handle.handle.fed_id == fed) &&
                (handle.handleType == InterfaceType::INPUT ||
                 handle.handleType == InterfaceType::TRANSLATOR) &&
                !handle.key.empty()) {
                storeInput(handle, base, !fed.isValid());
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "publications") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [fed](auto& handle) {
                return ((!fed.isValid() || handle.handle.fed_id == fed) &&
                        (handle.handleType == InterfaceType::PUBLICATION ||
                         handle.handleType == InterfaceType::TRANSLATOR) &&
                        !handle.key.empty());
            });
    }
    if (request == "publication_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["publications"] = nlohmann::json::array();
        for (const auto& handle : handles) {
            if ((!fed.isValid() || handle.handle.fed_id == fed) &&
                (handle.handleType == InterfaceType::PUBLICATION ||
                 handle.handleType == InterfaceType::TRANSLATOR) &&
                !handle.key.empty()) {
                storePublication(handle, base, !fed.isValid());
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "filters") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [fed](auto& handle) {
                return ((!fed.isValid() || handle.handle.fed_id == fed) &&
                        handle.handleType == InterfaceType::FILTER && !handle.key.empty());
            });
    }
    if (request == "filter_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["filters"] = nlohmann::json::array();
        for (const auto& handle : handles) {
            if ((!fed.isValid() || handle.handle.fed_id == fed) &&
                handle.handleType == InterfaceType::FILTER && !handle.key.empty()) {
                storeFilter(handle, base, !fed.isValid());
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "translators") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [fed](auto& handle) {
                return ((!fed.isValid() || handle.handle.fed_id == fed) &&
                        handle.handleType == InterfaceType::TRANSLATOR && !handle.key.empty());
            });
    }
    if (request == "translator_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["translators"] = nlohmann::json::array();
        for (const auto& handle : handles) {
            if ((!fed.isValid() || handle.handle.fed_id == fed) &&
                handle.handleType == InterfaceType::TRANSLATOR && !handle.key.empty()) {
                storeTranslator(handle, base, !fed.isValid());
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "endpoints") {
        return generateStringVector_if(
            handles,
            [](auto& handle) { return handle.key; },
            [fed](auto& handle) {
                return ((!fed.isValid() || handle.handle.fed_id == fed) &&
                        (handle.handleType == InterfaceType::ENDPOINT ||
                         handle.handleType == InterfaceType::TRANSLATOR) &&
                        !handle.key.empty());
            });
    }
    if (request == "endpoint_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["endpoints"] = nlohmann::json::array();
        for (const auto& handle : handles) {
            if ((!fed.isValid() || handle.handle.fed_id == fed) &&
                (handle.handleType == InterfaceType::ENDPOINT ||
                 handle.handleType == InterfaceType::TRANSLATOR) &&
                !handle.key.empty()) {
                storeEndpoint(handle, base, !fed.isValid());
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "interface_details") {
        nlohmann::json base = generateInterfaceConfig(handles, fed);
        addHeaderInfo(base);
        return fileops::generateJsonString(base);
    }

    return std::string{};
}

std::string generateInterfaceQueryResults(std::string_view request,
                                          const InterfaceInfo& info,
                                          const std::function<void(nlohmann::json&)>& addHeaderInfo)
{
    if (request == "publications") {
        return generateStringVector_if(
            info.getPublications(),
            [](auto& pub) { return pub->key; },
            [](auto& pub) { return !pub->key.empty(); });
    }
    if (request == "inputs") {
        return generateStringVector_if(
            info.getInputs(),
            [](auto& inp) { return inp->key; },
            [](auto& inp) { return !inp->key.empty(); });
    }

    if (request == "endpoints") {
        return generateStringVector_if(
            info.getEndpoints(),
            [](auto& ept) { return ept->key; },
            [](auto& ept) { return !ept->key.empty(); });
    }

    if (request == "input_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["inputs"] = nlohmann::json::array();
        for (const auto& handle : info.getInputs()) {
            if (!handle->key.empty()) {
                storeInput(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }

    if (request == "publication_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["publications"] = nlohmann::json::array();
        for (const auto& handle : info.getPublications()) {
            if (!handle->key.empty()) {
                storePublication(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }

    if (request == "endpoint_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        base["endpoints"] = nlohmann::json::array();
        for (const auto& handle : info.getEndpoints()) {
            if (!handle->key.empty()) {
                storeEndpoint(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "interface_details") {
        nlohmann::json base;
        addHeaderInfo(base);
        info.generateInferfaceConfig(base);
        return fileops::generateJsonString(base);
    }

    return std::string{};
}

}  // namespace helics
