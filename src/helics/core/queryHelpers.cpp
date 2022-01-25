/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "queryHelpers.hpp"

#include "FederateState.hpp"
#include "HandleManager.hpp"

namespace helics {

static void addTags(Json::Value& v, const BasicHandleInfo& bhi)
{
    if (bhi.tagCount() > 0) {
        Json::Value tagBlock = Json::objectValue;
        for (size_t ii = 0; ii < bhi.tagCount(); ++ii) {
            const auto& tg = bhi.getTagByIndex(ii);
            tagBlock[tg.first] = tg.second;
        }
        v["tags"] = tagBlock;
    }
}

void addFederateTags(Json::Value& v, const FederateState* fed)
{
    if (fed->tagCount() > 0) {
        Json::Value tagBlock = Json::objectValue;
        for (size_t ii = 0; ii < fed->tagCount(); ++ii) {
            const auto& tg = fed->getTagByIndex(ii);
            tagBlock[tg.first] = tg.second;
        }
        v["tags"] = tagBlock;
    }
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
    for (const auto& handle : hm) {
        if (handle.handle.fed_id == fed) {
            switch (handle.handleType) {
                case InterfaceType::ENDPOINT: {
                    Json::Value block;
                    block["name"] = handle.key;
                    block["type"] = handle.type;
                    addTags(block, handle);
                    if (!hasEpts) {
                        iblock["endpoints"] = Json::arrayValue;
                        hasEpts = true;
                    }
                    iblock["endpoints"].append(block);
                } break;
                case InterfaceType::INPUT: {
                    Json::Value block;
                    block["name"] = handle.key;
                    block["type"] = handle.type;
                    block["units"] = handle.units;
                    addTags(block, handle);
                    if (!hasInputs) {
                        iblock["inputs"] = Json::arrayValue;
                        hasInputs = true;
                    }
                    iblock["inputs"].append(block);
                } break;
                case InterfaceType::PUBLICATION: {
                    Json::Value block;
                    block["name"] = handle.key;
                    block["type"] = handle.type;
                    block["units"] = handle.units;
                    addTags(block, handle);
                    if (!hasPubs) {
                        iblock["publications"] = Json::arrayValue;
                        hasPubs = true;
                    }
                    iblock["publications"].append(block);
                } break;
                case InterfaceType::FILTER: {
                    Json::Value block;
                    block["name"] = handle.key;
                    block["type_in"] = handle.type_in;
                    block["type_out"] = handle.type_out;
                    addTags(block, handle);
                    if (!hasFilt) {
                        iblock["filters"] = Json::arrayValue;
                        hasFilt = true;
                    }
                    iblock["filters"].append(block);
                } break;
                default:
                    break;
            }
        }
    }
}
}  // namespace helics
