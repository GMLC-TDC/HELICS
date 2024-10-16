/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/CombinationFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <string>

/** these test cases test out the value converters and some of the other functions
 */

struct cfed_tests: public FederateTestFixture_cpp, public ::testing::Test {};

class cfed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {};

// const std::string CoreTypes[] = { "test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2" };

/** test block send and receive*/

//
TEST_F(cfed_tests, test_file_load)
{
    // fedInfo = helicsCreateFederateInfo();
    // path of the JSON file is hardcoded for now
    helicscpp::CombinationFederate cFed(TEST_DIR "/combo_config.json");
    ASSERT_TRUE(cFed.baseObject() != nullptr);
    std::string s = cFed.getName();
    EXPECT_EQ(s, "combo_fed");
    EXPECT_EQ(cFed.getPublicationCount(), 4);
    EXPECT_EQ(cFed.getEndpointCount(), 1);
    EXPECT_EQ(cFed.getFilterCount(), 0);
    EXPECT_EQ(cFed.getInputCount(), 4);
    EXPECT_EQ(cFed.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD), 1.3);
    //  helicscpp::ValueFederate vFed(std::string(TEST_DIR) +
    //  "/test_files/example_value_fed.json");
    cFed.finalize();
}
