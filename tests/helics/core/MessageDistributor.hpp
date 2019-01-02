/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../src/helics/core/ActionMessage.hpp"

#include <map>
#include <functional>
namespace helics
{
namespace test
{
	class MessageGenerator;

class MessageDistributor
{
  private:
    std::map<global_federate_id, std::function<void(ActionMessage &&)>> messageDistribution;

  public:
    MessageDistributor () = default;
    void addConsumer (global_federate_id id, std::function<void(ActionMessage &&)> cb)
	{
		messageDistribution[id] = std::move(cb);
	}

    void run (MessageGenerator &gen, int cnt = -1);
};
}  // namespace test
}  // namespace helics
