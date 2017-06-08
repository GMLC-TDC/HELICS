/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "SubscriptionInfo.h"
//#include "core/core-data.h"
#include <algorithm>
namespace helics
{
data_t *SubscriptionInfo::getData()
{
	auto ndata = new data_t;
	if (!current_data.empty())
	{
		ndata->data = new char[current_data.size()];
		memcpy(ndata->data, current_data.data(), current_data.size());
		ndata->len = current_data.size();
	}
	else
	{
		ndata->data = nullptr;
		ndata->len = 0;
	}
	return ndata;
}


void SubscriptionInfo::updateData(Time updateTime, const std::string &data)
{
	auto m = std::upper_bound(data_queue.begin(), data_queue.end(), updateTime, [](auto &time, auto &tm) {return (time < tm.first); });
	data_queue.emplace(m, updateTime, data);
}

}

