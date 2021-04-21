/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "federate_id.hpp"

#include "global_federate_id.hpp"

#include <iostream>

namespace helics {
std::ostream& operator<<(std::ostream& os, local_federate_id fid)
{
    os << fid.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, interface_handle handle)
{
    os << handle.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, global_broker_id id)
{
    os << id.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, global_federate_id id)
{
    os << id.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, global_handle id)
{
    os << id.fed_id.baseValue() << "::" << id.handle.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, route_id id)
{
    os << id.baseValue();
    return os;
}
}  // namespace helics
