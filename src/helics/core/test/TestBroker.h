/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

