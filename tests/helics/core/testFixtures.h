/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
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

struct coreBrokerTestFixture
{
    coreBrokerTestFixture ();
    ~coreBrokerTestFixture ();

    std::unique_ptr<helics::CoreBroker> broker;
};

struct commonCoreTestFixture
{
    commonCoreTestFixture ();
    ~commonCoreTestFixture ();

    std::unique_ptr<helics::CommonCore> core;
};

