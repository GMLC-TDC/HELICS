/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TestCore.h"
#include "../NetworkCore_impl.hpp"
#include "TestComms.h"

namespace helics
{
template class NetworkCore<testcore::TestComms, interface_type::inproc>;
}  // namespace helics
