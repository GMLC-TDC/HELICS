/*
Copyright (c) 2017-2024,
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
    /**future for the Enter initialization call*/
    std::future<bool> initFuture;
    /** future for the enter execution mode call*/
    std::future<iteration_time> execFuture;
    /** future for the timeRequest call*/
    std::future<Time> timeRequestFuture;
    /** future for the time request iterative call*/
    std::future<iteration_time> timeRequestIterativeFuture;
    /** future for the finalize call*/
    std::future<void> finalizeFuture;
    /** future for the iterative init call*/
    std::future<void> initIterativeFuture;
    /** counter for the number of queries */
    std::atomic<int> queryCounter{0};
    /** the queries that are actually in flight at a given time */
    std::map<int, std::future<std::string>> inFlightQueries;
    /** external function for checking async completion */
    std::function<bool()> asyncCheck;
};
}  // namespace helics
