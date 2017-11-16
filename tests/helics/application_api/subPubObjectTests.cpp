/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>
/** these test cases test out the value converters
 */
#include <boost/test/floating_point_comparison.hpp>

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "test_configuration.h"
#include <future>

BOOST_AUTO_TEST_SUITE (subPubObject_tests)

BOOST_AUTO_TEST_CASE (subscriptionTObject_tests)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubObj = helics::PublicationT<std::string> (helics::GLOBAL, vFed.get (), "pub1");

    auto subObj = helics::SubscriptionT<std::string> (vFed.get (), "pub1");
    vFed->setTimeDelta (1.0);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    pubObj.publish ("string1");
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s = subObj.getValue ();

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    pubObj.publish ("string2");
    // make sure the value is still what we expect
    subObj.getValue (s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subObj.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (subscriptionObject_tests)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubObj = helics::make_publication<std::string> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto subObj = helics::Subscription (vFed.get (), "pub1");
    vFed->setTimeDelta (1.0);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    pubObj->publish ("string1");
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s = subObj.getValue<std::string> ();
    // int64_t val = subObj.getValue<int64_t>();
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    pubObj->publish ("string2");
    // make sure the value is still what we expect
    subObj.getValue<std::string> (s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subObj.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed->finalize ();
}

template <class TX, class RX>
void runPubSubTypeTests (const TX &valtx, const RX &valrx)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = CORE_TYPE_TO_TEST;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubObj = helics::make_publication<TX> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto subObj = helics::Subscription (vFed.get (), "pub1");
    vFed->setTimeDelta (1.0);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    pubObj->publish (valtx);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    auto s = subObj.getValue<RX> ();
    // int64_t val = subObj.getValue<int64_t>();
    // make sure the string is what we expect
    BOOST_CHECK (s == valrx);
    vFed->finalize ();
}

#ifdef QUICK_TESTS_ONLY
#define SKIPTEST //
#else
#define SKIPTEST
#endif

BOOST_AUTO_TEST_CASE (subscriptionObject_type_tests)
{
	SKIPTEST runPubSubTypeTests<std::string, std::string> ("test1", "test1");
    runPubSubTypeTests<std::string, double> ("3.14159", 3.14159);
    runPubSubTypeTests<double, std::string> (3.14159, std::to_string (3.141590));
	SKIPTEST runPubSubTypeTests<double, int64_t> (3.14159, 3);
	SKIPTEST runPubSubTypeTests<int64_t, double> (34, 34.0);
	SKIPTEST runPubSubTypeTests<int64_t, std::string> (34, "34");
	SKIPTEST runPubSubTypeTests<std::string, int64_t> ("34.14", 34);
}

BOOST_AUTO_TEST_CASE (subscriptionObject_complex_tests)
{
    using c = std::complex<double>;

    runPubSubTypeTests<c, std::string> (c (12.4, 0.3), helics::helicsComplexString (c (12.4, 0.3)));
	SKIPTEST runPubSubTypeTests<std::string, c> ("3.14159+2j", c (3.14159, 2));
	SKIPTEST runPubSubTypeTests<std::string, c> ("3.14159-2j", c (3.14159, -2));
	SKIPTEST runPubSubTypeTests<std::string, c> ("-3.14159-2j", c (-3.14159, -2));
    runPubSubTypeTests<std::string, c> ("-3.14159 - 2i", c (-3.14159, -2));
	SKIPTEST runPubSubTypeTests<std::string, c> ("-3.14159 + 2i", c (-3.14159, 2));

	SKIPTEST runPubSubTypeTests<std::string, c> ("2i", c (0, 2));
    runPubSubTypeTests<c, double> (c (0, 2), 2.0);
	SKIPTEST runPubSubTypeTests<c, int64_t> (c (0, 2), 2);

	SKIPTEST runPubSubTypeTests<double, c> (2.0, c (2, 0));

	SKIPTEST runPubSubTypeTests<int64_t, c> (2, c (2, 0));
	SKIPTEST runPubSubTypeTests<c, double> (c (3.0, 4.0), 5.0);
	SKIPTEST runPubSubTypeTests<c, int64_t> (c (3.0, 4.0), 5);
}

