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

BOOST_FIXTURE_TEST_SUITE (value_federate_tests1, FederateTestFixture, *utf::label ("ci"))

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);
    BOOST_REQUIRE (vFed1 != nullptr);
    CE (helicsFederateEnterExecutingMode (vFed1, &err));

    helics_federate_state state;
    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_execution);

    CE (helicsFederateFinalize (vFed1, &err));

    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 = helicsFederateRegisterGlobalTypePublication (vFed1, "pub2", "int", "", &err);

    auto pubid3 = helicsFederateRegisterPublication (vFed1, "pub3", helics_data_type_double, "V", &err);
    CE (helicsFederateEnterExecutingMode (vFed1, &err));

    helics_federate_state state;
    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_execution);

    auto sv = helicsPublicationGetKey (pubid);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    auto sv2 = helicsPublicationGetKey (pubid2);
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = helicsPublicationGetKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    auto type = helicsPublicationGetType (pubid3);
    BOOST_CHECK_EQUAL (type, "double");
    const char *units = helicsPublicationGetUnits (pubid3);
    BOOST_CHECK_EQUAL (units, "V");

    // BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
    // BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
    // BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);
    CE (helicsFederateFinalize (vFed1, &err));

    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publisher_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 = helicsFederateRegisterGlobalPublication (vFed1, "pub2", helics_data_type_int, "", &err);
    auto pubid3 = helicsFederateRegisterPublication (vFed1, "pub3", helics_data_type_double, "V", &err);
    CE (helicsFederateEnterExecutingMode (vFed1, &err));

    helics_federate_state state;
    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_execution);

    auto sv = helicsPublicationGetKey (pubid);
    auto sv2 = helicsPublicationGetKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = helicsPublicationGetKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    auto type = helicsPublicationGetType (pubid3);
    BOOST_CHECK_EQUAL (type, "double");
    auto units = helicsPublicationGetUnits (pubid3);
    BOOST_CHECK_EQUAL (units, "V");

    // BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid.getID ());
    // BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    // BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    CE (helicsFederateFinalize (vFed1, &err));

    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_finalize);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types_single), core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    auto subid = helicsFederateRegisterSubscription (vFed1, "sub1", "V", &err);
    auto subid2 = helicsFederateRegisterSubscription (vFed1, "sub2", "", &err);

    auto subid3 = helicsFederateRegisterSubscription (vFed1, "sub3", "V", &err);
    CE (helicsFederateEnterExecutingMode (vFed1, &err));

    helics_federate_state state;
    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_execution);

    auto sv = helicsSubscriptionGetKey (subid);
    auto sv2 = helicsSubscriptionGetKey (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");

    auto sub3name = helicsSubscriptionGetKey (subid3);

    // vFed1->addSubscriptionShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    auto units = helicsInputGetUnits (subid3);
    BOOST_CHECK_EQUAL (units, "V");

    // BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    // BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    // BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid);

    CE (helicsFederateFinalize (vFed1, &err));

    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_finalize);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    SetupTest (helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt (0);

    // register the publications
    auto pubid = helicsFederateRegisterPublication (vFed1, "pub1", helics_data_type_string, "", &err);
    auto pubid2 = helicsFederateRegisterGlobalPublication (vFed1, "pub2", helics_data_type_int, "volts", &err);

    auto pubid3 = helicsFederateRegisterTypePublication (vFed1, "pub3", "double", "V", &err);

    auto subid = helicsFederateRegisterSubscription (vFed1, "sub1", "V", &err);
    auto subid2 = helicsFederateRegisterSubscription (vFed1, "sub2", "", &err);

    auto subid3 = helicsFederateRegisterSubscription (vFed1, "sub3", "V", &err);
    // enter execution
    CE (helicsFederateEnterExecutingMode (vFed1, &err));

    helics_federate_state state;
    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_execution);

    auto sv = helicsSubscriptionGetKey (subid);
    auto sv2 = helicsSubscriptionGetKey (subid2);

    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = helicsSubscriptionGetKey (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    auto units = helicsInputGetUnits (subid3);
    BOOST_CHECK_EQUAL (units, "V");

    // check the getSubscription function
    auto subid_b = helicsFederateGetSubscription (vFed1, "sub1", &err);
    const char *tmp = helicsSubscriptionGetKey (subid_b);
    BOOST_CHECK_EQUAL (tmp, "sub1");
    // check the getSubscriptionByIndex function
    auto subid_c = helicsFederateGetInputByIndex (vFed1, 2, &err);
    tmp = helicsInputGetUnits (subid_c);
    BOOST_CHECK_EQUAL (tmp, "V");
    // check publications

    sv = helicsPublicationGetKey (pubid);
    sv2 = helicsPublicationGetKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = helicsPublicationGetKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    const char *type = helicsPublicationGetType (pubid3);
    BOOST_CHECK_EQUAL (type, "double");
    units = helicsPublicationGetUnits (pubid3);
    BOOST_CHECK_EQUAL (units, "V");

    // check the getSubscription function

    auto pubid_b = helicsFederateGetPublication (vFed1, "fed0/pub1", &err);
    tmp = helicsPublicationGetType (pubid_b);
    BOOST_CHECK_EQUAL (tmp, "string");
    // check the getSubscriptionByIndex function
    auto pubid_c = helicsFederateGetPublicationByIndex (vFed1, 1, &err);
    tmp = helicsPublicationGetUnits (pubid_c);
    BOOST_CHECK_EQUAL (tmp, "volts");
    CE (helicsFederateFinalize (vFed1, &err));

    CE (state = helicsFederateGetState (vFed1, &err));
    BOOST_CHECK (state == helics_state_finalize);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types), core_type)
{
    // helics_time stime = 1.0;
    helics_time gtime;
#define STRINGLEN 100
    char s[STRINGLEN] = "n2";

    SetupTest (helicsCreateValueFederate, core_type, 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_string, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", nullptr, &err);

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    CE (helicsPublicationPublishString (pubid, "string1", &err));

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, s, STRINGLEN, nullptr, &err));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // check the time
    auto time = helicsInputLastUpdateTime (subid);
    BOOST_CHECK_EQUAL (time, 1.0);
    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2", &err));

    int actualLen;
    // make sure the value is still what we expect
    CE (helicsInputGetRawValue (subid, s, STRINGLEN, &actualLen, &err));
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string1");

    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));

    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the string is what we expect
    CE (helicsInputGetRawValue (subid, s, STRINGLEN, &actualLen, &err));

    BOOST_CHECK_EQUAL (actualLen, 7);
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string2");

    CE (helicsFederateFinalize (vFed, &err));
}

