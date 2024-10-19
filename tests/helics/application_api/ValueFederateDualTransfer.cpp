/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <string>

/** these test cases test out the value federates
 */
class vfed_dual_transfer_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

/** test case checking that the transfer between two federates works as expected
 */
TEST_P(vfed_dual_transfer_ci_skip, double1)
{
    runDualFederateTest<double>(GetParam(), 10.3, 45.3, 22.7);
}

TEST_P(vfed_dual_transfer_ci_skip, int1)
{
    runDualFederateTest<int>(GetParam(), 5, 8, 43);
}

TEST_P(vfed_dual_transfer_ci_skip, int2)
{
    runDualFederateTest<int>(GetParam(), -5, 1241515, -43);
}

TEST_P(vfed_dual_transfer_ci_skip, types4)
{
    runDualFederateTest<char>(GetParam(), 'c', '\0', '\n');
}

TEST_P(vfed_dual_transfer_ci_skip, uint64_t1)
{
    runDualFederateTest<uint64_t>(GetParam(), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

TEST_P(vfed_dual_transfer_ci_skip, float1)
{
    runDualFederateTest<float>(GetParam(), 10.3F, 45.3F, 22.7F);
}

TEST_P(vfed_dual_transfer_ci_skip, string)
{
    runDualFederateTest<std::string>(GetParam(),
                                     "start",
                                     "inside of the functional relationship of helics",
                                     std::string("I am a string"));
}
/** test checking the transfer when 1 federate is json */

TEST(vfed_dual_transfer_json, double1)
{
    runDualFederateTestJson<double>("test", 10.3, 45.3, 22.7);
}

TEST(vfed_dual_transfer_json, int1)
{
    runDualFederateTestJson<int>("test", 5, 8, 43);
}

TEST(vfed_dual_transfer_json, int2)
{
    runDualFederateTestJson<int>("test", -5, 1241515, -43);
}

TEST(vfed_dual_transfer_json, types4)
{
    runDualFederateTestJson<char>("test", 'c', '\0', '\n');
}

TEST(vfed_dual_transfer_json, uint64_t1)
{
    runDualFederateTestJson<uint64_t>("test", 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

TEST(vfed_dual_transfer_json, float1)
{
    runDualFederateTestJson<float>("test", 10.3F, 45.3F, 22.7F);
}

TEST(vfed_dual_transfer_json, string)
{
    runDualFederateTestJson<std::string>("test",
                                         "start",
                                         "inside of the functional relationship of helics",
                                         std::string("I am a string"));
}

/** test case checking that the transfer between two federates works as expected with publication
 * and subscription objects
 */
TEST_P(vfed_dual_transfer_ci_skip, obj_double)
{
    runDualFederateTestObj<double>(GetParam(), 10.3, 45.3, 22.7);
}

TEST_P(vfed_dual_transfer_ci_skip, obj_int1)
{
    runDualFederateTestObj<int>(GetParam(), 5, 8, 43);
}

TEST_P(vfed_dual_transfer_ci_skip, obj_int2)
{
    runDualFederateTestObj<int>(GetParam(), -5, 1241515, -43);
}

TEST_P(vfed_dual_transfer_ci_skip, obj_char)
{
    runDualFederateTestObj<char>(GetParam(), 'c', '\0', '\n');
}

TEST_P(vfed_dual_transfer_ci_skip, obj_uint64_t)
{
    runDualFederateTestObj<uint64_t>(GetParam(), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

TEST_P(vfed_dual_transfer_ci_skip, obj_float)
{
    runDualFederateTestObj<float>(GetParam(), 10.3F, 45.3F, 22.7F);
}

TEST_P(vfed_dual_transfer_ci_skip, obj_string)
{
    runDualFederateTestObj<std::string>(GetParam(),
                                        "start",
                                        "inside of the functional relationship of helics",
                                        std::string("I am a string"));
}

INSTANTIATE_TEST_SUITE_P(vfed_dual_transfer_tests,
                         vfed_dual_transfer_ci_skip,
                         ::testing::ValuesIn(CoreTypes));
