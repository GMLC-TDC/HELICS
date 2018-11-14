/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Endpoints.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
	Endpoint::Endpoint(interface_visibility locality,
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
}

