/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "EndpointInfo.hpp"

#include "../common/JsonGeneration.hpp"
//#include "core/core-data.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <utility>

namespace helics {
std::unique_ptr<Message> EndpointInfo::getMessage(Time maxTime)
{
    auto handle = message_queue.lock();
    if (handle->empty()) {
        return nullptr;
    }
    if (handle->front()->time <= maxTime) {
        auto msg = std::move(handle->front());
        handle->pop_front();
        return msg;
    }
    return nullptr;
}

Time EndpointInfo::firstMessageTime() const
{
    auto handle = message_queue.lock_shared();
    return (handle->empty()) ? Time::maxVal() : handle->front()->time;
}
// this is the function which determines message order
static auto msgSorter = [](const auto& m1, const auto& m2) {
    // first by time
    return (m1->time != m2->time) ? (m1->time < m2->time) :
                                    (m1->original_source < m2->original_source);
};

void EndpointInfo::addMessage(std::unique_ptr<Message> message)
{
    auto handle = message_queue.lock();
    handle->push_back(std::move(message));
    std::stable_sort(handle->begin(), handle->end(), msgSorter);
}

void EndpointInfo::clearQueue()
{
    message_queue.lock()->clear();
}

int32_t EndpointInfo::queueSize(Time maxTime) const
{
    auto handle = message_queue.lock_shared();
    int32_t cnt = 0;
    for (auto& msg : *handle) {
        if (msg->time <= maxTime) {
            ++cnt;
        } else {
            break;
        }
    }
    return cnt;
}

void EndpointInfo::addDestinationTarget(GlobalHandle dest,
                                        const std::string& destName,
                                        const std::string& destType)
{
    for (const auto& ti : targetInformation) {
        if (ti.id == dest) {
            return;
        }
    }
    targetInformation.emplace_back(dest, destName, destType);
    /** now update the target information*/
    targets.reserve(targetInformation.size());
    targets.clear();
    for (const auto& ti : targetInformation) {
        targets.emplace_back(ti.id, ti.key);
    }
}

/** add a source to an endpoint*/
void EndpointInfo::addSourceTarget(GlobalHandle dest,
                                   const std::string& sourceName,
                                   const std::string& sourceType)
{
    for (const auto& si : sourceInformation) {
        if (si.id == dest) {
            return;
        }
    }
    sourceInformation.emplace_back(dest, sourceName, sourceType);
}

/** remove a target from connection*/
void EndpointInfo::removeTarget(GlobalHandle targetId)
{
    auto ti = targetInformation.begin();
    while (ti != targetInformation.end()) {
        if (ti->id == targetId) {
            targetInformation.erase(ti);
            targets.clear();
            for (const auto& targetInfo : targetInformation) {
                targets.emplace_back(targetInfo.id, targetInfo.key);
            }
            break;
        }
    }
    auto si = sourceInformation.begin();
    while (si != sourceInformation.end()) {
        if (si->id == targetId) {
            sourceInformation.erase(si);
            return;
        }
    }
}

const std::string& EndpointInfo::getSourceTargets() const
{
    if (sourceTargets.empty()) {
        if (!sourceInformation.empty()) {
            if (sourceInformation.size() == 1) {
                sourceTargets = sourceInformation.front().key;
            } else {
                sourceTargets.push_back('[');
                for (const auto& src : sourceInformation) {
                    sourceTargets.append(generateJsonQuotedString(src.key));
                    sourceTargets.push_back(',');
                }
                sourceTargets.back() = ']';
            }
        }
    }
    return sourceTargets;
}
/** get a string with the names of the destination endpoints*/
const std::string& EndpointInfo::getDestinationTargets() const
{
    if (destinationTargets.empty()) {
        if (!targetInformation.empty()) {
            if (targetInformation.size() == 1) {
                destinationTargets = targetInformation.front().key;
            } else {
                destinationTargets.push_back('[');
                for (const auto& trgt : targetInformation) {
                    destinationTargets.append(generateJsonQuotedString(trgt.key));
                    destinationTargets.push_back(',');
                }
                destinationTargets.back() = ']';
            }
        }
    }
    return destinationTargets;
}

}  // namespace helics
