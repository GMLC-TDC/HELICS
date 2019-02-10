/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier:
BSD-3-ClauseBrokerFactory.h
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
    globalTestConfig () = default;
    ~globalTestConfig ()
    {
        helics::CoreFactory::cleanUpCores ();
        helics::BrokerFactory::cleanUpBrokers ();
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);
