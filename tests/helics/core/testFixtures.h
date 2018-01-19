/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_FIXTURES_HEADER_
#define TEST_FIXTURES_HEADER_

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

#endif