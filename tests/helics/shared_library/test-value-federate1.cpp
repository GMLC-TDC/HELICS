/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
#include <iostream>

#include "ctestFixtures.hpp"

/** these test cases test out the value federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_tests1, FederateTestFixture, *utf::label("ci"))

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);
    BOOST_REQUIRE (vFed1 != nullptr);
    CE (helicsFederateEnterExecutingMode (vFed1));

    federate_state state;
    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_execution_state);

    CE (helicsFederateFinalize (vFed1));

    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", HELICS_DATA_TYPE_STRING, "");
    auto pubid2 = helicsFederateRegisterGlobalTypePublication (vFed1, "pub2", "int", "");

    auto pubid3 = helicsFederateRegisterPublication (vFed1, "pub3", HELICS_DATA_TYPE_DOUBLE, "V");
    CE (helicsFederateEnterExecutingMode (vFed1));

    federate_state state;
    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid, sv, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    char sv2[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    char tmp[HELICS_SIZE_MAX];
    CE (helicsPublicationGetType (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "double");
    CE (helicsPublicationGetUnits (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "V");

    // BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
    // BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
    // BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);
    CE (helicsFederateFinalize (vFed1));

    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_publisher_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", HELICS_DATA_TYPE_STRING, "");
    auto pubid2 = helicsFederateRegisterGlobalPublication (vFed1, "pub2", HELICS_DATA_TYPE_INT, "");
    auto pubid3 = helicsFederateRegisterPublication (vFed1, "pub3", HELICS_DATA_TYPE_DOUBLE, "V");
    CE (helicsFederateEnterExecutingMode (vFed1));

    federate_state state;
    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    char tmp[HELICS_SIZE_MAX];
    CE (helicsPublicationGetType (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "double");
    CE (helicsPublicationGetUnits (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "V");

    // BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid.getID ());
    // BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    // BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    CE (helicsFederateFinalize (vFed1));

    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto subid = helicsFederateRegisterTypeSubscription (vFed1, "sub1", "double", "V");
    auto subid2 = helicsFederateRegisterSubscription (vFed1, "sub2", "");

    auto subid3 = helicsFederateRegisterTypeSubscription (vFed1, "sub3", "double", "V");
    CE (helicsFederateEnterExecutingMode (vFed1));

    federate_state state;
    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    char sub3name[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid3, sub3name, HELICS_SIZE_MAX));

    // vFed1->addSubscriptionShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    char tmp[HELICS_SIZE_MAX];
    CE (helicsInputGetType (subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "double");
    CE (helicsInputGetUnits (subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "V");

    // BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    // BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    // BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid);

    CE (helicsFederateFinalize (vFed1));

    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    // register the publications
    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", HELICS_DATA_TYPE_STRING, "");
    auto pubid2 = helicsFederateRegisterGlobalPublication (vFed1, "pub2", HELICS_DATA_TYPE_INT, "");

    auto pubid3 = helicsFederateRegisterTypePublication (vFed1, "pub3", "double", "V");

    auto subid = helicsFederateRegisterTypeSubscription (vFed1, "sub1", "double", "V");
    auto subid2 = helicsFederateRegisterSubscription (vFed1, "sub2", "");

    auto subid3 = helicsFederateRegisterTypeSubscription (vFed1, "sub3", "double", "V");
    // enter execution
    CE (helicsFederateEnterExecutingMode (vFed1));

    federate_state state;
    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    char sub3name[HELICS_SIZE_MAX];
    CE (helicsInputGetKey (subid3, sub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    char tmp[HELICS_SIZE_MAX];
    CE (helicsInputGetType (subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "double");
    CE (helicsInputGetUnits (subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "V");

    // check publications

    CE (helicsPublicationGetKey (pubid, sv, HELICS_SIZE_MAX));
    CE (helicsPublicationGetKey (pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE (helicsPublicationGetKey (pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    CE (helicsPublicationGetType (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "double");
    CE (helicsPublicationGetUnits (pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL (tmp, "V");
    CE (helicsFederateFinalize (vFed1));

    CE (helicsFederateGetState (vFed1, &state));
    BOOST_CHECK (state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types), core_type)
{
    // helics_time_t stime = 1.0;
    helics_time_t gtime;
#define STRINGLEN 100
    char s[STRINGLEN] = "n2";
    int len;

    SetupTest (helicsCreateValueFederate, core_type, 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_STRING, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "string", nullptr);

    CE (helicsFederateEnterExecutingMode (vFed));

    CE (helicsPublicationPublishString (pubid, "string1"));

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, s, STRINGLEN, &len));
    
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    //check the time
    auto time = helicsInputLastUpdateTime(subid);
    BOOST_CHECK_EQUAL(time, 1.0);
    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2"));

    int actualLen;
    // make sure the value is still what we expect
    CE (helicsInputGetRawValue (subid, s, STRINGLEN, &actualLen));
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string1");

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));

    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the string is what we expect
    CE (helicsInputGetRawValue (subid, s, STRINGLEN, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, 7);
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string2");

    CE (helicsFederateFinalize (vFed));
}

// template <class X>
void runFederateTestDouble (const char *core, double defaultValue, double testValue1, double testValue2)
{
    helics_time_t gtime;
    double val1 = 0;
    double *val = &val1;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_DOUBLE, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "double", "");
    CE (helicsInputSetDefaultDouble (subid, defaultValue));

    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishDouble (pubid, testValue1));

    CE (helicsInputGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetDouble (subid, val));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE (helicsPublicationPublishDouble (pubid, testValue2));

    // make sure the value is still what we expect
    CE (helicsInputGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE (helicsFederateFinalize (vFed));
}

void runFederateTestComplex (const char *core,
                             double defaultValue_r,
                             double defaultValue_i,
                             double testValue1_r,
                             double testValue1_i,
                             double testValue2_r,
                             double testValue2_i)
{
    helics_time_t gtime;
    double val1_r = 0.0;
    double val1_i = 0.0;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_COMPLEX, "");
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "");
    CE (helicsInputSetDefaultComplex (subid, defaultValue_r, defaultValue_i));

    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishComplex (pubid, testValue1_r, testValue1_i));

    CE (helicsInputGetComplex (subid, &val1_r, &val1_i));
    BOOST_CHECK_EQUAL (val1_r, defaultValue_r);
    BOOST_CHECK_EQUAL (val1_i, defaultValue_i);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetComplex (subid, &val1_r, &val1_i));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val1_r, testValue1_r);
    BOOST_CHECK_EQUAL (val1_i, testValue1_i);

    // publish a second value
    CE (helicsPublicationPublishComplex (pubid, testValue2_r, testValue2_i));

    // make sure the value is still what we expect
    CE (helicsInputGetComplex (subid, &val1_r, &val1_i));
    BOOST_CHECK_EQUAL (val1_r, testValue1_r);
    BOOST_CHECK_EQUAL (val1_i, testValue1_i);
    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetComplex (subid, &val1_r, &val1_i));
    BOOST_CHECK_EQUAL (val1_r, testValue2_r);
    BOOST_CHECK_EQUAL (val1_i, testValue2_i);

    CE (helicsFederateFinalize (vFed));
}

void runFederateTestInteger (const char *core, int64_t defaultValue, int64_t testValue1, int64_t testValue2)
{
    helics_time_t gtime;
    int64_t val1 = 0;
    int64_t *val = &val1;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_INT, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "int", "");
    CE (helicsInputSetDefaultDouble (subid, defaultValue));
    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishInteger (pubid, testValue1));
    CE (helicsInputGetInteger (subid, val));

    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetInteger (subid, val));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE (helicsPublicationPublishInteger (pubid, testValue2));

    // make sure the value is still what we expect
    CE (helicsInputGetInteger (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetInteger (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE (helicsFederateFinalize (vFed));
}

void runFederateTestBool (const char *core, bool defaultValue, bool testValue1, bool testValue2)
{
    helics_time_t gtime;
    helics_bool_t val1 = 0;
    helics_bool_t *val = &val1;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_BOOLEAN, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "bool", "");
    CE (helicsInputSetDefaultDouble (subid, defaultValue ? helics_true : helics_false));
    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishBoolean (pubid, testValue1 ? helics_true : helics_false));
    CE (helicsInputGetBoolean (subid, val));

    BOOST_CHECK_EQUAL (*val, defaultValue ? helics_true : helics_false);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetBoolean (subid, val));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1 ? helics_true : helics_false);

    // publish a second string
    CE (helicsPublicationPublishBoolean (pubid, testValue2 ? helics_true : helics_false));

    // make sure the value is still what we expect
    CE (helicsInputGetBoolean (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue1 ? helics_true : helics_false);
    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetBoolean (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue2 ? helics_true : helics_false);

    CE (helicsFederateFinalize (vFed));
}

void runFederateTestString (const char *core,
                            const char *defaultValue,
                            const char *testValue1,
                            const char *testValue2)
{
    helics_time_t gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";
    int len = STRINGSIZE;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalTypePublication (vFed, "pub1", HELICS_DATA_TYPE_STRING, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "string", "");
    CE (helicsInputSetDefaultString (subid, defaultValue));

    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, testValue1));

    CE (helicsInputGetString (subid, str, STRINGSIZE, &len));

    BOOST_CHECK_EQUAL (str, defaultValue);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, str, STRINGSIZE, &len));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (str, testValue1);

    // publish a second string
    CE (helicsPublicationPublishString (pubid, testValue2));

    // make sure the value is still what we expect
    CE (helicsInputGetString (subid, str, STRINGSIZE, &len));
    BOOST_CHECK_EQUAL (str, testValue1);

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetString (subid, str, STRINGSIZE, &len));
    BOOST_CHECK_EQUAL (str, testValue2);

    CE (helicsFederateFinalize (vFed));
}

void runFederateTestVectorD (const char *core,
                             const double defaultValue[],
                             const double testValue1[],
                             const double testValue2[],
                             int len,
                             int len1,
                             int len2)
{
    helics_time_t gtime;
    int maxlen = (len1 > len2) ? len1 : len2;
    maxlen = (maxlen > len) ? maxlen : len;
    double *val = new double[maxlen];

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the interfaces
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_VECTOR, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "vector", "");
    CE (helicsInputSetDefaultVector (subid, defaultValue, len));
    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishVector (pubid, testValue1, len1));

    int actualLen = helicsInputGetVectorSize (subid);
    BOOST_CHECK_EQUAL (actualLen, len);

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, len);
    for (int i = 0; i < len; i++)
    {
        BOOST_CHECK_EQUAL (val[i], defaultValue[i]);
        // std::cout << defaultValue[i] << "\n";
    }
    

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen));
    BOOST_CHECK_EQUAL (actualLen, len1);
    // make sure the vector is what we expect
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        // std::cout << testValue1[i] << "\n";
    }

    //test getting a vector as a string
    actualLen = helicsInputGetStringSize(subid);
    std::string buf;
    buf.resize(actualLen + 2);
    CE(helicsInputGetString(subid, &(buf[0]), static_cast<int>(buf.size()), &actualLen));
    buf.resize(actualLen-1);
    BOOST_CHECK_EQUAL(buf[0], 'v');
    BOOST_CHECK_EQUAL(buf.back(), ']');

    // publish a second vector
    CE (helicsPublicationPublishVector (pubid, testValue2, len2));

    // make sure the value is still what we expect
    CE (helicsInputGetVector (subid, val, maxlen, &actualLen));
    BOOST_CHECK_EQUAL (actualLen, len1);
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        //  std::cout << testValue1[i] << "\n";
    }

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, len2);
    for (int i = 0; i < len2; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue2[i]);
        //  std::cout << testValue2[i] << "\n";
    }

    CE (helicsFederateFinalize (vFed));
    delete[] val;
}

void runFederateTestNamedPoint (const char *core,
                                const char *defaultValue,
                                double defVal,
                                const char *testValue1,
                                double testVal1,
                                const char *testValue2,
                                double testVal2)
{
    helics_time_t gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";
    int len = STRINGSIZE;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", HELICS_DATA_TYPE_NAMEDPOINT, "");
    auto subid = helicsFederateRegisterTypeSubscription (vFed, "pub1", "named_point", "");
    CE (helicsInputSetDefaultNamedPoint (subid, defaultValue, defVal));

    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishNamedPoint (pubid, testValue1, testVal1));

    double val;
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, &len, &val));

    BOOST_CHECK_EQUAL (std::string(str), std::string(defaultValue));
    BOOST_CHECK_EQUAL (val, defVal);
    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, &len, &val));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (std::string(str), std::string(testValue1));
    BOOST_CHECK_EQUAL (val, testVal1);

    // publish a second string
    CE (helicsPublicationPublishNamedPoint (pubid, testValue2, testVal2));

    // make sure the value is still what we expect
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, &len, &val));
    BOOST_CHECK_EQUAL (std::string(str), std::string(testValue1));
    BOOST_CHECK_EQUAL (val, testVal1);

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, &len, &val));
    BOOST_CHECK_EQUAL (std::string(str), std::string(testValue2));
    BOOST_CHECK_EQUAL (val, testVal2);

    CE (helicsFederateFinalize (vFed));
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_double1, bdata::make (core_types), core_type)
{
    runFederateTestDouble (core_type.c_str (), 10.3, 45.3, 22.7);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_double2, bdata::make (core_types), core_type)
{
    runFederateTestDouble (core_type.c_str (), 1.0, 0.0, 3.0);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_integer1, bdata::make (core_types), core_type)
{
    runFederateTestInteger (core_type.c_str (), 5, 8, 43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_integer2, bdata::make (core_types), core_type)
{
    runFederateTestInteger (core_type.c_str (), -5, 1241515, -43);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_complex, bdata::make (core_types), core_type)
{
    runFederateTestComplex (core_type.c_str (), 54.23233, 0.7, -9.7, 3.2, -3e45, 1e-23);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_string, bdata::make (core_types), core_type)
{
    runFederateTestString (core_type.c_str (), "start", "inside of the functional relationship of helics",
                           "I am a string");
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_named_point, bdata::make (core_types), core_type)
{
    runFederateTestNamedPoint (core_type.c_str (), "start", 5.3, "inside of the functional relationship of helics",
                               45.7823, "I am a string", 0.0);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_boolean, bdata::make (core_types), core_type)
{
    runFederateTestBool (core_type.c_str (), true, true, false);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_vector, bdata::make (core_types), core_type)
{
    const double val1[] = {34.3, 24.2};
    const double val2[] = {12.4, 14.7, 16.34, 18.17};
    const double val3[] = {9.9999, 8.8888, 7.7777};
    runFederateTestVectorD (core_type.c_str (), val1, val2, val3, 2, 4, 3);
}

BOOST_DATA_TEST_CASE(value_federate_single_transfer_vector2, bdata::make(core_types), core_type)
{
    std::vector<double> V1(34, 39.4491966662);
    std::vector<double> V2(100, 45.236262626221);
    std::vector<double> V3(452, -25.25263858741);
    runFederateTestVectorD(core_type.c_str(), V1.data(), V2.data(), V3.data(), 34, 100, 452);
}




BOOST_DATA_TEST_CASE (value_federate_subscriber_and_publisher_registration, bdata::make (core_types), core_type)
{
    helics_publication pubid, pubid2, pubid3;
    helics_input subid, subid2, subid3;
    char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4",
         pubunit3[100] = "n5";
    char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype[100] = "n4",
         subtype2[100] = "n5", subtype3[100] = "n6", subunit3[100] = "n7";

    SetupTest (helicsCreateValueFederate, core_type.c_str (), 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications
    pubid = helicsFederateRegisterTypePublication (vFed, "pub1", "", "");
    pubid2 = helicsFederateRegisterGlobalTypePublication (vFed, "pub2", "int", "");
    pubid3 = helicsFederateRegisterPublication (vFed, "pub3", HELICS_DATA_TYPE_DOUBLE, "V");

    // these aren't meant to match the publications
    subid = helicsFederateRegisterSubscription (vFed, "sub1", "");
    subid2 = helicsFederateRegisterTypeSubscription (vFed, "sub2", "int", "");
    subid3 = helicsFederateRegisterSubscription (vFed, "sub3", "V");

    // enter execution
    CE (helicsFederateEnterExecutingMode (vFed));

    // check subscriptions
    CE (helicsInputGetKey (subid, subname, 100));
    CE (helicsInputGetKey (subid2, subname2, 100));

    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");
    CE (helicsInputGetKey (subid3, subname3, 100));
    BOOST_CHECK_EQUAL (subname3, "sub3");

    CE (helicsInputGetType (subid, subtype, 100));
    BOOST_CHECK_EQUAL (subtype, "def");
    CE (helicsInputGetType (subid2, subtype2, 100));
    BOOST_CHECK_EQUAL (subtype2, "int64");
    CE (helicsInputGetType (subid3, subtype3, 100));
    BOOST_CHECK_EQUAL (subtype3, "def");
    CE (helicsInputGetUnits (subid3, subunit3, 100));
    BOOST_CHECK_EQUAL (subunit3, "V");

    // check publications
    helicsPublicationGetKey (pubid, pubname, 100);
    CE (helicsPublicationGetKey (pubid2, pubname2, 100));

    BOOST_CHECK_EQUAL (pubname, "fed0/pub1");
    BOOST_CHECK_EQUAL (pubname2, "pub2");
    CE (helicsPublicationGetKey (pubid3, pubname3, 100));
    BOOST_CHECK_EQUAL (pubname3, "fed0/pub3");

    CE (helicsPublicationGetType (pubid3, pubtype, 100));  // in this function the publication type is returned
                                                           // in the char * argument of the function. The
                                                           // return type is just to check that the function
                                                           // execution was successful
    BOOST_CHECK_EQUAL (pubtype, "double");
    CE (helicsPublicationGetUnits (pubid3, pubunit3, 100));
    BOOST_CHECK_EQUAL (pubunit3, "V");

    CE (helicsFederateFinalize (vFed));
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types), core_type)
{
    //	helics_time_t stime = 1.0;
    helics_time_t gtime;

    char s[STRINGLEN] = "n2";
    int len = 0;

    SetupTest (helicsCreateValueFederate, core_type.c_str (), 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications

    auto pubid = helicsFederateRegisterGlobalTypePublication (vFed, "pub1", "string", "");
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "");
    CE (helicsFederateEnterExecutingMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, "string1"));
    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, s, STRINGLEN, &len));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2"));

    // make sure the value is still what we expect
    CE (helicsInputGetString (subid, s, STRINGLEN, &len));
    BOOST_CHECK_EQUAL (s, "string1");
    BOOST_CHECK_EQUAL (len-1, strlen ("string1"));

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsInputGetString (subid, s, STRINGLEN, &len));
    BOOST_CHECK_EQUAL (s, "string2");

    CE (helicsFederateFinalize (vFed));
}

BOOST_AUTO_TEST_SUITE_END ()
