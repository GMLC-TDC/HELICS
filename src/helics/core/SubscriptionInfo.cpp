/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "SubscriptionInfo.h"

#include <algorithm>

namespace helics
{
std::shared_ptr<const data_block> SubscriptionInfo::getData()
{
	return current_data;
}


void SubscriptionInfo::updateData(Time updateTime, std::shared_ptr<const data_block> data)
{
	auto m = std::upper_bound(data_queue.begin(), data_queue.end(), updateTime, [](auto &time, auto &tm) {return (time < tm.first); });
	data_queue.emplace(m, updateTime, std::move(data));
}

}

