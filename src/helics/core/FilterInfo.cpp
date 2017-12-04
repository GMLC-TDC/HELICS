/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "FilterInfo.h"
//#include "core/core-data.h"
#include <algorithm>
#include <cstring>

namespace helics
{
std::unique_ptr<Message> FilterInfo::getMessage (Time maxTime)
{
    std::lock_guard<std::mutex> lock (queueLock);
    if (message_queue.empty ())
    {
        return nullptr;
    }
    if (message_queue.front ()->time <= maxTime)
    {
        auto msg = std::move (message_queue.front ());
        message_queue.pop_front ();
        return msg;
    }
    return nullptr;
}

Time FilterInfo::firstMessageTime () const
{
    std::lock_guard<std::mutex> lock (queueLock);
    return (message_queue.empty ()) ? Time::maxVal () : message_queue.front ()->time;
}

static auto msgSorter = [](const auto &m1, const auto &m2) {
    // first by time
    return (m1->time != m2->time) ? (m1->time < m2->time) : (m1->origsrc < m2->origsrc);
};

void FilterInfo::addMessage (std::unique_ptr<Message> message)
{
    std::lock_guard<std::mutex> lock (queueLock);
    message_queue.push_back (std::move (message));
    std::sort (message_queue.begin (), message_queue.end (), msgSorter);
}

int32_t FilterInfo::queueSize (Time maxTime) const
{
    std::lock_guard<std::mutex> lock (queueLock);
    int32_t cnt = 0;
    for (auto &msg : message_queue)
    {
        if (msg->time <= maxTime)
        {
            ++cnt;
        }
        else
        {
            break;
        }
    }
    return cnt;
}
}  // namespace helics