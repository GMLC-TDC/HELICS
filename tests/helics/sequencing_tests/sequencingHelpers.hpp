/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"

#include <future>

namespace helics {
class Federate;
class Broker;

std::future<void> delayMessages(Federate* fed, int preDelayMessageCount, int msecDelay);

std::future<void> delayMessages(Broker* brk, int preDelayMessageCount, int msecDelay);
}  // namespace helics
