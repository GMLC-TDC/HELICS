/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Endpoints.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
Endpoint::Endpoint (MessageFederate *mFed, int endpointIndex) : fed (mFed)
{
    auto cnt = fed->getEndpointCount ();
    if ((endpointIndex >= cnt) || (cnt < 0))
    {
        throw (helics::InvalidParameter ("no subscription with the specified index"));
    }
    id = static_cast<endpoint_id_t> (endpointIndex);
    actualName = fed->getEndpointName(id);
}

void Endpoint::setCallback(std::function<void(const Endpoint *, Time)> callback)
{
    //copy by value otherwise this object can't be used in vectors by itself
    Endpoint local(*this);
    fed->registerEndpointCallback(id, [callback, local](endpoint_id_t, Time messageTime) {callback(&local, messageTime); });
}
}  // namespace helics