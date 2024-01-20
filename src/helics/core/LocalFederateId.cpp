/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "LocalFederateId.hpp"

#include "GlobalFederateId.hpp"

#include <iostream>

namespace helics {
std::ostream& operator<<(std::ostream& os, LocalFederateId fid)
{
    os << fid.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, InterfaceHandle handle)
{
    os << handle.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, GlobalBrokerId id)
{
    os << id.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, GlobalFederateId id)
{
    os << id.baseValue();
    return os;
}

std::ostream& operator<<(std::ostream& os, GlobalHandle id)
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
