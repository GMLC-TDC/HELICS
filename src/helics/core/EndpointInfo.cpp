/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "EndpointInfo.hpp"

#include "../common/JsonGeneration.hpp"
#include "helics_definitions.hpp"
// #include "core/core-data.hpp"
#include "../common/fmt_format.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <set>
#include <utility>

namespace helics {

bool EndpointInfo::updateTimeUpTo(Time newTime)
{
    int index{0};
    auto handle = message_queue.lock();

    auto cv = handle.begin();
    auto it_final = handle.end();
    while (cv != it_final) {
        if ((*cv)->time >= newTime) {
            break;
        }
        ++index;
        ++cv;
    }
    if (index != mAvailableMessages.load()) {
        mAvailableMessages.store(index);
        return true;
    }
    return false;
}

bool EndpointInfo::updateTimeNextIteration(Time newTime)
{
    int index{0};
    auto handle = message_queue.lock();

    auto cv = handle.begin();
    auto it_final = handle.end();
    while (cv != it_final) {
        if ((*cv)->time > newTime) {
            break;
        }
        ++index;
        ++cv;
    }
    if (index != mAvailableMessages.load()) {
        mAvailableMessages.store(index);
        return true;
    }
    return false;
}

bool EndpointInfo::updateTimeInclusive(Time newTime)
{
    int index{0};
    auto handle = message_queue.lock();

    auto cv = handle.begin();
    auto it_final = handle.end();
    while (cv != it_final) {
        if ((*cv)->time > newTime) {
            break;
        }
        ++index;
        ++cv;
    }
    if (index != mAvailableMessages.load()) {
        mAvailableMessages.store(index);
        return true;
    }
    return false;
}

std::unique_ptr<Message> EndpointInfo::getMessage(Time maxTime)
{
    if (mAvailableMessages.load() > 0) {
        auto handle = message_queue.lock();
        if (handle->empty()) {
            return nullptr;
        }
        if (handle->front()->time <= maxTime) {
            if (mAvailableMessages > 0) {
                --mAvailableMessages;
            }
            auto msg = std::move(handle->front());
            handle->pop_front();
            return msg;
        }
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
    mAvailableMessages.store(0);
    message_queue.lock()->clear();
}

int32_t EndpointInfo::availableMessages() const
{
    return mAvailableMessages;
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
/** get the number of messages available prior to a specific time*/
int32_t EndpointInfo::queueSizeUpTo(Time maxTime) const
{
    auto handle = message_queue.lock_shared();
    int32_t cnt = 0;
    for (auto& msg : *handle) {
        if (msg->time < maxTime) {
            ++cnt;
        } else {
            break;
        }
    }
    return cnt;
}

void EndpointInfo::addDestination(GlobalHandle dest,
                                  std::string_view destName,
                                  std::string_view destType)
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
void EndpointInfo::addSource(GlobalHandle source,
                             std::string_view sourceName,
                             std::string_view sourceType)
{
    for (const auto& si : sourceInformation) {
        if (si.id == source) {
            return;
        }
    }
    sourceInformation.emplace_back(source, sourceName, sourceType);
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

void EndpointInfo::checkInterfacesForIssues(std::vector<std::pair<int, std::string>>& issues)
{
    if (!targetedEndpoint) {
        return;
    }
    if (required) {
        if (sourceInformation.empty() && targetInformation.empty()) {
            issues.emplace_back(helics::defs::Errors::CONNECTION_FAILURE,
                                fmt::format("Endpoint {} is required but has no connections", key));
        }
    }
    if (requiredConnections > 0) {
        auto max_connections = (std::max)(targetInformation.size(), sourceInformation.size());
        auto sum_connections = targetInformation.size() + sourceInformation.size();

        if (max_connections > static_cast<size_t>(requiredConnections)) {
            if (requiredConnections == 1) {
                issues.emplace_back(
                    helics::defs::Errors::CONNECTION_FAILURE,
                    fmt::format(
                        "Endpoint {} is single source only but has more than one connection", key));
            } else {
                issues.emplace_back(
                    helics::defs::Errors::CONNECTION_FAILURE,
                    fmt::format("Endpoint {} requires {} connections but has at least {}",
                                key,
                                requiredConnections,
                                max_connections));
            }
        } else {
            if (static_cast<std::int32_t>(sum_connections) != requiredConnections) {
                std::set<GlobalHandle> handles;
                for (const auto& src : sourceInformation) {
                    handles.emplace(src.id);
                }
                for (const auto& trg : targetInformation) {
                    handles.emplace(trg.id);
                }
                if (static_cast<std::int32_t>(handles.size()) != requiredConnections) {
                    issues.emplace_back(
                        helics::defs::Errors::CONNECTION_FAILURE,
                        fmt::format("Endpoint {} requires {} connections but has only {}",
                                    key,
                                    requiredConnections,
                                    handles.size()));
                }
            }
        }
    }
}

void EndpointInfo::setProperty(int32_t option, int32_t value)
{
    bool bvalue = (value != 0);
    switch (option) {
        case defs::Options::CONNECTION_REQUIRED:
            required = bvalue;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            required = !bvalue;
            break;
        case defs::Options::SEND_ONLY:
            sourceOnly = bvalue;
            break;
        case defs::Options::RECEIVE_ONLY:
            receiveOnly = bvalue;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            requiredConnections = bvalue ? 1 : 0;
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            requiredConnections = !bvalue ? 0 : 1;
            break;
        case defs::Options::CONNECTIONS:
            requiredConnections = value;
            break;
        default:
            break;
    }
}

int32_t EndpointInfo::getProperty(int32_t option) const
{
    bool flagval = false;
    switch (option) {
        case defs::Options::CONNECTION_REQUIRED:
            flagval = required;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            flagval = !required;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            flagval = (requiredConnections == 1);
            break;
        case defs::Options::SEND_ONLY:
            flagval = sourceOnly;
            break;
        case defs::Options::RECEIVE_ONLY:
            flagval = receiveOnly;
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            flagval = (requiredConnections != 1);
            break;
        case defs::Options::CONNECTIONS:
            return static_cast<int32_t>(targetInformation.size());
        default:
            break;
    }
    return flagval ? 1 : 0;
}
}  // namespace helics
