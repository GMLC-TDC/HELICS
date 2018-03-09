/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include "helics-time.hpp"
#include "helics/helics-config.h"
#include "../core/Core.hpp"
#include <utility>
#include <vector>

namespace helics
{
/** data class for managing information about a subscription*/
class SubscriptionInfo
{
  public:
	  /** constructor with all the information*/
    SubscriptionInfo (Core::handle_id_t id_,
                      Core::federate_id_t fed_id_,
                      const std::string &key_,
                      const std::string &type_,
                      const std::string &units_,
                      bool required_ = false)
        : id (id_), fed_id (fed_id_), key (key_), type (type_), units (units_), required (required_)
    {
    }

    const Core::handle_id_t id;	//!< identifier for the handle
    const Core::federate_id_t fed_id;	//!< the federate that created the handle
    const std::string key;	//!< the identifier for the subscription
    const std::string type;	//! the type of data for the subscription
	std::string pubType; //!< the type of data that its matching publication uses
    const std::string units;	//!< the units of the subscription
    const bool required;	//!< flag indicating that the subscription requires a matching publication
    bool has_target = false;	//!< flag indicating that a target publication was found
    bool only_update_on_change = false;  //!< flag indicating that the data should only be updated on change
	std::shared_ptr<const data_block> current_data;	//!< the most recent published data
	std::pair<Core::federate_id_t, Core::handle_id_t> target;	//!< the publication information
  private:
	std::vector<std::pair<Time, std::shared_ptr<const data_block>>> data_queue; //!< queue of the data
  public:
	/** get the current data*/
    std::shared_ptr<const data_block> getData ();
	/** add a data block into the queue*/
    void addData (Time valueTime, std::shared_ptr<const data_block> data);

	/** update current status to new time
	@param newTime the time to move the subscription to
	@return true if the value has changed
	*/
    bool updateTime (Time newTime);
    /** get the event based on the event queue*/
    Time nextValueTime () const;
};
}  // namespace helics

