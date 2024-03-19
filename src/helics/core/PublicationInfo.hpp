/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_CoreTypes.hpp"
#include "helicsTime.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace helics {

struct SubscriberInformation {
    GlobalHandle id;
    std::string key;
    SubscriberInformation() = default;
    SubscriberInformation(GlobalHandle gid, std::string_view key_): id(gid), key(key_) {}
};

/** data class containing the information about a publication*/
class PublicationInfo {
  public:
    /** constructor from the basic information*/
    PublicationInfo(GlobalHandle pid,
                    std::string_view pkey,
                    std::string_view ptype,
                    std::string_view punits): id(pid), key(pkey), type(ptype), units(punits)
    {
    }
    const GlobalHandle id;  //!< the identifier for the containing federate
    /// container for all the subscribers of a publication
    std::vector<SubscriberInformation> subscribers;
    const std::string key;  //!< the key identifier for the publication
    const std::string type;  //!< the type of the publication data
    const std::string units;  //!< the units of the publication data
    SmallBuffer data;  //!< the most recent publication data
    Time lastPublishTime{timeZero};  //!< the time of the last publication
    bool has_update{false};  //!< indicator that the publication has updates
    bool only_update_on_change{false};
    bool required{false};  //!< indicator that it is required to be output someplace
    bool buffer_data{false};  //!< indicator that the publication should buffer data
    int32_t requiredConnections{0};  //!< the number of required connections 0 is no requirement
    Time minTimeGap{timeZero};  //!< a time restriction on amount of publishing
    /** check if the value should be published or not*/
    bool CheckSetValue(const char* dataToCheck,
                       uint64_t len,
                       Time currentTime,
                       bool forceChangeCheck);
    /** add a new subscriber to the publication
@return true if the subscriber was added false if duplicate
*/
    bool addSubscriber(GlobalHandle newSubscriber, std::string_view inputName);

    /** remove a subscriber*/
    void removeSubscriber(GlobalHandle subscriberToRemove);
    /** disconnect a federate from the subscriber*/
    void disconnectFederate(GlobalFederateId fedToDisconnect);
    void setProperty(int32_t option, int32_t value);
    int32_t getProperty(int32_t option) const;
    const std::string& getTargets() const;

  private:
    mutable std::string destTargets;
};
}  // namespace helics
