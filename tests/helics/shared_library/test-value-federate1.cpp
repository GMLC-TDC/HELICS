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

#include "test_configuration.h"

/** these test cases test out the value federates
 */

// BOOST_FIXTURE_TEST_SUITE (value_federate_tests, FederateTestFixture) // Should be used with test fixtures
BOOST_AUTO_TEST_SUITE (value_federate_tests)

namespace bdata = boost::unit_test::data;
//const std::string core_types[] = {"test", "ipc", "zmq", "test_2", "ipc_2", "zmq_2"};

/** test simple creation and destruction*/

BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types), core_type)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;

    // SetupTest<helics::ValueFederate> (core_type, 1);

    std::cout << "value_federate_initialize_tests - core_type:" << core_type << "\n";

    broker = helicsCreateBroker (core_type.c_str (), nullptr, "--federates=1");
    BOOST_REQUIRE(broker != nullptr);
    // create federate info object as the pointer to this object needs to be passed to the C API function
    // "helicsCreateValueFederate()"

    fi = helicsFederateInfoCreate ();

    CE( helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ()));
    // helicsCreateValueFederate returns a void pointer of the value federate.
    vFed = helicsCreateValueFederate (fi);
    BOOST_REQUIRE(vFed != nullptr);
    // to avoid changing the Boost test calls, the returned void pointer is cast into a ValueFederate pointer
    // helics::ValueFederate * vFed1 = reinterpret_cast<helics::ValueFederate *>(vFed);

    // rest of the commands are the same as in the C++ API tests

    CE(helicsFederateEnterExecutionMode (vFed));

    // vFed1->enterExecutionState();

    // BOOST_CHECK(vFed1->getCurrentState() == helics::Federate::op_states::execution);

    CE(helicsFederateFinalize(vFed));

    // vFed1->finalize();


    // BOOST_CHECK(vFed1->getCurrentState() == helics::Federate::op_states::finalize);

    CE(helicsFederateFinalize (vFed));
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    // helicsBrokerFree(broker);

    // helicsFederateInfoFree(fi);

    // helicsFederateFree(vFed);

    // helicsCloseLibrary();
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types), core_type)
{
    // SetupTest<helics::ValueFederate> (core_type, 1); // can be used when fixtures are enabled

    helics_status status;
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid, pubid2, pubid3;
    char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4",
         pubunit3[100] = "n5";

    std::cout << "value_federate_publication_registration - core_type:" << core_type << "\n";

    broker = helicsCreateBroker (core_type.c_str (), "", "--federates=1");

    fi = helicsFederateInfoCreate ();

    CE(helicsFederateInfoSetFederateName (fi, "fed0"));

    CE(helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ()));

    vFed = helicsCreateValueFederate (fi);

    pubid = helicsFederateRegisterPublication (vFed, "pub1", nullptr, nullptr);

    pubid2 = helicsFederateRegisterGlobalPublication (vFed, "pub2", nullptr, nullptr);

    pubid3 = helicsFederateRegisterPublication (vFed, "pub3", "double", "V");

    CE(helicsFederateEnterExecutionMode(vFed));

    /* This section is commented out as  helicsPublicationGetKey is not working without publication type and
     * units*/
    status = helicsPublicationGetKey (
      pubid, pubname, 100);  // the equivalent of getPublicationName is helicsPublicationGetKey in the C-API

   CE(helicsPublicationGetKey (pubid2, pubname2, 100));
    BOOST_CHECK_EQUAL (pubname, "fed0/pub1");

    BOOST_CHECK_EQUAL (pubname2, "pub2");

   CE(helicsPublicationGetKey (pubid3, pubname3, 100));

    BOOST_CHECK_EQUAL (pubname3, "fed0/pub3");

    CE(helicsPublicationGetType (pubid3, pubtype, 100));  // in this function the publication type is returned
                                                               // in the char * argument of the function. The
                                                               // return type is just to check that the function
                                                               // execution was successful

    BOOST_CHECK_EQUAL (pubtype, "double");

    CE(helicsPublicationGetUnits (pubid3, pubunit3, 100));

    BOOST_CHECK_EQUAL (pubunit3, "V");

    /*// getting publication id when using the C-API does not make sense, so these tests, which are valid in the
    C++ API, are being commented BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid); BOOST_CHECK
    (vFed1->getPublicationId ("pub2") == pubid2); BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);*/

    status = helicsFederateFinalize (vFed);

    BOOST_CHECK_EQUAL (status, helics_ok);

    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    // helicsBrokerFree(broker);

    // helicsFederateInfoFree(fi);

    // helicsFederateFree(vFed);

    // helicsCloseLibrary();
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types), core_type)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_subscription subid, subid2, subid3;
    char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype3[100] = "n4",
         subunit3[100] = "n5";
    std::cout << "value_federate_subscription_registration - core_type:" << core_type << "\n";

    broker = helicsCreateBroker (core_type.c_str (), "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE(helicsFederateInfoSetFederateName (fi, "fed0"));
    CE(helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str()));
    vFed = helicsCreateValueFederate (fi);

    // SetupTest<helics::ValueFederate> (core_type, 1);
    // auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    subid = helicsFederateRegisterSubscription (vFed, "sub1", "double", "V");
    subid2 = helicsFederateRegisterSubscription (vFed, "sub2", "int", "");
    // auto subid = vFed1->registerRequiredSubscription ("sub1", "double", "V");
    // auto subid2 = vFed1->registerRequiredSubscription<int> ("sub2");

    subid3 = helicsFederateRegisterSubscription (vFed, "sub3", "double", "V");
    // auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");

    CE(helicsFederateEnterExecutionMode (vFed));

    // vFed1->enterExecutionState ();

    // BOOST_CHECK (vFed->getCurrentState () == helics::Federate::op_states::execution);

    CE (helicsSubscriptionGetKey (subid, subname, 100));

    CE (helicsSubscriptionGetKey (subid2, subname2, 100));
    // auto sv = vFed1->getSubscriptionName(subid);
    // auto sv2 = vFed1->getSubscriptionName (subid2);
    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");
    // BOOST_CHECK_EQUAL (sv, "sub1");
    // BOOST_CHECK_EQUAL (sv2, "sub2");
    CE( helicsSubscriptionGetKey (subid3, subname3, 100));
    // auto sub3name = vFed1->getSubscriptionName (subid3);

    // vFed1->addSubscriptionShortcut (subid, "Shortcut"); //appears to be relevant for C++ API only
    BOOST_CHECK_EQUAL (subname3, "sub3");
    // BOOST_CHECK_EQUAL (sub3name, "sub3");

    CE(helicsSubscriptionGetType (subid3, subtype3, 100));
    BOOST_CHECK_EQUAL (subtype3, "double");
    // BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");

    CE( helicsSubscriptionGetUnits (subid3, subunit3, 100));
    BOOST_CHECK_EQUAL (subunit3, "V");
    // BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    // Similar to publications, IDs are not valid for subscriptions in C-API
    // BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    // BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    // BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid); //not relevant for C-API as subscription
    // shortcuts and IDs are relevant for C++ API only

   CE(helicsFederateFinalize (vFed));
    // vFed1->finalize ();

    // BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
    CE(helicsFederateFinalize (vFed));
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();
}

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types),
                      core_type)
{
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid, pubid2, pubid3;
    helics_subscription subid, subid2, subid3;
    char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4",
         pubunit3[100] = "n5";
    char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype3[100] = "n4",
         subunit3[100] = "n5";
    std::cout << "value_federate_subscription_and_publication_registration - core_type:" << core_type << "\n";

    broker = helicsCreateBroker (core_type.c_str (), "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    CE(helicsFederateInfoSetFederateName (fi, "fed0"));

    CE( helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ()));
    vFed = helicsCreateValueFederate (fi);

    pubid = helicsFederateRegisterPublication (vFed, "pub1", "", "");
    pubid2 = helicsFederateRegisterGlobalPublication (vFed, "pub2", "", "");
    pubid3 = helicsFederateRegisterPublication (vFed, "pub3", "double", "V");

    // replace these with optional subscription calls once these become available in the C API
    subid = helicsFederateRegisterSubscription (vFed, "sub1", "double", "V");
    subid2 = helicsFederateRegisterSubscription (vFed, "sub2", "int", "");
    subid3 = helicsFederateRegisterSubscription (vFed, "sub3", "double", "V");

    CE(helicsFederateEnterExecutionMode (vFed));

    helicsSubscriptionGetKey (subid, subname, 100);
    helicsSubscriptionGetKey (subid2, subname2, 100);
    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");

    helicsSubscriptionGetKey (subid3, subname3, 100);
    BOOST_CHECK_EQUAL (subname3, "sub3");

    helicsSubscriptionGetType (subid3, subtype3, 100);
    BOOST_CHECK_EQUAL (subtype3, "double");

    helicsSubscriptionGetUnits (subid3, subunit3, 100);
    BOOST_CHECK_EQUAL (subunit3, "V");

    helicsPublicationGetKey (pubid, pubname,
                             100);  // the equivalent of getPublicationName is helicsPublicationGetKey in the C-API
    helicsPublicationGetKey (pubid2, pubname2, 100);

    BOOST_CHECK_EQUAL (pubname, "fed0/pub1");
    BOOST_CHECK_EQUAL (pubname2, "pub2");

    helicsPublicationGetKey (pubid3, pubname3, 100);
    BOOST_CHECK_EQUAL (pubname3, "fed0/pub3");

    helicsPublicationGetType (pubid3, pubtype, 100);  // in this function the publication type is returned in the
                                                      // char * argument of the function. The return type is just
                                                      // to check that the function execution was successful
    BOOST_CHECK_EQUAL (pubtype, "double");

    helicsPublicationGetUnits (pubid3, pubunit3, 100);
    BOOST_CHECK_EQUAL (pubunit3, "V");

    CE(helicsFederateFinalize (vFed));

    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    // SetupTest<helics::ValueFederate> (core_type, 1);
    // auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    // auto pubid = vFed1->registerPublication<std::string> ("pub1");
    // auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    // auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");

    // auto subid = vFed1->registerOptionalSubscription ("sub1", "double", "V");
    // auto subid2 = vFed1->registerOptionalSubscription<int> ("sub2");

    // auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    // vFed1->enterExecutionState ();

    // BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);
    // check subscriptions
    // auto sv = vFed1->getSubscriptionName (subid);
    // auto sv2 = vFed1->getSubscriptionName (subid2);
    // BOOST_CHECK_EQUAL (sv, "sub1");
    // BOOST_CHECK_EQUAL (sv2, "sub2");
    // auto sub3name = vFed1->getSubscriptionName (subid3);
    // BOOST_CHECK_EQUAL (sub3name, "sub3");

    // BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    // BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    // check publications

    // sv = vFed1->getPublicationName (pubid);
    // sv2 = vFed1->getPublicationName (pubid2);
    // BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    // BOOST_CHECK_EQUAL (sv2, "pub2");
    // auto pub3name = vFed1->getPublicationName (pubid3);
    // BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    // BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    // BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");
    // vFed1->finalize ();

    // BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types), core_type)
{
    helics_status status;
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    // helics_time_t stime = 1.0;
    helics_time_t gtime;
    char s[100] = "n2";

    broker = helicsCreateBroker (core_type.c_str (), nullptr, "--federates=1");
    fi = helicsFederateInfoCreate ();
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ());
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "string", "");

    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsPublicationPublishString (pubid, "string1");
    BOOST_CHECK_EQUAL (status, helics_ok);
    helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    status = helicsSubscriptionGetString (subid, s, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    status = helicsPublicationPublishString (pubid, "string2");

    int actualLen;
    // make sure the value is still what we expect
    status = helicsSubscriptionGetValue (subid, s, 100, &actualLen);
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string1");
    BOOST_CHECK_EQUAL (status, helics_ok);

    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);

    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    // make sure the string is what we expect
    status = helicsSubscriptionGetValue (subid, s, 100, &actualLen);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (actualLen, 7);
    BOOST_CHECK_EQUAL (std::string (s, actualLen), "string2");

    status = helicsFederateFinalize (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    std::cout << "value_federate_single_transfer - core_type:" << core_type << "\n";
    // publish string1 at time=0.0;

    // SetupTest<helics::ValueFederate> (core_type, 1);
    // auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    // auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    // auto subid = vFed1->registerRequiredSubscription<std::string> ("pub1");
    // vFed1->setTimeDelta (1.0);
    // vFed1->enterExecutionState ();
    // publish string1 at time=0.0;
    // vFed1->publish (pubid, "string1");
    // auto gtime = vFed1->requestTime (1.0);

    // BOOST_CHECK_EQUAL (gtime, 1.0);
    // std::string s;
    // get the value
    // vFed1->getValue (subid, s);
    // make sure the string is what we expect
    // BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    // vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    // vFed1->getValue (subid, s);

    // BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    // gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    // BOOST_CHECK_EQUAL (gtime, 2.0);
    // vFed1->getValue (subid, s);

    // BOOST_CHECK_EQUAL (s, "string2");
}