// template <class X>
void runFederateTestDouble (const char *core, double defaultValue, double testValue1, double testValue2)
{
    helics_time gtime;
    double val1 = 0;
    double *val = &val1;
    helics_error err = helicsErrorInitialize ();
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_double, "", nullptr);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", nullptr);
    CE (helicsInputSetDefaultDouble (subid, defaultValue, &err));

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishDouble (pubid, testValue1, &err));

    CE (*val = helicsInputGetDouble (subid, &err));
    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (*val = helicsInputGetDouble (subid, &err));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE (helicsPublicationPublishDouble (pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE (*val = helicsInputGetDouble (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (*val = helicsInputGetDouble (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE (helicsFederateFinalize (vFed, &err));
}

void runFederateTestComplex (const char *core,
                             double defaultValue_r,
                             double defaultValue_i,
                             double testValue1_r,
                             double testValue1_i,
                             double testValue2_r,
                             double testValue2_i)
{
    helics_time gtime;
    double val1_r = 0.0;
    double val1_i = 0.0;
    helics_error err = helicsErrorInitialize ();
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_complex, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsInputSetDefaultComplex (subid, defaultValue_r, defaultValue_i, &err));

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishComplex (pubid, testValue1_r, testValue1_i, &err));

    CE (helicsInputGetComplex (subid, &val1_r, &val1_i, &err));
    BOOST_CHECK_EQUAL (val1_r, defaultValue_r);
    BOOST_CHECK_EQUAL (val1_i, defaultValue_i);

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helics_complex hc = helicsInputGetComplexObject (subid, &err));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (hc.real, testValue1_r);
    BOOST_CHECK_EQUAL (hc.imag, testValue1_i);

    // publish a second value
    CE (helicsPublicationPublishComplex (pubid, testValue2_r, testValue2_i, &err));

    // make sure the value is still what we expect
    CE (helicsInputGetComplex (subid, &val1_r, &val1_i, &err));
    BOOST_CHECK_EQUAL (val1_r, testValue1_r);
    BOOST_CHECK_EQUAL (val1_i, testValue1_i);
    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (hc = helicsInputGetComplexObject (subid, &err));
    BOOST_CHECK_EQUAL (hc.real, testValue2_r);
    BOOST_CHECK_EQUAL (hc.imag, testValue2_i);

    CE (helicsFederateFinalize (vFed, &err));
}

void runFederateTestInteger (const char *core, int64_t defaultValue, int64_t testValue1, int64_t testValue2)
{
    helics_time gtime;
    int64_t val1 = 0;
    int64_t *val = &val1;

    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    helics_error err = helicsErrorInitialize ();
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_int, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsInputSetDefaultDouble (subid, defaultValue, &err));
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishInteger (pubid, testValue1, &err));
    CE (*val = helicsInputGetInteger (subid, &err));

    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (*val = helicsInputGetInteger (subid, &err));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE (helicsPublicationPublishInteger (pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE (*val = helicsInputGetInteger (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (*val = helicsInputGetInteger (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE (helicsFederateFinalize (vFed, &err));
}

void runFederateTestBool (const char *core, bool defaultValue, bool testValue1, bool testValue2)
{
    helics_time gtime;
    helics_bool val1 = 0;
    helics_bool *val = &val1;

    FederateTestFixture fixture;
    helics_error err = helicsErrorInitialize ();
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_boolean, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsInputSetDefaultDouble (subid, defaultValue ? helics_true : helics_false, &err));
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishBoolean (pubid, testValue1 ? helics_true : helics_false, &err));
    CE (*val = helicsInputGetBoolean (subid, &err));

    BOOST_CHECK_EQUAL (*val, defaultValue ? helics_true : helics_false);

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (*val = helicsInputGetBoolean (subid, &err));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1 ? helics_true : helics_false);

    // publish a second string
    CE (helicsPublicationPublishBoolean (pubid, testValue2 ? helics_true : helics_false, &err));

    // make sure the value is still what we expect
    CE (*val = helicsInputGetBoolean (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue1 ? helics_true : helics_false);
    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (*val = helicsInputGetBoolean (subid, &err));
    BOOST_CHECK_EQUAL (*val, testValue2 ? helics_true : helics_false);

    CE (helicsFederateFinalize (vFed, &err));
}

void runFederateTestString (const char *core,
                            const char *defaultValue,
                            const char *testValue1,
                            const char *testValue2)
{
    helics_time gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";
    helics_error err = helicsErrorInitialize ();
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_string, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsInputSetDefaultString (subid, defaultValue, &err));

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, testValue1, &err));

    CE (helicsInputGetString (subid, str, STRINGSIZE, nullptr, &err));

    BOOST_CHECK_EQUAL (str, defaultValue);

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, str, STRINGSIZE, nullptr, &err));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (str, testValue1);

    // publish a second string
    CE (helicsPublicationPublishString (pubid, testValue2, &err));

    // make sure the value is still what we expect
    CE (helicsInputGetString (subid, str, STRINGSIZE, nullptr, &err));
    BOOST_CHECK_EQUAL (str, testValue1);

    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetString (subid, str, STRINGSIZE, nullptr, &err));
    BOOST_CHECK_EQUAL (str, testValue2);

    CE (helicsFederateFinalize (vFed, &err));
}

