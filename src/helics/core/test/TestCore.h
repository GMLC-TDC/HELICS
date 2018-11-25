/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../NetworkCore.hpp"

namespace helics
{
namespace testcore
{
class TestComms;
/** implementation for the core that can only communicate in process*/
using TestCore = NetworkCore<TestComms, interface_type::inproc>;

}  // namespace testcore
}  // namespace helics

