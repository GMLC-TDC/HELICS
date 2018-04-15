/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/chelics.h"
#include "helics/helics-config.h"

#ifndef BOOST_STATIC
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE cpp - shared - library - tests
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include <iostream>
#include <boost/test/unit_test.hpp>
struct globalTestConfig
{
    globalTestConfig () = default;
    ~globalTestConfig ()
    {
        // std::cout << "cleaning up" << std::endl;
        helicsCloseLibrary ();
        // std::cout << "finished cleaning up" << std::endl;
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);