void runFederateTestVectorD (const char *core,
                             const double defaultValue[],
                             const double testValue1[],
                             const double testValue2[],
                             int len,
                             int len1,
                             int len2)
{
    helics_time gtime;
    int maxlen = (len1 > len2) ? len1 : len2;
    maxlen = (maxlen > len) ? maxlen : len;
    double *val = new double[maxlen];
    helics_error err = helicsErrorInitialize ();
    FederateTestFixture fixture;
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the interfaces
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_vector, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", nullptr, &err);
    CE (helicsInputSetDefaultVector (subid, defaultValue, len, &err));
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishVector (pubid, testValue1, len1, &err));

    int actualLen = helicsInputGetVectorSize (subid);
    BOOST_CHECK_EQUAL (actualLen, len);

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen, &err));

    BOOST_CHECK_EQUAL (actualLen, len);
    for (int i = 0; i < len; i++)
    {
        BOOST_CHECK_EQUAL (val[i], defaultValue[i]);
        // std::cout << defaultValue[i] << "\n";
    }

    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen, &err));
    BOOST_CHECK_EQUAL (actualLen, len1);
    // make sure the vector is what we expect
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        // std::cout << testValue1[i] << "\n";
    }

    // test getting a vector as a string
    actualLen = helicsInputGetStringSize (subid);
    std::string buf;
    buf.resize (actualLen + 2);
    CE (helicsInputGetString (subid, &(buf[0]), static_cast<int> (buf.size ()), &actualLen, &err));
    buf.resize (actualLen - 1);
    BOOST_CHECK_EQUAL (buf[0], 'v');
    BOOST_CHECK_EQUAL (buf.back (), ']');

    // publish a second vector
    CE (helicsPublicationPublishVector (pubid, testValue2, len2, &err));

    // make sure the value is still what we expect
    CE (helicsInputGetVector (subid, val, maxlen, &actualLen, &err));
    BOOST_CHECK_EQUAL (actualLen, len1);
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_CLOSE (val[i], testValue1[i], 0.0001);
        //  std::cout << testValue1[i] << "\n";
    }

    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetVector (subid, val, maxlen, &actualLen, &err));

    BOOST_CHECK_EQUAL (actualLen, len2);
    for (int i = 0; i < len2; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue2[i]);
        //  std::cout << testValue2[i] << "\n";
    }

    CE (helicsFederateFinalize (vFed, &err));
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
    helics_time gtime;
