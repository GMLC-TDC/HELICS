/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "gmlc/networking/addressOperations.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics/network/NetworkBrokerData.hpp"

#include "gtest/gtest.h"

TEST(networkData_tests, basic_test)
{
    helics::NetworkBrokerData bdata;
    auto parser = bdata.commandLineParser("local");
    parser->helics_parse("--broker=bob --local_interface=harry --ipv4");
    EXPECT_EQ(bdata.brokerAddress, "bob");
    EXPECT_EQ(bdata.localInterface, "harry");
    EXPECT_TRUE(bdata.interfaceNetwork == helics::InterfaceNetworks::IPV4);
    parser->helics_parse("--brokername=tom --brokerport=20755 --port 45");
    EXPECT_EQ(bdata.brokerName, "tom");
    EXPECT_EQ(bdata.brokerPort, 20755);
    EXPECT_EQ(bdata.portNumber, 45);
}

TEST(networkData, local_address_test)
{
    auto netw = helics::getLocalExternalAddressV4();
    EXPECT_TRUE(!gmlc::networking::isIpv6(netw));
    EXPECT_TRUE(!netw.empty());
    auto netw2 = helics::getLocalExternalAddressV4("www.google.com");
    EXPECT_TRUE(!gmlc::networking::isIpv6(netw2));
    EXPECT_TRUE(!netw2.empty());
}

TEST(networkData, local_address_testv6)
{
    try {
        auto netw = helics::getLocalExternalAddressV6();
        EXPECT_TRUE(!netw.empty());
        EXPECT_TRUE(gmlc::networking::isIpv6(netw));
        auto netw2 = helics::getLocalExternalAddressV6("2001:db8::1");
        EXPECT_TRUE(gmlc::networking::isIpv6(netw2));
        EXPECT_TRUE(!netw2.empty());
    }
    catch (...) {
        GTEST_SKIP_("ipv6 not supported");
    }
}
