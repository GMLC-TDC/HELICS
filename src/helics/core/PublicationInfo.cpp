/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PublicationInfo.hpp"

#include <algorithm>
#include <string_view>

namespace helics {
bool PublicationInfo::CheckSetValue(const char* dataToCheck, uint64_t len, Time currentTime, bool forceChangeCheck)
{
    if (minTimeGap>timeZero) {
        if (currentTime-lastPublishTime<minTimeGap) {
            return false;
        }
    }
    if (only_update_on_change||forceChangeCheck) {
        if (len != data.length() ||
            std::string_view(data) != std::string_view(dataToCheck, len)) {
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

bool PublicationInfo::addSubscriber(GlobalHandle newSubscriber)
{
    for (const auto& sub : subscribers) {
        if (sub == newSubscriber) {
            return false;
        }
    }
    subscribers.push_back(newSubscriber);
    return true;
}

void PublicationInfo::removeSubscriber(GlobalHandle subscriberToRemove)
{
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriberToRemove),
                      subscribers.end());
}

}  // namespace helics
