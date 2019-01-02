/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ValueFederateTestTemplates.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <future>
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"

/** these test cases test out the value federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_dual_transfer, FederateTestFixture)

/** test case checking that the transfer between two federates works as expected
 */
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types1, bdata::make (core_types), core_type)
{
    runDualFederateTest<double> (core_type, 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types2, bdata::make (core_types), core_type)
{
    runDualFederateTest<int> (core_type, 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types3, bdata::make (core_types), core_type)
{
    runDualFederateTest<int> (core_type, -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types4, bdata::make (core_types), core_type)
{
    runDualFederateTest<char> (core_type, 'c', '\0', '\n');
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types5, bdata::make (core_types), core_type)
{
    runDualFederateTest<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types6, bdata::make (core_types), core_type)
{
    runDualFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types7, bdata::make (core_types), core_type)
{
    runDualFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                      std::string ("I am a string"));
}

/** test case checking that the transfer between two federates works as expected with publication and subscription
 * objects
 */
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj1, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<double> (core_type, 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj2, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<int> (core_type, 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj3, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<int> (core_type, -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj4, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<char> (core_type, 'c', '\0', '\n');
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj5, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<uint64_t> (core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj6, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<float> (core_type, 10.3f, 45.3f, 22.7f);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj7, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<std::string> (core_type, "start", "inside of the functional relationship of helics",
                                         std::string ("I am a string"));
}
BOOST_AUTO_TEST_SUITE_END ()