BOOST_AUTO_TEST_CASE (subscriptionObject_vector_tests)
{
    using v = std::vector<double>;
    using c = std::complex<double>;
    v tvec1{12.4, 0.3, 0.7};
    v tvec2{0.0, -1241.23, 5.0, 7.9};
    v eVec{};
    runPubSubTypeTests<v, std::string> (tvec1, helics::helicsVectorString (tvec1));
    runPubSubTypeTests<std::string, v> (helics::helicsVectorString (tvec1), tvec1);

    runPubSubTypeTests<v, std::string> (tvec2, helics::helicsVectorString (tvec2));
    runPubSubTypeTests<std::string, v> (helics::helicsVectorString (tvec2), tvec2);

	SKIPTEST runPubSubTypeTests<v, std::string> (eVec, helics::helicsVectorString (eVec));
	SKIPTEST runPubSubTypeTests<std::string, v> (helics::helicsVectorString (eVec), eVec);

	SKIPTEST runPubSubTypeTests<std::string, v> ("3.14159-2j", v{3.14159, -2});
	SKIPTEST runPubSubTypeTests<std::string, v> ("-3.14159-2j", v{-3.14159, -2});
	SKIPTEST runPubSubTypeTests<std::string, v> ("-3.14159", v{-3.14159});

	SKIPTEST runPubSubTypeTests<std::string, v> ("2i", v{0, 2});

	SKIPTEST runPubSubTypeTests<c, v> (c{3.14159, -2}, v{3.14159, -2});
	SKIPTEST runPubSubTypeTests<c, v> (c{-3.14159, -2}, v{-3.14159, -2});
	SKIPTEST runPubSubTypeTests<c, v> (c{-3.14159, 0.0}, v{-3.14159, 0.0});

	SKIPTEST runPubSubTypeTests<c, v> (c{0.0, 2}, v{0, 2});

	SKIPTEST runPubSubTypeTests<v, double> (tvec1, 12.4);

	SKIPTEST runPubSubTypeTests<double, v> (0.34, v{0.34});

	SKIPTEST runPubSubTypeTests<v, int64_t> (tvec1, 12);

	SKIPTEST runPubSubTypeTests<int64_t, v> (56, v{56});
}

BOOST_AUTO_TEST_CASE (subscriptionObject_complex_vector_tests)
{
    using v = std::vector<double>;
    using c = std::complex<double>;
    using vc = std::vector<std::complex<double>>;

    v tvec2{0.0, -1241.23, 5.0, 7.9};
    vc eVec{};

    vc tcvec1{c{-4.5, 27.4}, c{0.12, 0.34}};

    vc tcvec2{c{-3.0, -4.0}, c{23.7, 0.0}, c{0.01, 45.23}};

    runPubSubTypeTests<vc, std::string> (tcvec1, helics::helicsComplexVectorString (tcvec1));
    runPubSubTypeTests<std::string, vc> (helics::helicsComplexVectorString (tcvec1), tcvec1);

    runPubSubTypeTests<vc, std::string> (tcvec2, helics::helicsComplexVectorString (tcvec2));
    runPubSubTypeTests<std::string, vc> (helics::helicsComplexVectorString (tcvec2), tcvec2);

    runPubSubTypeTests<vc, std::string> (eVec, helics::helicsComplexVectorString (eVec));
    runPubSubTypeTests<std::string, vc> (helics::helicsComplexVectorString (eVec), eVec);

	SKIPTEST runPubSubTypeTests<std::string, vc> ("3.14159-2j", vc{c{3.14159, -2}});
	SKIPTEST runPubSubTypeTests<std::string, vc> ("-3.14159-2j", vc{c{-3.14159, -2}});
	SKIPTEST runPubSubTypeTests<std::string, vc> ("-3.14159", vc{c{-3.14159}});

	SKIPTEST runPubSubTypeTests<std::string, vc> ("2i", vc{c{0, 2}});

	SKIPTEST runPubSubTypeTests<c, vc> (c{3.14159, -2}, vc{c{3.14159, -2}});
	SKIPTEST runPubSubTypeTests<c, vc> (c{-3.14159, -2}, vc{c{-3.14159, -2}});
	SKIPTEST runPubSubTypeTests<c, vc> (c{-3.14159, 0.0}, vc{c{-3.14159, 0.0}});

	SKIPTEST runPubSubTypeTests<c, vc> (c{0.0, 2}, vc{c{0, 2}});

	SKIPTEST runPubSubTypeTests<vc, double> (tcvec2, 5.0);

	SKIPTEST runPubSubTypeTests<double, vc> (0.34, vc{c{0.34}});

	SKIPTEST runPubSubTypeTests<vc, int64_t> (tcvec2, 5);

	SKIPTEST runPubSubTypeTests<int64_t, vc> (56, vc{c{56}});
}

BOOST_AUTO_TEST_SUITE_END ()
