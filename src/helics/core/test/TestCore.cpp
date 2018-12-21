/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../NetworkCore_impl.hpp"
#include "TestComms.h"
#include "TestCore.h"

namespace helics
{
template class NetworkCore<testcore::TestComms, interface_type::inproc>;
}  // namespace helics
