/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../core/Core.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"
#include <utility>
#include <vector>

namespace helics
{
/** data class for managing information about a subscription*/
class SubscriptionInfo
{
  public:
    struct dataRecord
    {
        Time time;
        unsigned int index = 0;
        std::shared_ptr<const data_block> data;
        dataRecord () = default;
        dataRecord (Time recordTime, std::shared_ptr<const data_block> recordData)
            : time (recordTime), data (std::move (recordData))
        {
        }
        dataRecord (Time recordTime, int recordIndex, std::shared_ptr<const data_block> recordData)
            : time (recordTime), index (recordIndex), data (std::move (recordData))
        {
        }
    };

    /** constructor with all the information*/
    SubscriptionInfo (Core::handle_id_t id_,
                      global_federate_id_t fed_id_,
                      const std::string &key_,
                      const std::string &type_,
                      const std::string &units_,
                      bool required_ = false)
        : id (id_), fed_id (fed_id_), key (key_), type (type_), units (units_), required (required_)
    {
    }

    const Core::handle_id_t id;  //!< identifier for the handle
    const global_federate_id_t fed_id;  //!< the federate that created the handle
    const std::string key;  //!< the identifier for the subscription
    const std::string type;  //! the type of data for the subscription
    std::string pubType;  //!< the type of data that its matching publication uses
    const std::string units;  //!< the units of the subscription
    const bool required;  //!< flag indicating that the subscription requires a matching publication
    bool has_target = false;  //!< flag indicating that a target publication was found
    bool only_update_on_change = false;  //!< flag indicating that the data should only be updated on change
    dataRecord current_data;  //!< the most recent published data
    std::pair<global_federate_id_t, Core::handle_id_t> target;  //!< the publication information
  private:
    std::vector<dataRecord> data_queue;  //!< queue of the data

  public:
    /** get the current data*/
    std::shared_ptr<const data_block> getData ();
    /** add a data block into the queue*/
    void addData (Time valueTime, unsigned int index, std::shared_ptr<const data_block> data);

    /** update current data not including data at the specified time
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeUpTo (Time newTime);
    /** update current data to all new data at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeInclusive (Time newTime);

    /** update current data to get all data through the first iteration at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeNextIteration (Time newTime);
    /** get the event based on the event queue*/
    Time nextValueTime () const;

  private:
    bool updateData (dataRecord &&update);
};
}  // namespace helics
