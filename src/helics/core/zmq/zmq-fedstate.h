/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_FEDSTATE_
#define _HELICS_ZEROMQ_FEDSTATE_

#include <map>
#include <string>
#include <vector>

#include "helics/config.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"

namespace helics
{

typedef Core::federate_id_t federate_id_t;
typedef Core::Handle Handle;

class ZeroMQHandle;

class FederateState
{
  public:
    FederateState (const char *name_, const Core::FederateInfo &info_)
        : name (name_), info (info_), state (HELICS_CREATED)

    {
    }

    std::string name;
    const Core::FederateInfo &info;
    unsigned int id = (unsigned int)(-1); //!< id code, default to something invalid
    helics_federate_state_type state = HELICS_NONE;
    std::map<std::string, ZeroMQHandle *> subs;
    std::map<std::string, ZeroMQHandle *> pubs;
    std::map<std::string, ZeroMQHandle *> ends;
	std::map<std::string, ZeroMQHandle *> filters;
    BlockingQueue<std::string> queue;
    bool iterative = false;
    bool converged_requested = true;
	bool processing = false;
	bool messages_pending = false;
    Time time_requested = timeZero;
    Time time_delta=1.0;
    Time time_last_processed=timeZero;
	Time time_look_ahead = timeZero;
	Time time_impact = timeZero;
	std::uint64_t max_iterations = 1;
    std::vector<Core::Handle> events;
	std::map<Core::Handle, std::deque<message_t *>> message_queue;
	std::mutex _mutex;

    /** DISABLE_COPY_AND_ASSIGN */
  private:
    FederateState (const FederateState &) = delete;
    FederateState &operator= (const FederateState &) = delete;
};

} // namespace helics

#endif /* _HELICS_ZEROMQ_FEDSTATE_ */
