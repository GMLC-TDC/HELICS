/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "PublicationInfo.hpp"

#include "helics/external/string_view.hpp"
namespace helics {
bool PublicationInfo::CheckSetValue(const char* dataToCheck, uint64_t len)
{
    if ((len != data.length()) || (stx::string_view(data) != stx::string_view(dataToCheck, len))) {
        data.assign(dataToCheck, len);
        return true;
    }
    return false;
}

void PublicationInfo::removeSubscriber(global_handle subscriberToRemove)
{
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriberToRemove),
                      subscribers.end());
}

}  // namespace helics
