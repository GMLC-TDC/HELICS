/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Endpoints.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
Endpoint::Endpoint (interface_visibility locality,
                    MessageFederate *mFed,
                    const std::string &name,
                    const std::string &type)
{
    if (locality == interface_visibility::global)
    {
        operator= (mFed->registerGlobalEndpoint (name, type));
    }
    else
    {
        operator= (mFed->registerEndpoint (name, type));
    }
}
}  // namespace helics
