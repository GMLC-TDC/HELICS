/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/helics-config.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (broker_tests, *utf::label ("ci"))

/** test the assignment and retrieval of global value from a broker object*/
BOOST_AUTO_TEST_CASE (global_value_test)
{
    auto brk = helics::BrokerFactory::create (helics::core_type::TEST, "-f2 --root");
    std::string globalVal = "this is a string constant that functions as a global";
    std::string globalVal2 = "this is a second string constant that functions as a global";
    brk->setGlobal ("testglobal", globalVal);

    auto res = brk->query ("global", "testglobal");
    BOOST_CHECK_EQUAL (res, globalVal);
    brk->setGlobal ("testglobal2", globalVal2);

    res = brk->query ("global", "testglobal");
    BOOST_CHECK_EQUAL (res, globalVal);
    res = brk->query ("global", "testglobal2");
    BOOST_CHECK_EQUAL (res, globalVal2);
    brk->disconnect ();
    BOOST_CHECK (!brk->isConnected ());
}

BOOST_AUTO_TEST_SUITE_END ()
