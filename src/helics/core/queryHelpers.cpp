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

namespace helics {

static void addTags(Json::Value& v, const BasicHandleInfo& bhi)
{
    if (bhi.tagCount() > 0) {
        v["tags"] = Json::arrayValue;
        for (size_t ii = 0; ii < bhi.tagCount(); ++ii) {
            Json::Value tagBlock = Json::objectValue;
            const auto& tg = bhi.getTagByIndex(ii);
            tagBlock["name"] = tg.first;
            tagBlock["value"] = tg.second;
            v["tags"].append(tagBlock);
        }
    }
}

void addFederateTags(Json::Value& v, const FederateState* fed)
{
    if (fed->tagCount() > 0) {
        v["tags"] = Json::arrayValue;

        for (size_t ii = 0; ii < fed->tagCount(); ++ii) {
            Json::Value tagBlock = Json::arrayValue;
            const auto& tg = fed->getTagByIndex(ii);
            tagBlock["name"] = tg.first;
            tagBlock["value"] = tg.second;
            v["tags"].append(tagBlock);
        }
    }
}

static void storeEndpoint(const BasicHandleInfo& handle, Json::Value& block, bool includeID = false)
{
    Json::Value ept = Json::objectValue;
    ept["name"] = handle.key;
    if (includeID) {
        ept["parent"] = handle.getFederateId().baseValue();
        ept["handle"] = handle.getInterfaceHandle().baseValue();
    }
    ept["type"] = handle.type;
    addTags(ept, handle);
    block["endpoints"].append(ept);
}

static void storeEndpoint(const EndpointInfo& handle, Json::Value& block)
{
    Json::Value ept = Json::objectValue;
    ept["name"] = handle.key;
    ept["type"] = handle.type;
    block["endpoints"].append(ept);
}

static void storeInput(const BasicHandleInfo& handle, Json::Value& block, bool includeID = false)
{
    Json::Value ipt = Json::objectValue;
    ipt["name"] = handle.key;
    if (includeID) {
        ipt["parent"] = handle.getFederateId().baseValue();
        ipt["handle"] = handle.getInterfaceHandle().baseValue();
    }

    ipt["units"] = handle.units;
    ipt["type"] = handle.type;
    addTags(ipt, handle);
    block["inputs"].append(ipt);
}

static void storeInput(const InputInfo& handle, Json::Value& block)
{
    Json::Value ipt = Json::objectValue;
    ipt["name"] = handle.key;
    ipt["units"] = handle.units;
    ipt["type"] = handle.type;
    block["inputs"].append(ipt);
}

static void
    storePublication(const BasicHandleInfo& handle, Json::Value& block, bool includeID = false)
{
    Json::Value pub = Json::objectValue;
    pub["name"] = handle.key;
    if (includeID) {
        pub["parent"] = handle.getFederateId().baseValue();
        pub["handle"] = handle.getInterfaceHandle().baseValue();
    }
    pub["units"] = handle.units;
    pub["type"] = handle.type;
    addTags(pub, handle);
    block["publications"].append(pub);
}

static void storePublication(const PublicationInfo& handle, Json::Value& block)
{
    Json::Value pub = Json::objectValue;
    pub["name"] = handle.key;
    pub["units"] = handle.units;
    pub["type"] = handle.type;
    block["publications"].append(pub);
}

static void
    storeTranslator(const BasicHandleInfo& handle, Json::Value& block, bool includeID = false)
{
    Json::Value trans = Json::objectValue;
    trans["name"] = handle.key;
    if (includeID) {
        trans["parent"] = handle.getFederateId().baseValue();
        trans["handle"] = handle.getInterfaceHandle().baseValue();
    }
    trans["units"] = handle.units;
    trans["type"] = handle.type;
    addTags(trans, handle);
    block["translators"].append(trans);
}

static void storeFilter(const BasicHandleInfo& handle, Json::Value& block, bool includeID = false)
{
    Json::Value filt = Json::objectValue;
    filt["name"] = handle.key;
    if (includeID) {
        filt["parent"] = handle.getFederateId().baseValue();
        filt["handle"] = handle.getInterfaceHandle().baseValue();
    }
    filt["type_in"] = handle.type_in;
    filt["type_out"] = handle.type_out;
    addTags(filt, handle);
    block["filters"].append(filt);
}

Json::Value generateInterfaceConfig(const helics::HandleManager& hm,
                                    const helics::GlobalFederateId& fed)
{
    Json::Value iblock;
    generateInterfaceConfig(iblock, hm, fed);
    return iblock;
}

void generateInterfaceConfig(Json::Value& iblock,
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
                        iblock["endpoints"] = Json::arrayValue;
                        hasEpts = true;
                    }
                    storeEndpoint(handle, iblock, allInfo);
                    break;
                case InterfaceType::INPUT:
                    if (!hasInputs) {
                        iblock["inputs"] = Json::arrayValue;
                        hasInputs = true;
                    }
                    storeInput(handle, iblock, allInfo);
                    break;
                case InterfaceType::TRANSLATOR:
                    if (!hasTranslators) {
                        iblock["translators"] = Json::arrayValue;
                        hasTranslators = true;
                    }
                    storeTranslator(handle, iblock, allInfo);
                    break;
                case InterfaceType::PUBLICATION:
                    if (!hasPubs) {
                        iblock["publications"] = Json::arrayValue;
                        hasPubs = true;
                    }
                    storePublication(handle, iblock, allInfo);
                    break;
                case InterfaceType::FILTER:
                    if (!hasFilt) {
                        iblock["filters"] = Json::arrayValue;
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
                                          const std::function<void(Json::Value&)>& addHeaderInfo)
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
        Json::Value base;
        addHeaderInfo(base);
        base["inputs"] = Json::arrayValue;
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
        Json::Value base;
        addHeaderInfo(base);
        base["publications"] = Json::arrayValue;
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
        Json::Value base;
        addHeaderInfo(base);
        base["filters"] = Json::arrayValue;
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
        Json::Value base;
        addHeaderInfo(base);
        base["translators"] = Json::arrayValue;
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
        Json::Value base;
        addHeaderInfo(base);
        base["endpoints"] = Json::arrayValue;
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
        Json::Value base = generateInterfaceConfig(handles, fed);
        addHeaderInfo(base);
        return fileops::generateJsonString(base);
    }

    return std::string{};
}

std::string generateInterfaceQueryResults(std::string_view request,
                                          const InterfaceInfo& info,
                                          const std::function<void(Json::Value&)>& addHeaderInfo)
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
        Json::Value base;
        addHeaderInfo(base);
        base["inputs"] = Json::arrayValue;
        for (const auto& handle : info.getInputs()) {
            if (!handle->key.empty()) {
                storeInput(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }

    if (request == "publication_details") {
        Json::Value base;
        addHeaderInfo(base);
        base["publications"] = Json::arrayValue;
        for (const auto& handle : info.getPublications()) {
            if (!handle->key.empty()) {
                storePublication(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }

    if (request == "endpoint_details") {
        Json::Value base;
        addHeaderInfo(base);
        base["endpoints"] = Json::arrayValue;
        for (const auto& handle : info.getEndpoints()) {
            if (!handle->key.empty()) {
                storeEndpoint(*handle, base);
            }
        }
        return fileops::generateJsonString(base);
    }
    if (request == "interface_details") {
        Json::Value base;
        addHeaderInfo(base);
        info.generateInferfaceConfig(base);
        return fileops::generateJsonString(base);
    }

    return std::string{};
}

}  // namespace helics
