/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.BrokerFactory.h
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
