/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/helicsTime.hpp"

#include <future>
#include <map>
#include <string>

namespace helics {
/** helper class for Federate info that holds the futures for asynchronous calls*/
class AsyncFedCallInfo {
  public:
    std::future<void> initFuture; /**future for the Enter initialization call*/
    std::future<IterationResult> execFuture; /** future for the enter execution mode call*/
    std::future<Time> timeRequestFuture; /** future for the timeRequest call*/
    std::future<iteration_time>
        timeRequestIterativeFuture; /** future for the time request iterative call*/
    std::future<void> finalizeFuture; /** future for the finalize call*/
    std::atomic<int> queryCounter{0};  //!< counter for the number of queries
    std::map<int, std::future<std::string>>
        inFlightQueries;  //!< the queries that are actually in flight at a given time
    std::function<bool()> asyncCheck; //!< external function for checking async completion
};
}  // namespace helics