// template <class X>
void runFederateTestDouble (const char *core,
                            double defaultValue,
                            double testValue1,
                            double testValue2,
                            const char *datatype)
{
    helics_status status;
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
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "double", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
    status = helicsSubscriptionSetDefaultDouble (subid, defaultValue);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);

    // publish string1 at time=0.0;
    status = helicsPublicationPublishDouble (pubid, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetDouble (subid, val);
    BOOST_CHECK_EQUAL (*val, defaultValue);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // get the value
    status = helicsSubscriptionGetDouble (subid, val);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    status = helicsPublicationPublishDouble (pubid, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the value is still what we expect
    status = helicsSubscriptionGetDouble (subid, val);
    BOOST_CHECK_EQUAL (*val, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    status = helicsSubscriptionGetDouble (subid, val);
    BOOST_CHECK_EQUAL (*val, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateFinalize (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
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
    helics_status status;
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
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "double", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "double", "");
    status = helicsSubscriptionSetDefaultDouble (subid, defaultValue);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // publish string1 at time=0.0;
    status = helicsPublicationPublishInteger (pubid, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetInteger (subid, val);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (*val, defaultValue);

    helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    status = helicsSubscriptionGetInteger (subid, val);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (*val, testValue1);

    // publish a second string
    status = helicsPublicationPublishInteger (pubid, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the value is still what we expect
    status = helicsSubscriptionGetInteger (subid, val);
    BOOST_CHECK_EQUAL (*val, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    status = helicsSubscriptionGetInteger (subid, val);
    BOOST_CHECK_EQUAL (*val, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateFinalize (vFed);

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
    helics_status status;
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
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "string", "");
    status = helicsSubscriptionSetDefaultString (subid, defaultValue);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // publish string1 at time=0.0;
    status = helicsPublicationPublishString (pubid, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetString (subid, str, len);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (str, defaultValue);

    status = helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    status = helicsSubscriptionGetString (subid, str, len);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (str, testValue1);

    // publish a second string
    status = helicsPublicationPublishString (pubid, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the value is still what we expect
    status = helicsSubscriptionGetString (subid, str, len);
    BOOST_CHECK_EQUAL (str, testValue1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    status = helicsSubscriptionGetString (subid, str, len);
    BOOST_CHECK_EQUAL (str, testValue2);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateFinalize (vFed);

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
    helics_status status;
    helics_federate_info_t fi;
    helics_broker broker;
    helics_federate vFed;
    helics_publication pubid;
    helics_subscription subid;
    helics_time_t gtime;

    double val[100] = {0};

    broker = helicsCreateBroker (core, "", "--federates=1");
    fi = helicsFederateInfoCreate ();
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // FederateTestFixture fixture;

    // fixture.SetupTest<helics::ValueFederate> (core_type_str, 1);
    // auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "vector", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "vector", "");
    status = helicsSubscriptionSetDefaultVector (subid, defaultValue, len);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // publish string1 at time=0.0;
    status = helicsPublicationPublishVector (pubid, testValue1, len1);
    int actualLen;
    status = helicsSubscriptionGetVector (subid, val, 100, &actualLen);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (actualLen, len);
    for (int i = 0; i < len; i++)
    {
        BOOST_CHECK_EQUAL (val[i], defaultValue[i]);
        std::cout << defaultValue[i] << "\n";
    }

    helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value

    status = helicsSubscriptionGetVector (subid, val, 100, &actualLen);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (actualLen, len1);
    // make sure the string is what we expect
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        std::cout << testValue1[i] << "\n";
    }

    // publish a second string
    status = helicsPublicationPublishVector (pubid, testValue2, len2);

    // make sure the value is still what we expect
    status = helicsSubscriptionGetVector (subid, val, 100, &actualLen);
    BOOST_CHECK_EQUAL (actualLen, len1);
    BOOST_CHECK_EQUAL (status, helics_ok);
    for (int i = 0; i < len1; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue1[i]);
        std::cout << testValue1[i] << "\n";
    }

    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);

    status = helicsSubscriptionGetVector (subid, val, 100, &actualLen);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (actualLen, len2);
    for (int i = 0; i < len2; i++)
    {
        BOOST_CHECK_EQUAL (val[i], testValue2[i]);
        std::cout << testValue2[i] << "\n";
    }

    status = helicsFederateFinalize (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
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
BOOST_DATA_TEST_CASE(value_federate_single_transfer_vector_string, bdata::make(core_types), core_type)
{
    char datatype[20] = "vector";
    const char * sv1[][100]={ "hip", "hop" };
    const char * val2[][100] = { "this is the first string\n", "this is the second string", 		//
"this is the third\0"                                 " string"};; const char * val3[][100] = { 9.9999,
8.8888, 7.7777 }; runFederateTestVectorD(core_type.c_str(), val1, val2, val3, datatype);
}
*/
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
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ());
    BOOST_CHECK_EQUAL (status, helics_ok);
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
    status = helicsFederateEnterExecutionMode (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);

    // check subscriptions
    status = helicsSubscriptionGetKey (subid, subname, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetKey (subid2, subname2, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (subname, "sub1");
    BOOST_CHECK_EQUAL (subname2, "sub2");
    status = helicsSubscriptionGetKey (subid3, subname3, 100);
    BOOST_CHECK_EQUAL (subname3, "sub3");

    status = helicsSubscriptionGetType (subid, subtype, 100);
    BOOST_CHECK_EQUAL (subtype, "def");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetType (subid2, subtype2, 100);
    BOOST_CHECK_EQUAL (subtype2, "int64");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetType (subid3, subtype3, 100);
    BOOST_CHECK_EQUAL (subtype3, "def");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsSubscriptionGetUnits (subid3, subunit3, 100);
    BOOST_CHECK_EQUAL (subunit3, "V");
    BOOST_CHECK_EQUAL (status, helics_ok);

    // check publications
    helicsPublicationGetKey (pubid, pubname, 100);
    status = helicsPublicationGetKey (pubid2, pubname2, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (pubname, "fed0/pub1");
    BOOST_CHECK_EQUAL (pubname2, "pub2");
    status = helicsPublicationGetKey (pubid3, pubname3, 100);
    BOOST_CHECK_EQUAL (pubname3, "fed0/pub3");
    BOOST_CHECK_EQUAL (status, helics_ok);

    status = helicsPublicationGetType (pubid3, pubtype, 100);  // in this function the publication type is returned
                                                               // in the char * argument of the function. The
                                                               // return type is just to check that the function
                                                               // execution was successful
    BOOST_CHECK_EQUAL (pubtype, "double");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsPublicationGetUnits (pubid3, pubunit3, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (pubunit3, "V");

    status = helicsFederateFinalize (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);

    status = helicsFederateFinalize (vFed);
    BOOST_CHECK_EQUAL (status, helics_ok);
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    // SetupTest<helics::ValueFederate>(core_type, 1);
    // auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    // helics::Publication pubid(vFed1.get(), "pub1", helics::helicsType<std::string>());
    // helics::PublicationT<int> pubid2(helics::GLOBAL, vFed1.get(), "pub2");

    // helics::Publication pubid3(vFed1.get(), "pub3", helics::helicsType<double>(), "V");

    // these aren't meant to match the publications
    // helics::Subscription subid1(false, vFed1.get(), "sub1");

    // helics::SubscriptionT<int> subid2(false,vFed1.get(), "sub2");

    // helics::Subscription subid3(false, vFed1.get(), "sub3", "V");
    // enter execution
    // vFed1->enterExecutionState();

    // BOOST_CHECK(vFed1->getCurrentState() == helics::Federate::op_states::execution);
    // check subscriptions
    // auto sv = subid1.getName();
    // auto sv2 = subid2.getName();
    // BOOST_CHECK_EQUAL(sv, "sub1");
    // BOOST_CHECK_EQUAL(sv2, "sub2");
    // auto sub3name = subid3.getKey();
    // BOOST_CHECK_EQUAL(sub3name, "sub3");

    // BOOST_CHECK_EQUAL(subid1.getType(), "def"); //def is the default type
    // BOOST_CHECK_EQUAL(subid2.getType(), "int32");
    // BOOST_CHECK_EQUAL(subid3.getType(), "def");
    // BOOST_CHECK_EQUAL(subid3.getUnits(), "V");

    // check publications

    // sv = pubid.getKey();
    // sv2 = pubid2.getKey();
    // BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    // BOOST_CHECK_EQUAL(sv2, "pub2");
    // auto pub3name = pubid3.getKey();
    // BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    // BOOST_CHECK_EQUAL(pubid3.getType(), "double");
    // BOOST_CHECK_EQUAL(pubid3.getUnits(), "V");
    // vFed1->finalize();

    // BOOST_CHECK(vFed1->getCurrentState() == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types), core_type)
{
    helics_status status;
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
    status = helicsFederateInfoSetFederateName (fi, "fed0");
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetCoreTypeFromString (fi, core_type.c_str ());
    BOOST_CHECK_EQUAL (status, helics_ok);
    status = helicsFederateInfoSetTimeDelta (fi, 1.0);
    BOOST_CHECK_EQUAL (status, helics_ok);
    vFed = helicsCreateValueFederate (fi);

    // register the publications

    pubid = helicsFederateRegisterGlobalPublication (vFed, "pub1", "string", "");
    subid = helicsFederateRegisterSubscription (vFed, "pub1", "", "");
    status = helicsFederateEnterExecutionMode (vFed);

    // publish string1 at time=0.0;
    status = helicsPublicationPublishString (pubid, "string1");
    BOOST_CHECK_EQUAL (status, helics_ok);
    helicsFederateRequestTime (vFed, 1.0, &gtime);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    status = helicsSubscriptionGetString (subid, s, 100);
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");

    // publish a second string
    status = helicsPublicationPublishString (pubid, "string2");
    BOOST_CHECK_EQUAL (status, helics_ok);
    // make sure the value is still what we expect
    status = helicsSubscriptionGetValue (subid, s, 100, &retValue);
    BOOST_CHECK_EQUAL (s, "string1");
    BOOST_CHECK_EQUAL (retValue, 7);
    BOOST_CHECK_EQUAL (status, helics_ok);

    // advance time
    helicsFederateRequestTime (vFed, 2.0, &gtime);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    status = helicsSubscriptionGetValue (subid, s, 100, &retValue);
    BOOST_CHECK_EQUAL (status, helics_ok);
    BOOST_CHECK_EQUAL (s, "string2");

    status = helicsFederateFinalize (vFed);
    helicsFederateInfoFree (fi);
    helicsFederateFree (vFed);
    helicsBrokerFree (broker);
    helicsCloseLibrary ();

    std::cout << "value_federate_single_transfer_publisher - core_type:" << core_type << "\n";
    // SetupTest<helics::ValueFederate>(core_type, 1);
    // auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    // helics::Publication pubid(helics::GLOBAL,vFed1.get(), "pub1",helics::helics_type_t::helicsString);

    // helics::Subscription subid(vFed1.get(),"pub1");
    // vFed1->setTimeDelta(1.0);
    // vFed1->enterExecutionState();
    // publish string1 at time=0.0;
    // pubid.publish("string1");
    // auto gtime = vFed1->requestTime(1.0);

    // BOOST_CHECK_EQUAL(gtime, 1.0);
    // std::string s;
    // get the value
    // subid.getValue(s);
    // make sure the string is what we expect
    // BOOST_CHECK_EQUAL(s, "string1");
    // publish a second string
    // pubid.publish("string2");
    // make sure the value is still what we expect
    // subid.getValue(s);

    // BOOST_CHECK_EQUAL(s, "string1");
    // advance time
    // gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    // BOOST_CHECK_EQUAL(gtime, 2.0);
    // subid.getValue(s);

    // BOOST_CHECK_EQUAL(s, "string2");
}

BOOST_AUTO_TEST_SUITE_END ()
