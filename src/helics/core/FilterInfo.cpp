/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "FilterInfo.h"
//#include "core/core-data.h"
#include <algorithm>
#include <cstring>

namespace helics
{
message_t *FilterInfo::getMessage(Time maxTime)
{
	//TODO make thread safe
	if (message_queue.empty())
	{
		return nullptr;
	}
	if (message_queue.front()->time <= maxTime)
	{
		auto msg = message_queue.front();
		message_queue.pop_front();
		return msg;
	}
	else
	{
		return nullptr;
	}
}


Time FilterInfo::firstMessageTime()
{
	return (message_queue.empty()) ? Time::maxVal() : message_queue.front()->time;
}

static auto msgSorter = [](const message_t *m1, const message_t *m2)
{
	//first by time
	if (m1->time != m2->time)
	{
		return (m1->time < m2->time);
	}
	return (strcmp(m1->origsrc, m2->origsrc) < 0);
};

void FilterInfo::addMessage(message_t *m)
{
	message_queue.push_back(m);
	std::sort(message_queue.begin(), message_queue.end(), msgSorter);
}

int32_t FilterInfo::queueSize(Time maxTime)
{
	//TODO make thread safe
	int32_t cnt = 0;
	for (auto msg : message_queue)
	{
		if (msg->time <= maxTime)
			++cnt;
		else
			break;
	}
	return cnt;
}
}