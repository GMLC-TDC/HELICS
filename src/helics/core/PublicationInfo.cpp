/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PublicationInfo.hpp"

#include "../common/JsonGeneration.hpp"
#include "helics_definitions.hpp"

#include <algorithm>
#include <string>
#include <string_view>

namespace helics {
bool PublicationInfo::CheckSetValue(const char* dataToCheck,
                                    uint64_t len,
                                    Time currentTime,
                                    bool forceChangeCheck)
{
    if (minTimeGap > timeZero) {
        if (currentTime - lastPublishTime < minTimeGap) {
            return false;
        }
    }
    if (only_update_on_change || forceChangeCheck) {
        if (std::string_view(dataToCheck, len) != data.to_string()) {
            data.assign(dataToCheck, len);
        } else {
            return false;
        }
    } else if (buffer_data) {
        data.assign(dataToCheck, len);
    }
    lastPublishTime = currentTime;
    return true;
}

bool PublicationInfo::addSubscriber(GlobalHandle newSubscriber, std::string_view subscriberName)
{
    for (auto& sub : subscribers) {
        if (sub.id == newSubscriber) {
            return false;
        }
    }
    subscribers.emplace_back(newSubscriber, subscriberName);
    return true;
}

void PublicationInfo::disconnectFederate(GlobalFederateId fedToDisconnect)
{
    subscribers.erase(std::remove_if(subscribers.begin(),
                                     subscribers.end(),
                                     [fedToDisconnect](const auto& val) {
                                         return val.id.fed_id == fedToDisconnect;
                                     }),
                      subscribers.end());
}

void PublicationInfo::removeSubscriber(GlobalHandle subscriberToRemove)
{
    subscribers.erase(std::remove_if(subscribers.begin(),
                                     subscribers.end(),
                                     [subscriberToRemove](const auto& val) {
                                         return val.id == subscriberToRemove;
                                     }),
                      subscribers.end());
}

void PublicationInfo::setProperty(int32_t option, int32_t value)
{
    bool bvalue = (value != 0);
    switch (option) {
        case defs::Options::HANDLE_ONLY_TRANSMIT_ON_CHANGE:
            only_update_on_change = bvalue;
            break;
        case defs::Options::CONNECTION_REQUIRED:
            required = bvalue;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            required = !bvalue;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            requiredConnections = bvalue ? 1 : 0;
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            requiredConnections = !bvalue ? 0 : 1;
            break;
        case defs::Options::BUFFER_DATA:
            buffer_data = bvalue;
            break;
        case defs::Options::CONNECTIONS:
            requiredConnections = value;
            break;
        case defs::Options::TIME_RESTRICTED:
            minTimeGap = Time(value, time_units::ms);
            break;
        default:
            break;
    }
}

int32_t PublicationInfo::getProperty(int32_t option) const
{
    bool flagval = false;
    switch (option) {
        case defs::Options::HANDLE_ONLY_TRANSMIT_ON_CHANGE:
            flagval = only_update_on_change;
            break;
        case defs::Options::CONNECTION_REQUIRED:
            flagval = required;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            flagval = !required;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            flagval = (requiredConnections == 1);
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            flagval = requiredConnections != 1;
            break;
        case defs::Options::BUFFER_DATA:
            flagval = buffer_data;
            break;
        case defs::Options::CONNECTIONS:
            return static_cast<int32_t>(subscribers.size());
        case defs::Options::TIME_RESTRICTED:
            return static_cast<std::int32_t>(minTimeGap.to_ms().count());
        default:
            break;
    }
    return flagval ? 1 : 0;
}

const std::string& PublicationInfo::getTargets() const
{
    if (destTargets.empty()) {
        if (!subscribers.empty()) {
            if (subscribers.size() == 1) {
                destTargets = subscribers.front().key;
            } else {
                destTargets.push_back('[');
                for (const auto& sub : subscribers) {
                    destTargets.append(generateJsonQuotedString(sub.key));
                    destTargets.push_back(',');
                }
                destTargets.back() = ']';
            }
        }
    }
    return destTargets;
}
}  // namespace helics
