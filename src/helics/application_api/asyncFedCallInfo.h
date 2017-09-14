/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ASYNC_FED_CALL_INFO_H_
#define ASYNC_FED_CALL_INFO_H_
#pragma once
#include "core/helics-time.h"
#include <future>
#include <map>
#include <string>

/** helper class for Federate info that holds the futures for async calls*/
class asyncFedCallInfo
{
public:
	std::future<void> initFuture; /**future for the init call*/
	std::future<helics::convergence_state> execFuture; /** future for the enter execution mode call*/
	std::future<helics::Time> timeRequestFuture; /** future for the timeRequest call*/
	std::future<helics::iterationTime> timeRequestIterativeFuture; /** future for the time request iterative call*/
	std::atomic<int> queryCounter{ 0 }; //!< counter for the number of queries
	std::map<int, std::future<std::string>> inFlightQueries; //!< the queries that are actually in flight at a given time
};

#endif
