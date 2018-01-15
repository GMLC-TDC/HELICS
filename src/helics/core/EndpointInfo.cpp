/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "EndpointInfo.h"
//#include "core/core-data.h"

#include <algorithm>
#include <cstring>

namespace helics
{
std::unique_ptr<Message> EndpointInfo::getMessage (Time maxTime)
{
    auto handle = message_queue.lock ();
    if (handle->empty ())
    {
        return nullptr;
    }
    if (handle->front ()->time <= maxTime)
    {
        auto msg = std::move (handle->front ());
        handle->pop_front ();
        return msg;
    }
    return nullptr;
}

Time EndpointInfo::firstMessageTime () const
{
    auto handle = message_queue.lock_shared ();
    return (handle->empty ()) ? Time::maxVal () : handle->front ()->time;
}
// this is the function which determines message order
static auto msgSorter = [](const auto &m1, const auto &m2) {
    // first by time
    return (m1->time != m2->time) ? (m1->time < m2->time) : (m1->original_source < m2->original_source);
};

void EndpointInfo::addMessage (std::unique_ptr<Message> message)
{
    auto handle = message_queue.lock ();
    handle->push_back (std::move (message));
    std::stable_sort (handle->begin (), handle->end (), msgSorter);
}

int32_t EndpointInfo::queueSize (Time maxTime) const
{
    auto handle = message_queue.lock_shared ();
    int32_t cnt = 0;
    for (auto &msg : *handle)
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