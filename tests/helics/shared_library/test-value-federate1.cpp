/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>
#include <iostream>

#include "ctestFixtures.hpp"

/** these test cases test out the value federates
 */

BOOST_FIXTURE_TEST_SUITE(value_federate_tests1, FederateTestFixture)

namespace bdata = boost::unit_test::data;

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE(value_federate_initialize_tests, bdata::make(core_types_single), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt(0);

    CE(helicsFederateEnterExecutionMode(vFed1));

    federate_state state;
    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_execution_state);

    CE(helicsFederateFinalize(vFed1));

    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE(value_federate_publication_registration, bdata::make(core_types_single), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt(0);

    auto pubid = helicsFederateRegisterPublication(vFed1, "pub1", "string", "");
    auto pubid2 = helicsFederateRegisterGlobalPublication(vFed1, "pub2", "int", "");

    auto pubid3 = helicsFederateRegisterPublication(vFed1, "pub3", "double", "V");
    CE(helicsFederateEnterExecutionMode(vFed1));

    federate_state state;
    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid, sv, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    char sv2[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    char tmp[HELICS_SIZE_MAX];
    CE(helicsPublicationGetType(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "double");
    CE(helicsPublicationGetUnits(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "V");

    //BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
    //BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
    //BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);
    CE(helicsFederateFinalize(vFed1));

    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE(value_federate_publisher_registration, bdata::make(core_types_single), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt(0);

    auto pubid = helicsFederateRegisterTypePublication(vFed1, "pub1", HELICS_STRING_TYPE, "");
    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed1, "pub2", HELICS_INT_TYPE, "");
    auto pubid3 = helicsFederateRegisterTypePublication(vFed1, "pub3", HELICS_DOUBLE_TYPE, "V");
    CE(helicsFederateEnterExecutionMode(vFed1));

    federate_state state;
    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    BOOST_CHECK_EQUAL(sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    char tmp[HELICS_SIZE_MAX];
    CE(helicsPublicationGetType(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "double");
    CE(helicsPublicationGetUnits(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "V");

    //BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid.getID ());
    //BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    //BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    CE(helicsFederateFinalize(vFed1));

    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE(value_federate_subscription_registration, bdata::make(core_types_single), core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt(0);

    auto subid = helicsFederateRegisterSubscription(vFed1, "sub1", "double", "V");
    auto subid2 = helicsFederateRegisterTypeSubscription(vFed1, "sub2", HELICS_INT_TYPE, "");

    auto subid3 = helicsFederateRegisterOptionalSubscription(vFed1, "sub3", "double", "V");
    CE(helicsFederateEnterExecutionMode(vFed1));

    federate_state state;
    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv, "sub1");
    BOOST_CHECK_EQUAL(sv2, "sub2");
    char sub3name[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid3, sub3name, HELICS_SIZE_MAX));

    //vFed1->addSubscriptionShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL(sub3name, "sub3");

    char tmp[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetType(subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "double");
    CE(helicsSubscriptionGetUnits(subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "V");

    //BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    //BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    //BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid);

    CE(helicsFederateFinalize(vFed1));

    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE(value_federate_subscription_and_publication_registration,
    bdata::make(core_types_single),
    core_type)
{
    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed1 = GetFederateAt(0);

    // register the publications
    auto pubid = helicsFederateRegisterTypePublication(vFed1, "pub1", HELICS_STRING_TYPE, "");
    auto pubid2 = helicsFederateRegisterGlobalTypePublication(vFed1, "pub2", HELICS_INT_TYPE, "");

    auto pubid3 = helicsFederateRegisterPublication(vFed1, "pub3", "double", "V");

    auto subid = helicsFederateRegisterOptionalSubscription(vFed1, "sub1", "double", "V");
    auto subid2 = helicsFederateRegisterOptionalTypeSubscription(vFed1, "sub2", HELICS_INT_TYPE, "");

    auto subid3 = helicsFederateRegisterOptionalSubscription(vFed1, "sub3", "double", "V");
    // enter execution
    CE(helicsFederateEnterExecutionMode(vFed1));

    federate_state state;
    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_execution_state);

    char sv[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid, sv, HELICS_SIZE_MAX));
    char sv2[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv, "sub1");
    BOOST_CHECK_EQUAL(sv2, "sub2");
    char sub3name[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetKey(subid3, sub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sub3name, "sub3");

    char tmp[HELICS_SIZE_MAX];
    CE(helicsSubscriptionGetType(subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "double");
    CE(helicsSubscriptionGetUnits(subid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "V");

    // check publications

    CE(helicsPublicationGetKey(pubid, sv, HELICS_SIZE_MAX));
    CE(helicsPublicationGetKey(pubid2, sv2, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    BOOST_CHECK_EQUAL(sv2, "pub2");
    char pub3name[HELICS_SIZE_MAX];
    CE(helicsPublicationGetKey(pubid3, pub3name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    CE(helicsPublicationGetType(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "double");
    CE(helicsPublicationGetUnits(pubid3, tmp, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(tmp, "V");
    CE(helicsFederateFinalize(vFed1));

    CE(helicsFederateGetState(vFed1, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types), core_type)
{

    // helics_time_t stime = 1.0;
    helics_time_t gtime;
    char s[100] = "n2";

    SetupTest(helicsCreateValueFederate, core_type, 1);
    auto vFed = GetFederateAt(0);

    // register the publications
    auto pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    auto subid = helicsFederateRegisterSubscription (vFed, "pub1", "string", nullptr);

    CE(helicsFederateEnterExecutionMode (vFed));

    CE(helicsPublicationPublishString (pubid, "string1"));

    CE(helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE( helicsSubscriptionGetString (subid, s, 100));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2"));

    int actualLen;
    // make sure the value is still what we expect
    CE(helicsSubscriptionGetValue (subid, s, 100, &actualLen));
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string1");

    // advance time
    CE (helicsFederateRequestTime (vFed, 2.0, &gtime));

    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the string is what we expect
    CE(helicsSubscriptionGetValue (subid, s, 100, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, 7);
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string2");

    CE (helicsFederateFinalize (vFed));
}

// template <class X>
void runFederateTestDouble (const char *core,
                            double defaultValue,
                            double testValue1,
                            double testValue2,
                            const char *datatype)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    helics_time_t gtime;
    double val1 = 0;
    double *val = &val1;

    broker = helicsCreateBroker (core, "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE( helicsFederateInfoSetFederateName (fi, "fed0"));

    CE(helicsFederateInfoSetCoreTypeFromString (fi, core));

    CE(helicsFederateInfoSetTimeDelta (fi, 1.0));
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "double", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
    CE(helicsSubscriptionSetDefaultDouble (subid, defaultValue));

    CE(helicsFederateEnterExecutionMode (vFed));

    // publish string1 at time=0.0;
    CE(helicsPublicationPublishDouble (pubid, testValue1));

    CE(helicsSubscriptionGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE(helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE(helicsSubscriptionGetDouble (subid, val));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE(helicsPublicationPublishDouble (pubid, testValue2));

    // make sure the value is still what we expect
    CE(helicsSubscriptionGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE(helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE(helicsSubscriptionGetDouble (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE(helicsFederateFinalize (vFed));

    std::cout << "value_federate_single_transfer_types - datatype:" << datatype << " core_type " << core << "\n";
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();
    // auto pubid = vFed->registerGlobalPublication<X> ("pub1");
    // auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    // vFed->setTimeDelta (1.0); // in the C-API this is currently done prior to creating the federate
    // vFed->setDefaultValue<X> (subid, defaultValue);
    // vFed->enterExecutionState ();

    // publish string1 at time=0.0;
    // vFed->publish<X> (pubid, testValue1);

    // X val;
    // vFed->getValue<X> (subid, val);
    // BOOST_CHECK_EQUAL (val, defaultValue);

    // auto gtime = vFed->requestTime (1.0);
    // BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    // vFed->getValue (subid, val);
    // make sure the string is what we expect
    // BOOST_CHECK_EQUAL (val, testValue1);
    // publish a second string
    // vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
    // vFed->getValue (subid, val);
    // BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    // gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    // BOOST_CHECK_EQUAL (gtime, 2.0);
    // vFed->getValue (subid, val);

    // BOOST_CHECK_EQUAL (val, testValue2);

    // vFed->finalize ();
}

void runFederateTestInteger (const char *core,
                             int defaultValue,
                             int testValue1,
                             int testValue2,
                             const char *datatype)
{

    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    helics_time_t gtime;
    int64_t val1 = 0;
    int64_t *val = &val1;

    broker = helicsCreateBroker (core, "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE (helicsFederateInfoSetFederateName (fi, "fed0"));
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core));
    CE (helicsFederateInfoSetTimeDelta (fi, 1.0));
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "double", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
    CE (helicsSubscriptionSetDefaultDouble (subid, defaultValue));
    CE (helicsFederateEnterExecutionMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishInteger (pubid, testValue1));
    CE (helicsSubscriptionGetInteger (subid, val));

    BOOST_CHECK_EQUAL (*val, defaultValue);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsSubscriptionGetInteger (subid, val));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    CE (helicsPublicationPublishInteger (pubid, testValue2));

    // make sure the value is still what we expect
    CE (helicsSubscriptionGetInteger (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue1);
    // advance time
    CE(helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsSubscriptionGetInteger (subid, val));
    BOOST_CHECK_EQUAL (*val, testValue2);

    CE (helicsFederateFinalize (vFed));

    std::cout << "value_federate_single_transfer_types - datatype:" << datatype << " core_type " << core << "\n";
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();
}

void runFederateTestString (const char *core,
                            const char *defaultValue,
                            const char *testValue1,
                            const char *testValue2,
                            const char *datatype)
{

    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    helics_time_t gtime;
    char str[100] = "";
    const int len = 100;

    broker = helicsCreateBroker (core, "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE (helicsFederateInfoSetFederateName (fi, "fed0"));
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core));
    CE (helicsFederateInfoSetTimeDelta (fi, 1.0));
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "string", "");
    CE (helicsSubscriptionSetDefaultString (subid, defaultValue));

    CE (helicsFederateEnterExecutionMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, testValue1));

    CE (helicsSubscriptionGetString (subid, str, len));

    BOOST_CHECK_EQUAL (str, defaultValue);

    CE (helicsFederateRequestTime (vFed, 1.0, &gtime));

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsSubscriptionGetString (subid, str, len));

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (str, testValue1);

    // publish a second string
    CE (helicsPublicationPublishString (pubid, testValue2));

    // make sure the value is still what we expect
    CE (helicsSubscriptionGetString (subid, str, len));
    BOOST_CHECK_EQUAL (str, testValue1);

    // advance time
    CE(helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsSubscriptionGetString (subid, str, len));
    BOOST_CHECK_EQUAL (str, testValue2);

    CE (helicsFederateFinalize (vFed));

    std::cout << "value_federate_single_transfer_types - datatype:" << datatype << " core_type " << core << "\n";
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();
}

void runFederateTestVectorD (const char *core,
                             const double defaultValue[],
                             const double testValue1[],
                             const double testValue2[],
                             int len,
                             int len1,
                             int len2,
                             const char *datatype)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    helics_time_t gtime;

    double val[100] = {0};

    broker = helicsCreateBroker (core, "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE (helicsFederateInfoSetFederateName (fi, "fed0"));
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core));
    CE (helicsFederateInfoSetTimeDelta (fi, 1.0));

    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "vector", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "vector", "");
    CE (helicsSubscriptionSetDefaultVector (subid, defaultValue, len));
    CE (helicsFederateEnterExecutionMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishVector (pubid, testValue1, len1));
    int actualLen;
    CE (helicsSubscriptionGetVector (subid, val, 100, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, len);
    for (int i = 0; i < len; i++)
    {
        BOOST_CHECK_EQUAL (val[i], defaultValue[i]);
        std::cout << defaultValue[i] << "\n";
    }

    CE(helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value

    CE (helicsSubscriptionGetVector (subid, val, 100, &actualLen));
    BOOST_CHECK_EQUAL (actualLen, len1);
    // make sure the string is what we expect
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        std::cout << testValue1[i] << "\n";
    }

    // publish a second string
    CE (helicsPublicationPublishVector (pubid, testValue2, len2));

    // make sure the value is still what we expect
    CE (helicsSubscriptionGetVector (subid, val, 100, &actualLen));
    BOOST_CHECK_EQUAL (actualLen, len1);
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        std::cout << testValue1[i] << "\n";
    }

    // advance time
    CE(helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    CE (helicsSubscriptionGetVector (subid, val, 100, &actualLen));

    BOOST_CHECK_EQUAL (actualLen, len2);
    for (int i = 0; i < len2; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue2[i]);
        std::cout << testValue2[i] << "\n";
    }

    CE (helicsFederateFinalize (vFed));

    std::cout << "value_federate_single_transfer_types - datatype:" << datatype << " core_type " << core << "\n";
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();
}

/*
template <class X>
void runFederateTestObj(const std::string &core_type_str,
    const X &defaultValue,
    const X &testValue1,
    const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate>(core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    helics::PublicationT<X> pubid(helics::GLOBAL,vFed.get(),"pub1");

    helics::SubscriptionT<X> subid(vFed.get(),"pub1");
    vFed->setTimeDelta(1.0);
    subid.setDefault(defaultValue);
    vFed->enterExecutionState();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);
    X val;
    subid.getValue(val);
    BOOST_CHECK_EQUAL(val, defaultValue);

    auto gtime = vFed->requestTime(1.0);
    BOOST_CHECK_EQUAL(gtime, 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL(val, testValue1);
    // publish a second string
    pubid.publish(testValue2);
    // make sure the value is still what we expect
    val = subid.getValue();
    BOOST_CHECK_EQUAL(val, testValue1);

    // advance time
    gtime = vFed->requestTime(2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL(gtime, 2.0);
    val = subid.getValue();
    BOOST_CHECK_EQUAL(val, testValue2);

    vFed->finalize();
}
template <class X>
void runFederateTestv2 (const std::string &core_type_str,
                        const X &defaultValue,
                        const X &testValue1,
                        const X &testValue2)
{
    FederateTestFixture fixture;
    fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);    auto vFed =
fixture.GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto pubid = vFed->registerGlobalPublication<X> ("pub1");

    auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);    vFed->enterExecutionState ();
    // publish string1 at time=0.0;    vFed->publish<X> (pubid, testValue1);

    X val;
    vFed->getValue<X> (subid, val);    BOOST_CHECK (val == defaultValue);
    auto gtime = vFed->requestTime (1.0);    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    vFed->getValue (subid, val);    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);
    // publish a second string
    vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed->getValue (subid, val);
    BOOST_CHECK (val == testValue1);    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed->getValue (subid, val);
    BOOST_CHECK (val == testValue2);
    vFed->finalize ();
}

*/

BOOST_DATA_TEST_CASE (value_federate_single_transfer_double1, bdata::make (core_types), core_type)
{
    char datatype[20] = "double";
    runFederateTestDouble (core_type.c_str (), 10.3, 45.3, 22.7, datatype);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_double2, bdata::make (core_types), core_type)
{
    char datatype[20] = "double";
    runFederateTestDouble (core_type.c_str (), 1.0, 0.0, 3.0, datatype);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_integer1, bdata::make (core_types), core_type)
{
    char datatype[20] = "integer";
    runFederateTestInteger (core_type.c_str (), 5, 8, 43, datatype);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_integer2, bdata::make (core_types), core_type)
{
    char datatype[20] = "integer";
    runFederateTestInteger (core_type.c_str (), -5, 1241515, -43, datatype);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_string, bdata::make (core_types), core_type)
{
    char datatype[20] = "string";
    runFederateTestString (core_type.c_str (), "start", "inside of the functional relationship of helics",
                           "I am a string", datatype);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_vector, bdata::make (core_types), core_type)
{
    char datatype[20] = "vector";
    const int len = 100;
    const double val1[len] = {34.3, 24.2};
    const double val2[len] = {12.4, 14.7, 16.34, 18.17};
    const double val3[len] = {9.9999, 8.8888, 7.7777};
    runFederateTestVectorD (core_type.c_str (), val1, val2, val3, 2, 4, 3, datatype);
}

/*
BOOST_DATA_TEST_CASE(value_federate_single_transfer, bdata::make(core_types), core_type)
{
    //runFederateTest<double> (core_type, 10.3, 45.3, 22.7);
    //runFederateTest<double> (core_type, 1.0, 0.0, 3.0);
    //runFederateTest<int> (core_type, 5, 8, 43);
    //runFederateTest<int> (core_type, -5, 1241515, -43);
    //runFederateTest<short> (core_type, -5, 23023, -43);
    //runFederateTest<uint64_t> (core_type, 234252315,0xFFF1'2345'7124'1412, 23521513412);
 //   runFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
 //   runFederateTest<std::string> (core_type, "start", "inside of the functional relationship of
helics",std::string ("I am a string"));
 //   runFederateTestv2<std::vector<double>> (core_type, {34.3, 24.2}, {12.4,14.7, 16.34, 18.17}, {9.9999,
8.8888, 7.7777});
 //   std::vector<std::string> sv1{"hip", "hop"};
 //   std::vector<std::string> sv2{"this is the first string\n", "this is the second string",
 //                                "this is the third\0"                                 " string"};
 //   std::vector<std::string> sv3{"string1", "String2", "string3", "string4", "string5", "string6", "string8"};
 //   runFederateTestv2 (core_type, sv1, sv2, sv3);
 //   std::complex<double> def = {54.23233, 0.7};    std::complex<double> v1 = std::polar (10.0, 0.43);
 //   std::complex<double> v2 = {-3e45, 1e-23};
 //   runFederateTest<std::complex<double>> (core_type, def, v1, v2);
}
*/
/*

BOOST_DATA_TEST_CASE(value_federate_single_transfer_types_publishers, bdata::make(core_types), core_type)
{
    runFederateTestObj<double>(core_type, 10.3, 45.3, 22.7);
    runFederateTestObj<double>(core_type, 1.0, 0.0, 3.0);
    runFederateTestObj<int>(core_type, 5, 8, 43);
    runFederateTestObj<int>(core_type, -5, 1241515, -43);
    runFederateTestObj<short>(core_type, -5, 23023, -43);
    runFederateTestObj<uint64_t>(core_type, 234252315, 0xFFF1'2345'7124'1412, 23521513412);
    runFederateTestObj<float>(core_type, 10.3f, 45.3f, 22.7f);
    runFederateTestObj<std::string>(core_type, "start", "inside of the functional relationship of helics",
        std::string("I am a string"));
    











    std::complex<double> def = { 54.23233, 0.7 };
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = { -3e45, 1e-23 };
    runFederateTestObj<std::complex<double>>(core_type, def, v1, v2);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<std::string> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    auto f1finish = std::async (std::launch::async, [&]() { vFed1->enterExecutionState (); });
    vFed2->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    std::string s;
    // get the value
    vFed2->getValue (subid, s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    vFed2->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    f1time = std::async (std::launch::async, [&]() { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    // make sure the value was updated

    vFed2->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
}

template <class X>
void runDualFederateTest (const std::string &core_type_str,
                          const X &defaultValue,
                          const X &testValue1,
                          const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate> (0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = fedA->registerGlobalPublication<X> ("pub1");

    auto subid = fedB->registerRequiredSubscription<X> ("pub1");
    fedA->setTimeDelta (1.0);
    fedB->setTimeDelta (1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutionState (); });
    fedB->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    fedA->publish<X> (pubid, testValue1);

    X val;
    fedB->getValue<X> (subid, val);

    BOOST_CHECK_EQUAL (val, defaultValue);

    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    fedB->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);

    // publish a second string
    fedA->publish (pubid, testValue2);
    // make sure the value is still what we expect
    fedB->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    fedB->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue2);
}

template <class X>
void runDualFederateTestv2 (const std::string &core_type_str,
                            X &defaultValue,
                            const X &testValue1,
                            const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupTest<helics::ValueFederate> (core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate> (0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = fedA->registerGlobalPublication<X> ("pub1");

    auto subid = fedB->registerRequiredSubscription<X> ("pub1");
    fedA->setTimeDelta (1.0);
    fedB->setTimeDelta (1.0);

    fedB->setDefaultValue<X> (subid, defaultValue);

    auto f1finish = std::async (std::launch::async, [&]() { fedA->enterExecutionState (); });
    fedB->enterExecutionState ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    fedA->publish<X> (pubid, testValue1);

    X val;
    fedB->getValue<X> (subid, val);
    BOOST_CHECK (val == defaultValue);
    auto f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (1.0); });
    auto gtime = fedB->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);
    // get the value
    fedB->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK (val == testValue1);
    // publish a second string
    fedA->publish (pubid, testValue2);
    // make sure the value is still what we expect
    fedB->getValue (subid, val);
    BOOST_CHECK (val == testValue1);
    // advance time
    f1time = std::async (std::launch::async, [&]() { return fedA->requestTime (2.0); });
    gtime = fedB->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);

    // make sure the value was updated
    fedB->getValue (subid, val);
    BOOST_CHECK (val == testValue2);
}

template <class X>
void runDualFederateTestObj(const std::string &core_type_str,
    const X &defaultValue,
    const X &testValue1,
    const X &testValue2)
{
    FederateTestFixture fixture;
    using namespace helics;
    fixture.SetupTest<helics::ValueFederate>(core_type_str, 2);
    auto fedA = fixture.GetFederateAs<helics::ValueFederate>(0);
    auto fedB = fixture.GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    PublicationT<X> pubid(GLOBAL, fedA.get(), "pub1");

    SubscriptionT<X> subid(fedB.get(), "pub1");
    fedA->setTimeDelta(1.0);
    fedB->setTimeDelta(1.0);

    subid.setDefault(defaultValue);

    auto f1finish = std::async(std::launch::async, [&]() { fedA->enterExecutionState(); });
    fedB->enterExecutionState();
    f1finish.wait();
    // publish string1 at time=0.0;
    pubid.publish(testValue1);

    X val = subid.getValue();

    BOOST_CHECK_EQUAL(val, defaultValue);

    auto f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(1.0); });
    auto gtime = fedB->requestTime(1.0);

    BOOST_CHECK_EQUAL(gtime, 1.0);
    BOOST_CHECK_EQUAL(f1time.get(), 1.0);
    // get the value
    subid.getValue(val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL(val, testValue1);

    // publish a second string
    pubid.publish(testValue2);

    subid.getValue(val);
    BOOST_CHECK_EQUAL(val, testValue1);

    // advance time
    f1time = std::async(std::launch::async, [&]() { return fedA->requestTime(2.0); });
    gtime = fedB->requestTime(2.0);

    BOOST_CHECK_EQUAL(gtime, 2.0);
    BOOST_CHECK_EQUAL(f1time.get(), 2.0);

    // make sure the value was updated
    fedB->getValue(subid.getID(), val);
    BOOST_CHECK_EQUAL(val, testValue2);
}
*/
/** test case checking that the transfer between two federates works as expected
 */

/*
BOOST_DATA_TEST_CASE (value_federate_single_init_publish, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<double> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterInitializationState ();
    vFed1->publish (pubid, 1.0);

    vFed1->enterExecutionState ();
    // get the value set at initialization
    double val;
    vFed1->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, 1.0);
    // publish string1 at time=0.0;
    vFed1->publish (pubid, 2.0);
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    vFed1->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, 2.0);
    // publish a second string
    vFed1->publish (pubid, 3.0);
    // make sure the value is still what we expect
    vFed1->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, 2.0);
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed1->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, 3.0);
}

*/

BOOST_DATA_TEST_CASE (value_federate_subscriber_and_publisher_registration, bdata::make (core_types), core_type)
{
    helics_status status;
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid, pubid2, pubid3;
    helics_subscription subid, subid2, subid3;
    char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4",
         pubunit3[100] = "n5";
    char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype[100] = "n4",
         subtype2[100] = "n5", subtype3[100] = "n6", subunit3[100] = "n7";
    std::cout << "value_federate_subscriber_and_publisher_registration - core_type:" << core_type << "\n";

    broker = helicsCreateBroker (core_type.c_str (), nullptr, "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE (helicsFederateInfoSetFederateName (fi, "fed0"));
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ()));

    vFed = helicsCreateValueFederate (fi);

    // register the publications
    pubid = helicsFederateRegisterPublication (vFed, "pub1", "", "");
    pubid2 = helicsFederateRegisterGlobalPublication (vFed, "pub2", "int", "");
    pubid3 = helicsFederateRegisterPublication (vFed, "pub3", "double", "V");

    // these aren't meant to match the publications
    subid = helicsFederateRegisterSubscription (vFed, "sub1", "", "");
    subid2 = helicsFederateRegisterSubscription (vFed, "sub2", "int", "");
    subid3 = helicsFederateRegisterSubscription (vFed, "sub3", "", "V");

    // enter execution
    CE (helicsFederateEnterExecutionMode (vFed));

    // check subscriptions
    CE (helicsSubscriptionGetKey (subid, subname, 100));
    CE (helicsSubscriptionGetKey (subid2, subname2, 100));

    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");
    CE (helicsSubscriptionGetKey (subid3, subname3, 100));
    BOOST_CHECK_EQUAL (subname3, "sub3");

    CE (helicsSubscriptionGetType (subid, subtype, 100));
    BOOST_CHECK_EQUAL (subtype, "def");
    CE (helicsSubscriptionGetType (subid2, subtype2, 100));
    BOOST_CHECK_EQUAL (subtype2, "int64");
    CE (helicsSubscriptionGetType (subid3, subtype3, 100));
    BOOST_CHECK_EQUAL (subtype3, "def");
    CE (helicsSubscriptionGetUnits (subid3, subunit3, 100));
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

    CE (helicsFederateFinalize (vFed));
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types), core_type)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    //	helics_time_t stime = 1.0;
    helics_time_t gtime;
    char s[100] = "n2";
    int retValue;

    broker = helicsCreateBroker (core_type.c_str (), nullptr, "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE (helicsFederateInfoSetFederateName (fi, "fed0"));
    CE (helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ()));
    CE (helicsFederateInfoSetTimeDelta (fi, 1.0));
    vFed = helicsCreateValueFederate (fi);

    // register the publications

    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "", "");
    CE (helicsFederateEnterExecutionMode (vFed));

    // publish string1 at time=0.0;
    CE (helicsPublicationPublishString (pubid, "string1"));
    CE(helicsFederateRequestTime (vFed, 1.0, &gtime));
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    CE (helicsSubscriptionGetString (subid, s, 100));
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    CE (helicsPublicationPublishString (pubid, "string2"));

    // make sure the value is still what we expect
    CE (helicsSubscriptionGetValue (subid, s, 100, &retValue));
    BOOST_CHECK_EQUAL (s, "string1");
    BOOST_CHECK_EQUAL (retValue, 7);

    // advance time
    CE(helicsFederateRequestTime (vFed, 2.0, &gtime));
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    CE (helicsSubscriptionGetValue (subid, s, 100, &retValue));
    BOOST_CHECK_EQUAL (s, "string2");

    CE (helicsFederateFinalize (vFed));
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    std::cout << "value_federate_single_transfer_publisher - core_type:" << core_type << "\n";
}

BOOST_AUTO_TEST_SUITE_END ()
