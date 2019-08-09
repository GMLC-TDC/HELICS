/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

//#include <iostream>
#include "helics/helics-config.h"
#include <helics/application_api/Federate.hpp>
//#include <iostream>


#ifndef BOOST_STATIC
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE application_api_tests
#define BOOST_TEST_DETECT_MEMORY_LEAK 0

#include <boost/test/unit_test.hpp>

struct globalTestConfig
{
    globalTestConfig () = default;
    ~globalTestConfig ()
    {
        //    std::cout << "cleaning up" << std::endl;
        helics::cleanupHelicsLibrary ();
        //     std::cout << "finished cleaning up" << std::endl;
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);
