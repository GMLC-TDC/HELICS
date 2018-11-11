/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../NetworkBroker_impl.hpp"
#include "TestBroker.h"
#include "TestComms.h"

namespace helics
{
template class NetworkBroker<testcore::TestComms, interface_type::inproc, static_cast<int> (core_type::TEST)>;
}  // namespace helics
