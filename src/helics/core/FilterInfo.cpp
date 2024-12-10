/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "FilterInfo.hpp"
// #include "core/core-data.h"
#include "../common/JsonGeneration.hpp"

#include <algorithm>
#include <cstring>
#include <string>

namespace helics {

void FilterInfo::addDestinationEndpoint(GlobalHandle dest,
                                        std::string_view destName,
                                        std::string_view destType)
{
    for (const auto& ti : destEndpoints) {
        if (ti.id == dest) {
            return;
        }
    }
    destEndpoints.emplace_back(dest, destName, destType);
    /** now update the target information*/
    destTargets.reserve(destEndpoints.size());
    destTargets.clear();
    destEpts.clear();
    for (const auto& ti : destEndpoints) {
        destTargets.emplace_back(ti.id);
    }
}

/** add a source to an endpoint*/
void FilterInfo::addSourceEndpoint(GlobalHandle dest,
                                   std::string_view sourceName,
                                   std::string_view sourceType)
{
    for (const auto& si : sourceEndpoints) {
        if (si.id == dest) {
            return;
        }
    }
    sourceEndpoints.emplace_back(dest, sourceName, sourceType);
    /** now update the target information*/
    sourceTargets.reserve(sourceEndpoints.size());
    sourceTargets.clear();
    sourceEpts.clear();
    for (const auto& ti : sourceEndpoints) {
        sourceTargets.emplace_back(ti.id);
    }
}

/** remove a target from connection*/
void FilterInfo::removeTarget(GlobalHandle targetId)
{
    auto si = sourceEndpoints.begin();
    while (si != sourceEndpoints.end()) {
        if (si->id == targetId) {
            sourceEndpoints.erase(si);
            sourceTargets.clear();
            for (const auto& sim : sourceEndpoints) {
                sourceTargets.emplace_back(sim.id);
            }
            sourceEpts.clear();
            break;
        }
    }
    auto di = destEndpoints.begin();
    while (di != destEndpoints.end()) {
        if (di->id == targetId) {
            destEndpoints.erase(si);
            destTargets.clear();
            for (const auto& dim : destEndpoints) {
                destTargets.emplace_back(dim.id);
            }
            destEpts.clear();
            break;
        }
    }
}

const std::string& FilterInfo::getSourceEndpoints() const
{
    if (sourceEpts.empty()) {
        if (!sourceEndpoints.empty()) {
            if (sourceEndpoints.size() == 1) {
                sourceEpts = sourceEndpoints.front().key;
            } else {
                sourceEpts.push_back('[');
                for (const auto& src : sourceEndpoints) {
                    sourceEpts.append(generateJsonQuotedString(src.key));
                    sourceEpts.push_back(',');
                }
                sourceEpts.back() = ']';
            }
        }
    }
    return sourceEpts;
}
/** get a string with the names of the destination endpoints*/
const std::string& FilterInfo::getDestinationEndpoints() const
{
    if (destEpts.empty()) {
        if (!destEndpoints.empty()) {
            if (destEndpoints.size() == 1) {
                destEpts = destEndpoints.front().key;
            } else {
                destEpts.push_back('[');
                for (const auto& trgt : destEndpoints) {
                    destEpts.append(generateJsonQuotedString(trgt.key));
                    destEpts.push_back(',');
                }
                destEpts.back() = ']';
            }
        }
    }
    return destEpts;
}
}  // namespace helics
