/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
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
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

/** these test cases test out the value federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_single_transfer, FederateTestFixture)

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types1, bdata::make (core_types_single), core_type)
{
    runFederateTest<double> (core_type, 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types2, bdata::make (core_types_single), core_type)
{
    runFederateTest<double> (core_type, 1.0, 0.0, 3.0);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types3, bdata::make (core_types_single), core_type)
{
    runFederateTest<int> (core_type, 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types4, bdata::make (core_types_single), core_type)
{
    runFederateTest<int> (core_type, -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types5, bdata::make (core_types_single), core_type)
{
    runFederateTest<int16_t> (core_type, -5, 23023, -43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types6, bdata::make (core_types_single), core_type)
{
    runFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types7, bdata::make (core_types_single), core_type)
{
    runFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types8, bdata::make (core_types_single), core_type)
{
    runFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                  std::string ("I am a string"));
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types9, bdata::make (core_types_single), core_type)
{
    runFederateTestv2<std::vector<double>> (core_type, {34.3, 24.2}, {12.4, 14.7, 16.34, 18.17},
                                            {9.9999, 8.8888, 7.7777});
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types10, bdata::make (core_types_single), core_type)
{
    // this is a bizarre string since it contains a \0 and in icc 17 can't be used inside a boost data test case
    // for some unknown reason
    decltype (auto) cstr = "this is the third\0 string";
    std::string specialString (cstr, sizeof (cstr));
    std::vector<std::string> sv1{"hip", "hop"};
    std::vector<std::string> sv2{"this is the first string\n", "this is the second string", specialString};
    std::vector<std::string> sv3{"string1", "String2", "string3", "string4", "string5", "string6", "string8"};
    runFederateTestv2 (core_type, sv1, sv2, sv3);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types11, bdata::make (core_types_single), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers1, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<double> (core_type, 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers2, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<double> (core_type, 1.0, 0.0, 3.0);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers3, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int> (core_type, 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers4, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int> (core_type, -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers5, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<int16_t> (core_type, -5, 23023, -43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers6, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers7, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<float> (core_type, 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers8, bdata::make (core_types_single), core_type)
{
    runFederateTestObj<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                     std::string ("I am a string"));
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_types_publishers9, bdata::make (core_types_single), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTestObj<std::complex<double>> (core_type, def, v1, v2);
}

BOOST_AUTO_TEST_SUITE_END ()
