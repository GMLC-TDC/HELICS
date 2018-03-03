/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once
#include "../core/helics-time.hpp"
#include <future>
#include <map>
#include <string>

namespace helics
{
/** helper class for Federate info that holds the futures for asynchronous calls*/
class AsyncFedCallInfo
{
  public:
    std::future<void> initFuture; /**future for the Enter initialization call*/
    std::future<helics::iteration_result> execFuture; /** future for the enter execution mode call*/
    std::future<helics::Time> timeRequestFuture; /** future for the timeRequest call*/
    std::future<helics::iteration_time>
      timeRequestIterativeFuture; /** future for the time request iterative call*/
    std::atomic<int> queryCounter{0};  //!< counter for the number of queries
    std::map<int, std::future<std::string>>
      inFlightQueries;  //!< the queries that are actually in flight at a given time
};
}  // namespace helics
