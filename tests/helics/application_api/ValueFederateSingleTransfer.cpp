/*
Copyright (c) 2017-2021,
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

/** these test cases test out the value federates
 */
class valuefed_single_transfer:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(valuefed_single_transfer, types1)
{
    runFederateTest<double>(GetParam(), 10.3, 45.3, 22.7);
}

TEST_P(valuefed_single_transfer, types2)
{
    runFederateTest<double>(GetParam(), 1.0, 0.0, 3.0);
}

TEST_P(valuefed_single_transfer, types3)
{
    runFederateTest<int>(GetParam(), 5, 8, 43);
}

TEST_P(valuefed_single_transfer, types4)
{
    runFederateTest<int>(GetParam(), -5, 1241515, -43);
}

TEST_P(valuefed_single_transfer, types5)
{
    runFederateTest<int16_t>(GetParam(), -5, 23023, -43);
}

TEST_P(valuefed_single_transfer, types6)
{
    runFederateTest<uint64_t>(GetParam(), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

TEST_P(valuefed_single_transfer, types7)
{
    runFederateTest<float>(GetParam(), 10.3f, 45.3f, 22.7f);
}

TEST_P(valuefed_single_transfer, types8)
{
    runFederateTest<std::string>(GetParam(),
                                 "start",
                                 "inside of the functional relationship of helics",
                                 std::string("I am a string"));
}

TEST_P(valuefed_single_transfer, types9)
{
    runFederateTestv2<std::vector<double>>(GetParam(),
                                           {34.3, 24.2},
                                           {12.4, 14.7, 16.34, 18.17},
                                           {9.9999, 8.8888, 7.7777});
}

TEST_P(valuefed_single_transfer, types10)
{
    // this is a bizarre string since it contains a \0 and in icc 17 can't be used inside a boost
    // data test case for some unknown reason
    decltype(auto) cstr = "this is the third\0 string";
    std::string specialString(cstr, sizeof(cstr));
    std::vector<std::string> sv1{"hip", "hop"};
    std::vector<std::string> sv2{"this is the first string\n",
                                 "this is the second string",
                                 specialString};
    std::vector<std::string> sv3{
        "string1", "String2", "string3", "string4", "string5", "string6", "string8"};
    runFederateTestv2(GetParam(), sv1, sv2, sv3);
}

TEST_P(valuefed_single_transfer, types11)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTest<std::complex<double>>(GetParam(), def, v1, v2);
}

TEST_P(valuefed_single_transfer, types_publishers1)
{
    runFederateTestObj<double>(GetParam(), 10.3, 45.3, 22.7);
}

TEST_P(valuefed_single_transfer, types_publishers2)
{
    runFederateTestObj<double>(GetParam(), 1.0, 0.0, 3.0);
}

TEST_P(valuefed_single_transfer, types_publishers3)
{
    runFederateTestObj<int>(GetParam(), 5, 8, 43);
}

TEST_P(valuefed_single_transfer, types_publishers4)
{
    runFederateTestObj<int>(GetParam(), -5, 1241515, -43);
}

TEST_P(valuefed_single_transfer, types_publishers5)
{
    runFederateTestObj<int16_t>(GetParam(), -5, 23023, -43);
}

TEST_P(valuefed_single_transfer, types_publishers6)
{
    runFederateTestObj<uint64_t>(GetParam(), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

TEST_P(valuefed_single_transfer, publishers7)
{
    runFederateTestObj<float>(GetParam(), 10.3f, 45.3f, 22.7f);
}

TEST_P(valuefed_single_transfer, types_publishers8)
{
    runFederateTestObj<std::string>(GetParam(),
                                    "start",
                                    "inside of the functional relationship of helics",
                                    std::string("I am a string"));
}

TEST_P(valuefed_single_transfer, types_publishers9)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTestObj<std::complex<double>>(GetParam(), def, v1, v2);
}
INSTANTIATE_TEST_SUITE_P(valuefed_single_transfer_ci_skip,
                         valuefed_single_transfer,
                         ::testing::ValuesIn(core_types_single));
