/*
Copyright (c) 2017-2024,
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
    EXPECT_TRUE(bdata.interfaceNetwork == gmlc::networking::InterfaceNetworks::IPV4);
    parser->helics_parse("--brokername=tom --brokerport=20755 --port 45");
    EXPECT_EQ(bdata.brokerName, "tom");
    EXPECT_EQ(bdata.brokerPort, 20755);
    EXPECT_EQ(bdata.portNumber, 45);
    parser->helics_parse("--encrypted --encryption_config=openssl.json");
    EXPECT_EQ(bdata.encrypted, true);
    EXPECT_EQ(bdata.encryptionConfig, "openssl.json");
}
