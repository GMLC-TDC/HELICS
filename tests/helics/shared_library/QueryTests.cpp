/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ctestFixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (query_tests, FederateTestFixture, *utf::label("ci"))

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_publication_queries, bdata::make (core_types), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);
    
    // register the publications

    helicsFederateRegisterGlobalPublication(vFed1, "pub1", "double", "");
    helicsFederateRegisterPublication(vFed1, "pub2", "double", "");
    helicsFederateRegisterPublication(vFed2, "pub3", "double", "");
    CE(helicsFederateEnterInitializationModeAsync(vFed1));
    CE(helicsFederateEnterInitializationMode(vFed2));
    CE(helicsFederateEnterInitializationModeComplete(vFed1));

    auto core = helicsFederateGetCoreObject(vFed1);

    auto q1 = helicsCreateQuery("fed0", "publications");
    
    std::string res(helicsQueryCoreExecute(q1, core));
    
    BOOST_CHECK_EQUAL (res, "[pub1;fed0/pub2]");

    std::string res2(helicsQueryExecute(q1, vFed2));
    BOOST_CHECK_EQUAL(res2, "[pub1;fed0/pub2]");

    helicsQueryFree(q1);
    q1 = helicsCreateQuery("fed1", "isinit");
   
    res = helicsQueryExecute(q1, vFed1);
    BOOST_CHECK_EQUAL (res, "true");
    helicsQueryFree(q1);

    q1 = helicsCreateQuery("fed1", "publications");
    res = helicsQueryExecute(q1, vFed1);
    BOOST_CHECK_EQUAL (res, "[fed1/pub3]");
    helicsQueryFree(q1);
    helicsCoreFree(core);
    CE(helicsFederateFinalize(vFed1));
    CE(helicsFederateFinalize(vFed2));
}


BOOST_DATA_TEST_CASE (test_broker_queries, bdata::make (core_types), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 2);
    auto vFed1 = GetFederateAt(0);
    auto vFed2 = GetFederateAt(1);

    // register the publications

    

    auto core = helicsFederateGetCoreObject(vFed1);

    auto q1 = helicsCreateQuery("root", "federates");
    std::string res = helicsQueryCoreExecute(q1, core);
    std::string str ("[");
    std::string name;
    name.resize(100);
    CE(helicsFederateGetName(vFed1, &(name[0]), 100));
    str.append (name.c_str());
    str.push_back (';');
    CE(helicsFederateGetName(vFed2, &(name[0]), 100));
    str.append (name.c_str());
    str.push_back (']');

    BOOST_CHECK_EQUAL (res, str);

    std::string res2 = helicsQueryExecute(q1, vFed1);
    BOOST_CHECK_EQUAL(res2, str);
    CE(helicsFederateEnterInitializationModeAsync(vFed1));
    CE(helicsFederateEnterInitializationMode(vFed2));
    CE(helicsFederateEnterInitializationModeComplete(vFed1));
    helicsQueryFree(q1);
    helicsCoreFree(core);
    CE(helicsFederateFinalize(vFed1));
    CE(helicsFederateFinalize(vFed2));
}

BOOST_AUTO_TEST_SUITE_END ()