#define STRINGSIZE 100
    char str[STRINGSIZE] = "";

    FederateTestFixture fixture;
    helics_error err = helicsErrorInitialize ();
    fixture.SetupTest (helicsCreateValueFederate, core, 1, 1.0);
    auto vFed = fixture.GetFederateAt (0);
    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", helics_data_type_named_point, "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsInputSetDefaultNamedPoint (subid, defaultValue, defVal, &err));

    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishNamedPoint (pubid, testValue1, testVal1, &err));

    double val;
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, nullptr, &val, &err));

    BOOST_CHECK_EQUAL (std::string (str), std::string (defaultValue));
    BOOST_CHECK_EQUAL (val, defVal);
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, nullptr, &val, &err));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (std::string (str), std::string (testValue1));
    BOOST_CHECK_EQUAL (val, testVal1);

    // publish a second string
    CE (helicsPublicationPublishNamedPoint (pubid, testValue2, testVal2, &err));

    // make sure the value is still what we expect
    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, nullptr, &val, &err));
    BOOST_CHECK_EQUAL (std::string (str), std::string (testValue1));
    BOOST_CHECK_EQUAL (val, testVal1);

    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsInputGetNamedPoint (subid, str, STRINGSIZE, nullptr, &val, &err));
    BOOST_CHECK_EQUAL (std::string (str), std::string (testValue2));
    BOOST_CHECK_EQUAL (val, testVal2);

    CE (helicsFederateFinalize (vFed, &err));
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

BOOST_DATA_TEST_CASE (value_federate_single_transfer_vector2, bdata::make (core_types), core_type)
{
    std::vector<double> V1 (34, 39.4491966662);
    std::vector<double> V2 (100, 45.236262626221);
    std::vector<double> V3 (452, -25.25263858741);
    runFederateTestVectorD (core_type.c_str (), V1.data (), V2.data (), V3.data (), 34, 100, 452);
}

