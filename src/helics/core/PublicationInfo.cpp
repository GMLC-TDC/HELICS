/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "PublicationInfo.hpp"
#include "helics_includes/string_view.hpp"
namespace helics
{
bool PublicationInfo::CheckSetValue (const char *dataToCheck, uint64_t len)
{
    if ((len != data.length ()) || (stx::string_view (data) != stx::string_view (dataToCheck, len)))
    {
        data.assign(dataToCheck, len);
        return true;
    }
    return false;
}

void PublicationInfo::removeSubscriber(global_handle subscriberToRemove)
{
	subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriberToRemove), subscribers.end());
}

}  // namespace helics
