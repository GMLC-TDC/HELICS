/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "helics/core/zmq/zmq-core.h"
#include "helics/core/zmq/zmq-helper.h"
#include "helics/core/ActionMessage.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <zmq.h>

#define USE_LOGGING 1
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream()
#define ENDL std::endl
#endif

static const std::string DEFAULT_BROKER = "tcp://localhost:5555";

static inline std::string gen_id() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
		std::string uuid_str = boost::lexical_cast<std::string>(uuid);
#ifdef _WIN32
    std::string pid_str = boost::lexical_cast<std::string>(GetCurrentProcessId());
#else
    std::string pid_str = boost::lexical_cast<std::string>(getpid());
#endif
    return pid_str+"-"+uuid_str;
}

namespace helics
{



void ZeroMQCore::initialize (const std::string &initializationString)
{
	//do something with initialization the transmit sockets and other ZMQ speficic stuff

	CommonCore::initialize(initializationString);
}


ZeroMQCore::~ZeroMQCore() = default;



void ZeroMQCore::transmit(int route_id, ActionMessage &cmd)
{

}

void ZeroMQCore::addRoute(int route_id, const std::string &routeInfo)
{

}

}  // namespace helics
