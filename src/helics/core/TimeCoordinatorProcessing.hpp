/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "TimeCoordinator.hpp"

#include <tuple>

namespace helics {
std::tuple<federate_state, message_processing_result, bool>
    processCoordinatorMessage(ActionMessage& cmd,
                              TimeCoordinator* timeCoord,
                              const federate_state state,
                              const bool timeGranted_mode,
                              const global_federate_id localID);

}