BOOST_DATA_TEST_CASE (value_federate_subscriber_and_publisher_registration, bdata::make (core_types), core_type)
{
    helics_publication pubid, pubid2, pubid3;
    helics_input subid, subid2, subid3;
    const char *pubname, *pubname2, *pubname3, *pubtype, *pubunit3;
    const char *subname, *subname2, *subname3;
    const char *subunit3;

    SetupTest (helicsCreateValueFederate, core_type.c_str (), 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications
    pubid = helicsFederateRegisterTypePublication (vFed, "pub1", "", "", &err);
    pubid2 = helicsFederateRegisterGlobalTypePublication (vFed, "pub2", "int", "", &err);
    pubid3 = helicsFederateRegisterPublication (vFed, "pub3", helics_data_type_double, "V", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    // these aren't meant to match the publications
    subid = helicsFederateRegisterSubscription (vFed, "sub1", "", &err);
    subid2 = helicsFederateRegisterSubscription (vFed, "sub2", "", &err);
    subid3 = helicsFederateRegisterSubscription (vFed, "sub3", "V", &err);
    BOOST_CHECK_EQUAL (err.error_code, helics_ok);
    // enter execution
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // check subscriptions
    subname = helicsSubscriptionGetKey (subid);
    subname2 = helicsSubscriptionGetKey (subid2);

    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");
    subname3 = helicsSubscriptionGetKey (subid3);
    BOOST_CHECK_EQUAL (subname3, "sub3");

    // subtype=helicsInputGetType (subid);
    // BOOST_CHECK_EQUAL (subtype, "def");
    // subtype2=helicsInputGetType (subid2);
    // BOOST_CHECK_EQUAL (subtype2, "int64");
    // subtype3=helicsInputGetType (subid3);
    // BOOST_CHECK_EQUAL (subtype3, "def");
    subunit3 = helicsInputGetUnits (subid3);
    BOOST_CHECK_EQUAL (subunit3, "V");

    // check publications
    pubname = helicsPublicationGetKey (pubid);
    pubname2 = helicsPublicationGetKey (pubid2);

    BOOST_CHECK_EQUAL (pubname, "fed0/pub1");
    BOOST_CHECK_EQUAL (pubname2, "pub2");
    pubname3 = helicsPublicationGetKey (pubid3);
    BOOST_CHECK_EQUAL (pubname3, "fed0/pub3");

    pubtype = helicsPublicationGetType (pubid3);
    BOOST_CHECK_EQUAL (pubtype, "double");
    pubunit3 = helicsPublicationGetUnits (pubid3);
    BOOST_CHECK_EQUAL (pubunit3, "V");

    CE (helicsFederateFinalize (vFed, &err));
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types), core_type)
{
    //	helics_time stime = 1.0;
    helics_time gtime;

    char s[STRINGLEN] = "n2";
    int len = 0;
    SetupTest (helicsCreateValueFederate, core_type.c_str (), 1, 1.0);
    auto vFed = GetFederateAt (0);

    // register the publications

    auto pubid = helicsFederateRegisterGlobalTypePublication (vFed, "pub1", "string", "", &err);
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "", &err);
    CE (helicsFederateEnterExecutingMode (vFed, &err));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, "string1", &err));
    CE (gtime = helicsFederateRequestTime (vFed, 1.0, &err));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsInputGetString (subid, s, STRINGLEN, &len, &err));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2", &err));

    // make sure the value is still what we expect
    CE (helicsInputGetString (subid, s, STRINGLEN, &len, &err));
    BOOST_CHECK_EQUAL (s, "string1");
    BOOST_CHECK_EQUAL (len - 1, strlen ("string1"));

    // advance time
    CE (gtime = helicsFederateRequestTime (vFed, 2.0, &err));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsInputGetString (subid, s, STRINGLEN, &len, &err));
    BOOST_CHECK_EQUAL (s, "string2");

    CE (helicsFederateFinalize (vFed, &err));
}

/** test info field for multiple publications */
BOOST_DATA_TEST_CASE (test_info_field, bdata::make(core_types_single), core_type) {
SetupTest (helicsCreateValueFederate, core_type.c_str (), 1, 1.0);
auto vFed = GetFederateAt (0);

// register the publications/subscriptions

auto subid1 = helicsFederateRegisterSubscription (vFed, "sub1", "", &err);
auto pubid1 = helicsFederateRegisterTypePublication(vFed, "pub1", "string", "", &err);
auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed, "pub2", "string", "", &err);

// Set info fields
CE (helicsInputSetInfo(subid1, "sub1_test", &err));
CE (helicsPublicationSetInfo(pubid1, "pub1_test", &err));
CE (helicsPublicationSetInfo(pubid2, "pub2_test", &err));
CE (helicsFederateEnterExecutingMode (vFed, &err));


BOOST_CHECK_EQUAL (helicsInputGetInfo(subid1), "sub1_test");
BOOST_CHECK_EQUAL (helicsPublicationGetInfo(pubid1), "pub1_test");
BOOST_CHECK_EQUAL (helicsPublicationGetInfo(pubid2), "pub2_test");

CE (helicsFederateFinalize (vFed, &err));
}

BOOST_AUTO_TEST_SUITE_END ()
