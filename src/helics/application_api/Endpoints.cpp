/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Endpoints.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
Endpoint::Endpoint(MessageFederate *mFed, const std::string &name, const std::string &type)
    : fed(mFed)
{
    try
    {
        id = fed->registerEndpoint(name, type);
    }
    catch (const RegistrationFailure &)
    {
        id = fed->getEndpointId(name);
    }
    actualName = fed->getEndpointName(id);
}

Endpoint::Endpoint(interface_visibility locality,
    MessageFederate *mFed,
    const std::string &name,
    const std::string &type)
    : fed(mFed)
{
    try
    {
        id = (locality == GLOBAL) ? fed->registerGlobalEndpoint(name, type) : fed->registerEndpoint(name, type);
    }
    catch (const RegistrationFailure &)
    {
        id = fed->getEndpointId(name);
    }
    actualName = fed->getEndpointName(id);
}

Endpoint::Endpoint (MessageFederate *mFed, int endpointIndex) : fed (mFed)
{
    auto cnt = fed->getEndpointCount ();
    if ((endpointIndex >= cnt) || (cnt < 0))
    { 
        throw (helics::InvalidParameter ("no endpoint with the specified index"));
    }
    id = static_cast<endpoint_id_t> (endpointIndex);
    actualName = fed->getEndpointName (id);
}

void Endpoint::setCallback (std::function<void(const Endpoint *, Time)> callback)
{
    // copy by value otherwise this object can't be used in vectors by itself
    Endpoint local (*this);
    fed->registerEndpointCallback (id, [callback, local](endpoint_id_t, Time messageTime) {
        callback (&local, messageTime);
    });
}
}  // namespace helics
