/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

