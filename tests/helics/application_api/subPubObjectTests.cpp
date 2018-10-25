/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>
/** these test cases test out the value converters
 */
#include <boost/test/floating_point_comparison.hpp>

#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (subPubObject_tests)

#define CORE_TYPE_TO_TEST helics::core_type::TEST

BOOST_AUTO_TEST_CASE (subscriptionTObject_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::PublicationT<std::string> (helics::GLOBAL, vFed.get (), "pub1");

    auto subObj = helics::make_subscription<std::string> (*vFed, "pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubObj.publish ("string1");
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s = subObj.getValue();

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

BOOST_AUTO_TEST_CASE (subscriptionObject_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);

    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::make_publication<std::string> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj = vFed->registerSubscription( "pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
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
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::make_publication<TX> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj = vFed->registerSubscription( "pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubObj->publish (valtx);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    auto s = subObj.getValue<RX> ();
    // int64_t val = subObj.getValue<int64_t>();
    // make sure the object is what we expect
    BOOST_CHECK_MESSAGE (s == valrx, std::string (typeid (TX).name ()) + " to " + typeid (RX).name ());
    vFed->finalize ();
}

template <class IX, class TX, class RX>
void runPubSubThroughTypeTests (const TX &valtx, const RX &valrx)
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::make_publication<IX> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj = vFed->registerSubscription("pub1");
    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubObj->publish (valtx);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    auto s = subObj.getValue<RX> ();
    // int64_t val = subObj.getValue<int64_t>();
    // make sure the object is what we expect
    BOOST_CHECK_MESSAGE (s == valrx, std::string (typeid (TX).name ()) + " -> " + typeid (IX).name () + " -> " +
                                       typeid (RX).name ());
    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (subscriptionObject_type_tests, *utf::label("ci"))
{
    runPubSubTypeTests<std::string, double> ("3.14159", 3.14159);
    runPubSubTypeTests<double, std::string> (3.14159, std::to_string (3.141590));
}

BOOST_AUTO_TEST_CASE(subscriptionObject_type_tests_ext)
{
     runPubSubTypeTests<int64_t, double>(34, 34.0);
     runPubSubTypeTests<int64_t, std::string>(34, "34");
     runPubSubTypeTests<std::string, int64_t>("34.14", 34);
     runPubSubTypeTests<helics::named_point, double>({ std::string(), -3.14159 }, -3.14159);
     runPubSubTypeTests<helics::named_point, int64_t>({ std::string(), -3.14159 }, -3);
}

BOOST_AUTO_TEST_CASE (subscriptionObject_bool_tests, *utf::label("ci"))
{
    runPubSubTypeTests<bool, int64_t> (true, 1);
    runPubSubTypeTests<bool, std::string> (true, "1");
    runPubSubTypeTests<double, bool> (47.9, true);
    runPubSubTypeTests<std::string, bool> ("0", false);
}

BOOST_AUTO_TEST_CASE(subscriptionObject_bool_tests_ext)
{
     runPubSubTypeTests<bool, double>(false, 0.0);
     runPubSubTypeTests<int64_t, bool>(-10, true);
     runPubSubTypeTests<int64_t, bool>(0, false);
     runPubSubTypeTests<helics::named_point, bool>({ std::string(), -3.14159 }, true);
     runPubSubTypeTests<helics::named_point, bool>({ "0", std::nan("0") }, false);
}

BOOST_AUTO_TEST_CASE (subscriptionObject_complex_tests, *utf::label("ci"))
{
    using c = std::complex<double>;

    runPubSubTypeTests<c, std::string> (c (12.4, 0.3), helics::helicsComplexString (c (12.4, 0.3)));
    runPubSubTypeTests<std::string, c> ("-3.14159 - 2i", c (-3.14159, -2));
    runPubSubTypeTests<helics::named_point, c> ({"-3.14159 - 2i", std::nan ("0")}, c (-3.14159, -2));
    runPubSubTypeTests<helics::named_point, c> ({"", -3.14159}, c (-3.14159, 0));
    runPubSubTypeTests<c, double> (c (0, 2), 2.0);
}

BOOST_AUTO_TEST_CASE(subscriptionObject_complex_tests_ext)
{
    using c = std::complex<double>;

     runPubSubTypeTests<std::string, c>("3.14159+2j", c(3.14159, 2));
     runPubSubTypeTests<std::string, c>("3.14159-2j", c(3.14159, -2));
     runPubSubTypeTests<std::string, c>("-3.14159-2j", c(-3.14159, -2));

     runPubSubTypeTests<c, helics::named_point>(c(-3.14159, -2), { "-3.14159 -2j", std::nan("0") });
     runPubSubTypeTests<c, helics::named_point>(c(-3.14159, 0), { "value", -3.14159 });
     runPubSubTypeTests<std::string, c>("-3.14159 + 2i", c(-3.14159, 2));

     runPubSubTypeTests<std::string, c>("2i", c(0, 2));
     runPubSubTypeTests<c, int64_t>(c(0, 2), 2);

     runPubSubTypeTests<double, c>(2.0, c(2, 0));

     runPubSubTypeTests<int64_t, c>(2, c(2, 0));
     runPubSubTypeTests<c, double>(c(3.0, 4.0), 5.0);
     runPubSubTypeTests<c, int64_t>(c(3.0, 4.0), 5);
}

BOOST_AUTO_TEST_CASE (subscriptionObject_vector_tests, *utf::label("ci"))
{
    using v = std::vector<double>;
    v tvec1{12.4, 0.3, 0.7};
    v tvec2{0.0, -1241.23, 5.0, 7.9};
    v eVec{};
    runPubSubTypeTests<v, std::string> (tvec1, helics::helicsVectorString (tvec1));
    runPubSubTypeTests<std::string, v> (helics::helicsVectorString (tvec1), tvec1);

    runPubSubTypeTests<v, std::string> (tvec2, helics::helicsVectorString (tvec2));
    runPubSubTypeTests<std::string, v> (helics::helicsVectorString (tvec2), tvec2);
}

BOOST_AUTO_TEST_CASE(subscriptionObject_vector_tests_ext)
{
    using v = std::vector<double>;
    using c = std::complex<double>;
    v tvec1{ 12.4, 0.3, 0.7 };
    v tvec2{ 0.0, -1241.23, 5.0, 7.9 };
    v eVec{};

     runPubSubTypeTests<v, std::string>(eVec, helics::helicsVectorString(eVec));
     runPubSubTypeTests<std::string, v>(helics::helicsVectorString(eVec), eVec);

     runPubSubTypeTests<std::string, v>("3.14159-2j", v{ 3.14159, -2 });
     runPubSubTypeTests<std::string, v>("-3.14159-2j", v{ -3.14159, -2 });
     runPubSubTypeTests<std::string, v>("-3.14159", v{ -3.14159 });

     runPubSubTypeTests<std::string, v>("2i", v{ 0, 2 });

     runPubSubTypeTests<c, v>(c{ 3.14159, -2 }, v{ 3.14159, -2 });
     runPubSubTypeTests<c, v>(c{ -3.14159, -2 }, v{ -3.14159, -2 });
     runPubSubTypeTests<c, v>(c{ -3.14159, 0.0 }, v{ -3.14159, 0.0 });

     runPubSubTypeTests<c, v>(c{ 0.0, 2 }, v{ 0, 2 });

     runPubSubTypeTests<v, double>(tvec1, sqrt(12.4 * 12.4 + 0.3 * 0.3 + 0.7 * 0.7));

     runPubSubTypeTests<double, v>(0.34, v{ 0.34 });

     runPubSubTypeTests<v, int64_t>(tvec1, 12);

     runPubSubTypeTests<int64_t, v>(56, v{ 56 });
}

BOOST_AUTO_TEST_CASE (subscriptionObject_complex_vector_tests, *utf::label("ci"))
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

    runPubSubTypeTests<vc, helics::named_point> (tcvec2,
                                                 {helics::helicsComplexVectorString (tcvec2), std::nan ("0")});

}

BOOST_AUTO_TEST_CASE(subscriptionObject_complex_vector_tests_ext)
{
    using v = std::vector<double>;
    using c = std::complex<double>;
    using vc = std::vector<std::complex<double>>;

    v tvec2{ 0.0, -1241.23, 5.0, 7.9 };
    vc eVec{};

    vc tcvec1{ c{ -4.5, 27.4 }, c{ 0.12, 0.34 } };

    vc tcvec2{ c{ -3.0, -4.0 }, c{ 23.7, 0.0 }, c{ 0.01, 45.23 } };

     runPubSubTypeTests<std::string, vc>("3.14159-2j", vc{ c{ 3.14159, -2 } });
     runPubSubTypeTests<std::string, vc>("-3.14159-2j", vc{ c{ -3.14159, -2 } });
     runPubSubTypeTests<std::string, vc>("-3.14159", vc{ c{ -3.14159 } });

     runPubSubTypeTests<std::string, vc>("2i", vc{ c{ 0, 2 } });

     runPubSubTypeTests<c, vc>(c{ 3.14159, -2 }, vc{ c{ 3.14159, -2 } });
     runPubSubTypeTests<c, vc>(c{ -3.14159, -2 }, vc{ c{ -3.14159, -2 } });
     runPubSubTypeTests<c, vc>(c{ -3.14159, 0.0 }, vc{ c{ -3.14159, 0.0 } });

     runPubSubTypeTests<c, vc>(c{ 0.0, 2 }, vc{ c{ 0, 2 } });

     runPubSubTypeTests<vc, double>(tcvec2, helics::vectorNorm(tcvec2));

     runPubSubTypeTests<double, vc>(0.34, vc{ c{ 0.34 } });

     runPubSubTypeTests<vc, int64_t>(tcvec2, 51);

     runPubSubTypeTests<int64_t, vc>(56, vc{ c{ 56 } });
}

BOOST_AUTO_TEST_CASE (subscriptionChangeDetection_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::make_publication<double> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj1 = vFed->registerSubscription("pub1");
    auto &subObj2 = vFed->registerSubscription ("pub1");
    subObj2.setMinimumChange (0.1);
    vFed->setTimeProperty (TIME_DELTA_PROPERTY,1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubObj->publish (23.7);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK (subObj1.isUpdated ());
    BOOST_CHECK (subObj2.isUpdated ());
    // check a second time
    BOOST_CHECK (subObj1.isUpdated ());
    BOOST_CHECK (subObj2.isUpdated ());
    auto val1 = subObj1.getValue<double> ();
    auto val2 = subObj2.getValue<double> ();
    // now that we got the value it should not be updated
    BOOST_CHECK (!subObj1.isUpdated ());
    BOOST_CHECK (!subObj2.isUpdated ());
    BOOST_CHECK_EQUAL (val1, val2);
    BOOST_CHECK_EQUAL (val1, 23.7);
    // publish a second string
    pubObj->publish (23.61);
    // advance time
    gtime = vFed->requestTime (2.0);

    BOOST_CHECK (subObj1.isUpdated ());
    BOOST_CHECK (!subObj2.isUpdated ());

    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (subscriptionstringSize_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);

    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj = helics::make_publication<std::string> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj = vFed->registerSubscription ("pub1");

    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    std::string str ("this is a string test");
    pubObj->publish (str);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK (subObj.isUpdated ());
    BOOST_CHECK_EQUAL (subObj.getStringSize (), str.size ());
    BOOST_CHECK_EQUAL (subObj.getRawSize (), str.size ());
    auto val1 = subObj.getValue<std::string> ();
    // now that we got the value it should not be updated
    BOOST_CHECK (!subObj.isUpdated ());
    BOOST_CHECK_EQUAL (val1, str);
    vFed->finalize ();
}

BOOST_AUTO_TEST_CASE (subscriptionVectorSize_tests, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1",fi);
    // register the publications
    auto pubObj =
      helics::make_publication<std::vector<double>> (helics::GLOBAL, vFed.get (), std::string ("pub1"));

    auto &subObj = vFed->registerSubscription ("pub1");

    vFed->setTimeProperty (TIME_DELTA_PROPERTY, 1.0);
    vFed->enterExecutingMode ();
    // publish string1 at time=0.0;
    std::vector<double> tvec{5, 7, 234.23, 99.1, 1e7, 0.0};
    pubObj->publish (tvec);
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK (subObj.isUpdated ());
    BOOST_CHECK_EQUAL (subObj.getVectorSize (), tvec.size ());

    auto val1 = subObj.getValue<std::vector<double>> ();
    // now that we got the value it should not be updated
    BOOST_CHECK (!subObj.isUpdated ());
    BOOST_CHECK (val1 == tvec);
    vFed->finalize ();
}


BOOST_AUTO_TEST_CASE(subscriptionDefaults_test, *utf::label("ci"))
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> ("test1", fi);
    // register the publications
    auto &subObj1=vFed->registerSubscription ("pub1");
    auto &subObj2 = vFed->registerSubscription ("pub2");
    subObj1.setDefault(45.3);
    subObj2.setDefault(67.4);
    vFed->enterExecutingMode();
    auto gtime = vFed->requestTime(1.0);

    BOOST_CHECK_EQUAL(gtime, 1.0);
    BOOST_CHECK(!subObj1.isUpdated());
    BOOST_CHECK(!subObj2.isUpdated());
    
    auto val1 = subObj1.getValue<double>();
    auto val2 = subObj2.getValue<double>();

    BOOST_CHECK_EQUAL(val1,45.3);
    BOOST_CHECK_EQUAL(val2, 67.4);
    
    // advance time
    gtime = vFed->requestTime(2.0);

    BOOST_CHECK(!subObj1.isUpdated());
    BOOST_CHECK(!subObj2.isUpdated());
    val1 = subObj1.getValue<double>();
    val2 = subObj2.getValue<double>();

    BOOST_CHECK_EQUAL(val1, 45.3);
    BOOST_CHECK_EQUAL(val2, 67.4);
    vFed->finalize();
}

BOOST_AUTO_TEST_SUITE_END ()
