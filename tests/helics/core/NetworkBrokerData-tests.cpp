/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include <numeric>

//#include "boost/process.hpp"
#include <future>

namespace utf = boost::unit_test;
using namespace std::literals::chrono_literals;

BOOST_AUTO_TEST_SUITE (NetworkBrokerData_tests, *utf::label("ci"))

BOOST_AUTO_TEST_CASE(networkbrokerdata_stripProtocol_test)
{
    BOOST_CHECK_EQUAL(stripProtocol("tcp://127.0.0.1"), "127.0.0.1");
    BOOST_CHECK_EQUAL(stripProtocol("udp://127.0.0.1"), "127.0.0.1");
    BOOST_CHECK_EQUAL(stripProtocol("http://127.0.0.1"), "127.0.0.1");
    BOOST_CHECK_EQUAL(stripProtocol("https://127.0.0.1"), "127.0.0.1");
}


BOOST_AUTO_TEST_SUITE_END ()

