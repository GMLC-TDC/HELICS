/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_SUBSCRIPTIONINFO_
#define _HELICS_SUBSCRIPTIONINFO_

#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/config.h"
#include "helics/core/core.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <string>

namespace helics
{
class SubscriptionInfo
{
  public:
    SubscriptionInfo (Core::Handle id_,
                      Core::federate_id_t fed_id_,
                      const std::string &key_,
                      const std::string &type_,
                      const std::string &units_,
                      bool required_ = false)
        : id (id_), fed_id (fed_id_), key (key_), type (type_), units (units_), required (required_)
    {
    }

    ~SubscriptionInfo () {}

    Core::Handle id;
    Core::federate_id_t fed_id;
    std::string key;
    std::string type;
    std::string units;
    bool required;
    bool has_target = false;
	std::string current_data;
	std::pair<Core::federate_id_t, Core::Handle> target;
private:
	std::vector<std::pair<Time, std::string>> data_queue;
public:
	data_t *getData();
	void updateData(Time updateTime, const std::string &data);
};
}  // namespace helics

#endif /* _HELICS_TEST_CORE_ */
