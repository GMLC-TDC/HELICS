/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/helicsCLI11.hpp"
#include "helics/network/NetworkBrokerData.hpp"

#include "gtest/gtest.h"

TEST(networkData_tests, basic_test)
{
    helics::NetworkBrokerData bdata;
    auto parser = bdata.commandLineParser("local");
    parser->helics_parse("--broker=bob --interface=harry --ipv4");
    EXPECT_EQ(bdata.brokerAddress, "bob");
    EXPECT_EQ(bdata.localInterface, "harry");
    EXPECT_TRUE(bdata.interfaceNetwork == helics::interface_networks::ipv4);
    parser->helics_parse("--brokername=tom --brokerport=20755 --port 45");
    EXPECT_EQ(bdata.brokerName, "tom");
    EXPECT_EQ(bdata.brokerPort, 20755);
    EXPECT_EQ(bdata.portNumber, 45);
}

TEST(networkData_tests, networkbrokerdata_stripProtocol_test)
{
    EXPECT_EQ(helics::stripProtocol("tcp://127.0.0.1"), "127.0.0.1");
}

TEST(networkData_tests, networkbrokerdata_removeProtocol_test)
{
    std::string networkAddress = "tcp://127.0.0.1";
    helics::removeProtocol(networkAddress);
    EXPECT_EQ(networkAddress, "127.0.0.1");
}

TEST(networkData_tests, networkbrokerdata_addProtocol_test)
{
    EXPECT_EQ(helics::addProtocol("127.0.0.1", helics::interface_type::tcp), "tcp://127.0.0.1");
}

TEST(networkData_tests, networkbrokerdata_insertProtocol_test)
{
    std::string networkAddress = "127.0.0.1";
    helics::insertProtocol(networkAddress, helics::interface_type::tcp);
    EXPECT_EQ(networkAddress, "tcp://127.0.0.1");
}

TEST(networkData_tests, add_check_detection)
{
    EXPECT_TRUE(helics::isipv6("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210"));
    EXPECT_TRUE(helics::isipv6("::192.9.5.5"));
    EXPECT_TRUE(helics::isipv6("http://[1080::8:800:200C:417A]/foo"));
    EXPECT_TRUE(helics::isipv6("::0"));
    EXPECT_TRUE(!helics::isipv6("192.9.5.5"));
    EXPECT_TRUE(!helics::isipv6("tcp://192.9.5.5:80"));
}

TEST(networkData_tests, local_address_test)
{
    auto netw = helics::getLocalExternalAddressV4();
    EXPECT_TRUE(!helics::isipv6(netw));
    EXPECT_TRUE(!netw.empty());
    auto netw2 = helics::getLocalExternalAddressV4("www.google.com");
    EXPECT_TRUE(!helics::isipv6(netw2));
    EXPECT_TRUE(!netw2.empty());
}

TEST(networkData_tests, local_address_testv6)
{
    try {
        auto netw = helics::getLocalExternalAddressV6();
        EXPECT_TRUE(!netw.empty());
        EXPECT_TRUE(helics::isipv6(netw));
        auto netw2 = helics::getLocalExternalAddressV6("2001:db8::1");
        EXPECT_TRUE(helics::isipv6(netw2));
        EXPECT_TRUE(!netw2.empty());
    }
    catch (...) {
        GTEST_SKIP_("ipv6 not supported");
    }
}
