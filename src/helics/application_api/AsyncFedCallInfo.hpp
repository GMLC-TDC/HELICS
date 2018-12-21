/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
    std::future<iteration_result> execFuture; /** future for the enter execution mode call*/
    std::future<Time> timeRequestFuture; /** future for the timeRequest call*/
    std::future<iteration_time> timeRequestIterativeFuture; /** future for the time request iterative call*/
    std::future<void> finalizeFuture; /** future for the finalize call*/
    std::atomic<int> queryCounter{0};  //!< counter for the number of queries
    std::map<int, std::future<std::string>>
      inFlightQueries;  //!< the queries that are actually in flight at a given time
};
}  // namespace helics
