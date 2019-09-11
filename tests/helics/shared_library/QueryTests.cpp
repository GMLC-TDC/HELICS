/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ctestFixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (query_tests, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_publication_queries, bdata::make (core_types), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    // register the publications

    helicsFederateRegisterGlobalTypePublication (vFed1, "pub1", "double", "", &err);
    helicsFederateRegisterTypePublication (vFed1, "pub2", "double", "", &err);
    helicsFederateRegisterTypePublication (vFed2, "pub3", "double", "", &err);
    CE (helicsFederateEnterInitializingModeAsync (vFed1, &err));
    CE (helicsFederateEnterInitializingMode (vFed2, &err));
    CE (helicsFederateEnterInitializingModeComplete (vFed1, &err));

    auto core = helicsFederateGetCoreObject (vFed1, &err);

    auto q1 = helicsCreateQuery ("fed0", "publications");

    CE (std::string res (helicsQueryCoreExecute (q1, core, &err)));

    BOOST_CHECK_EQUAL (res, "[pub1;fed0/pub2]");

    CE (std::string res2 = helicsQueryExecute (q1, vFed2, &err));
    BOOST_CHECK_EQUAL (res2, "[pub1;fed0/pub2]");

    helicsQueryFree (q1);
    q1 = helicsCreateQuery ("fed1", "isinit");

    CE (res = helicsQueryExecute (q1, vFed1, &err));
    BOOST_CHECK_EQUAL (res, "true");
    helicsQueryFree (q1);

    q1 = helicsCreateQuery ("fed1", "publications");
    CE (res = helicsQueryExecute (q1, vFed1, &err));
    BOOST_CHECK_EQUAL (res, "[fed1/pub3]");
    helicsQueryFree (q1);
    helicsCoreFree (core);
    CE (helicsFederateFinalizeAsync (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
    CE (helicsFederateFinalizeComplete (vFed1, &err));
}

BOOST_DATA_TEST_CASE (test_broker_queries, bdata::make (core_types), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 2);
    auto vFed1 = GetFederateAt (0);
    auto vFed2 = GetFederateAt (1);

    CE (auto core = helicsFederateGetCoreObject (vFed1, &err));

    auto q1 = helicsCreateQuery ("root", "federates");
    std::string res = helicsQueryCoreExecute (q1, core, nullptr);
    std::string str ("[");
    str.append (helicsFederateGetName (vFed1));
    str.push_back (';');
    str.append (helicsFederateGetName (vFed2));
    str.push_back (']');

    BOOST_CHECK_EQUAL (res, str);

    CE (std::string res2 = helicsQueryExecute (q1, vFed1, &err));
    BOOST_CHECK_EQUAL (res2, str);
    CE (helicsFederateEnterInitializingModeAsync (vFed1, &err));
    CE (helicsFederateEnterInitializingMode (vFed2, &err));
    CE (helicsFederateEnterInitializingModeComplete (vFed1, &err));
    helicsQueryFree (q1);
    helicsCoreFree (core);
    CE (helicsFederateFinalizeAsync (vFed1, &err));
    CE (helicsFederateFinalize (vFed2, &err));
    CE (helicsFederateFinalizeComplete (vFed1, &err));
}

BOOST_AUTO_TEST_SUITE_END ()
