/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/NetworkBrokerData.hpp"
#include "helics/common/stringToCmdLine.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(networkData_tests, *utf::label("daily") *utf::label("release"))

BOOST_AUTO_TEST_CASE(basic_test)
{
    helics::NetworkBrokerData bdata;
    StringToCmdLine cmd("--broker=bob --interface=harry --ipv4");
    bdata.initializeFromArgs(cmd.getArgCount(), cmd.getArgV(), "local");
    BOOST_CHECK_EQUAL(bdata.brokerAddress, "bob");
    BOOST_CHECK_EQUAL(bdata.localInterface, "harry");
    BOOST_CHECK(bdata.interfaceNetwork == helics::interface_networks::ipv4);
    StringToCmdLine cmd2("--brokername=tom --brokerport=20755 --port 45");
    bdata.initializeFromArgs(cmd2.getArgCount(), cmd2.getArgV(), "local");
    BOOST_CHECK_EQUAL(bdata.brokerName, "tom");
    BOOST_CHECK_EQUAL(bdata.brokerPort, 20755);
    BOOST_CHECK_EQUAL(bdata.portNumber, 45);
}


BOOST_AUTO_TEST_CASE(add_check_detection)
{
    BOOST_CHECK(helics::isipv6("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210"));
    BOOST_CHECK(helics::isipv6("::192.9.5.5"));
    BOOST_CHECK(helics::isipv6("http://[1080::8:800:200C:417A]/foo"));
    BOOST_CHECK(helics::isipv6("::0"));
    BOOST_CHECK(!helics::isipv6("192.9.5.5"));
    BOOST_CHECK(!helics::isipv6("tcp://192.9.5.5:80"));
}

BOOST_AUTO_TEST_CASE(local_address_test)
{
    auto netw = helics::getLocalExternalAddressV4();
    BOOST_CHECK(!helics::isipv6(netw));
    BOOST_CHECK(!netw.empty());
    auto netw2 = helics::getLocalExternalAddressV4("www.google.com");
    BOOST_CHECK(!helics::isipv6(netw2));
    BOOST_CHECK(!netw2.empty());
}

BOOST_AUTO_TEST_CASE(local_address_testv6)
{
    auto netw = helics::getLocalExternalAddressV6();
    BOOST_CHECK(!netw.empty());
    BOOST_CHECK(helics::isipv6(netw));
    auto netw2 = helics::getLocalExternalAddressV6("2001:db8::1");
    BOOST_CHECK(helics::isipv6(netw2));
    BOOST_CHECK(!netw2.empty());
}
BOOST_AUTO_TEST_SUITE_END()
