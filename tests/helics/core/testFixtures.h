/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <memory>
namespace helics
{
class FederateState;
class CoreBroker;
class CommonCore;
}  // namespace helics

struct federateStateTestFixture
{
    federateStateTestFixture ();
    ~federateStateTestFixture ();

    std::unique_ptr<helics::FederateState> fs;
};
