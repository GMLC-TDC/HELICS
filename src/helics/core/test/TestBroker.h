/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../NetworkBroker.hpp"

namespace helics
{
namespace testcore
{
class TestComms;

/** implementation for the core that uses IPC messages to communicate*/
using TestBroker = NetworkBroker<TestComms, interface_type::inproc, static_cast<int> (core_type::TEST)>;

}  // namespace test
}  // namespace helics

