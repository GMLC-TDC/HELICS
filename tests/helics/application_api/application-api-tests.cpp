/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

//#include <iostream>
#include <helics/application_api/Federate.h>

#ifndef _MSC_VER
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
        // std::cout << "cleaning up" << std::endl;
        helics::cleanupHelicsLibrary ();
    }
};

//____________________________________________________________________________//

BOOST_GLOBAL_FIXTURE (globalTestConfig);