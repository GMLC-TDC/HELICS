/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.BrokerFactory.h

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/helics-config.h"

#ifndef BOOST_STATIC
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE core_tests
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include <helics/core/BrokerFactory.hpp>
#include <helics/core/CoreFactory.hpp>
#include <boost/test/unit_test.hpp>

struct globalTestConfig
{
    globalTestConfig () {}
    ~globalTestConfig ()
    {
        helics::CoreFactory::cleanUpCores ();
        helics::BrokerFactory::cleanUpBrokers ();
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);
