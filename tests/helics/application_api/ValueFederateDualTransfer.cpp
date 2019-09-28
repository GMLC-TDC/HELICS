/*
Copyright (c) 2017-2019,
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
#include <gtest/gtest.h>

/** these test cases test out the value federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_dual_transfer, FederateTestFixture)

/** test case checking that the transfer between two federates works as expected
 */
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types1, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<double> (GetParam (), 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types2, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<int> (GetParam (), 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types3, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<int> (GetParam (), -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types4, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<char> (GetParam (), 'c', '\0', '\n');
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types5, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<uint64_t> (GetParam (), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types6, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<float> (GetParam (), 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types7, bdata::make (core_types), GetParam ())
{
    runDualFederateTest<std::string> (GetParam (), "start", "inside of the functional relationship of helics",
                                      std::string ("I am a string"));
}

/** test case checking that the transfer between two federates works as expected with publication and subscription
 * objects
 */
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj1, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<double> (GetParam (), 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj2, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<int> (GetParam (), 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj3, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<int> (GetParam (), -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj4, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<char> (GetParam (), 'c', '\0', '\n');
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj5, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<uint64_t> (GetParam (), 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj6, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<float> (GetParam (), 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj7, bdata::make (core_types), GetParam ())
{
    runDualFederateTestObj<std::string> (GetParam (), "start", "inside of the functional relationship of helics",
                                         std::string ("I am a string"));
}
