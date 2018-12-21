/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "federate_id.hpp"

#include <iostream>

namespace helics
{
std::ostream &operator<< (std::ostream &os, federate_id_t fid)
{
    os << fid.baseValue ();
    return os;
}

std::ostream &operator<< (std::ostream &os, interface_handle handle)
{
    os << handle.baseValue ();
    return os;
}

std::ostream &operator<< (std::ostream &os, global_broker_id id)
{
    os << id.baseValue ();
    return os;
}

std::ostream &operator<< (std::ostream &os, global_federate_id id)
{
    os << id.baseValue ();
    return os;
}

std::ostream &operator<< (std::ostream &os, global_handle id)
{
    os << id.fed_id.baseValue()<<"::"<<id.handle.baseValue();
    return os;
}

std::ostream &operator<< (std::ostream &os, route_id id)
{
    os << id.baseValue ();
    return os;
}
}
