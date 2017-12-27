/*
Copyright (C) 2017, Battelle Memorial Institute
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

#include "ctestFixtures.h"

#include "test_configuration.h"

#include <ValueFederate_c.h>

/** these test cases test out the value federates
 */

//BOOST_FIXTURE_TEST_SUITE (value_federate_tests, FederateTestFixture) // Should be used with test fixtures
BOOST_AUTO_TEST_SUITE(value_federate_tests)

namespace bdata = boost::unit_test::data;
const std::string core_types[] = {"test", "test_2", "ipc", "ipc_2", "zmq", "zmq_2"};

/** test simple creation and destruction*/

BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types), core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;

	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);

	std::cout <<"value_federate_initialize_tests - core_type:" << core_type << "\n";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");

	// create federate info object as the pointer to this object needs to be passed to the C API function "helicsCreateValueFederate()"

	fi = helicsFederateInfoCreate();

	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());

	// helicsCreateValueFederate returns a void pointer of the value federate.
	vFed = helicsCreateValueFederate(fi);

	// to avoid changing the Boost test calls, the returned void pointer is cast into a ValueFederate pointer
	// helics::ValueFederate * vFed1 = reinterpret_cast<helics::ValueFederate *>(vFed);

	// rest of the commands are the same as in the C++ API tests

	status = helicsEnterExecutionMode(vFed);

	//vFed1->enterExecutionState();

	BOOST_CHECK(status == helicsOK);

	// BOOST_CHECK(vFed1->currentState() == helics::Federate::op_states::execution);

	status = helicsFinalize(vFed);

	//vFed1->finalize();

	BOOST_CHECK(status == helicsOK);

	//BOOST_CHECK(vFed1->currentState() == helics::Federate::op_states::finalize);

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	delete broker, vFed, fi;

	//helicsFreeBroker(broker);

	//helicsFederateInfoFree(fi);

	//helicsFreeFederate(vFed);

	//helicsCloseLibrary();
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types), core_type)
{
	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1); // can be used when fixtures are enabled

	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_publication pubid, pubid2, pubid3;
	char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4", pubunit3[100] = "n5";

	std::cout << "value_federate_publication_registration - core_type:" << core_type << "\n";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");

	fi = helicsFederateInfoCreate();

	status = helicsFederateInfoSetFederateName(fi, "fed0");

	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());

	vFed = helicsCreateValueFederate(fi);

	pubid = helicsRegisterPublication(vFed, "pub1", nullptr, nullptr);

	pubid2 = helicsRegisterGlobalPublication(vFed, "pub2", nullptr, nullptr);

	pubid3 = helicsRegisterPublication(vFed, "pub3", "double", "V");

	status = helicsEnterExecutionMode(vFed);

	BOOST_CHECK(status == helicsOK);

	/* This section is commented out as  helicsGetPublicationKey is not working without publication type and units*/
	status = helicsGetPublicationKey(pubid, pubname, 100); //the equivalent of getPublicationName is helicsGetPublicationKey in the C-API 

	status = helicsGetPublicationKey(pubid2, pubname2, 100);

	BOOST_CHECK_EQUAL (pubname, "fed0/pub1");

	BOOST_CHECK_EQUAL (pubname2, "pub2");

	status = helicsGetPublicationKey(pubid3, pubname3, 100);

	BOOST_CHECK_EQUAL(pubname3, "fed0/pub3");

	status = helicsGetPublicationType(pubid3, pubtype, 100); // in this function the publication type is returned in the char * argument of the function. The return type is just to check that the function execution was successful

	BOOST_CHECK_EQUAL(pubtype, "double");

	status = helicsGetPublicationUnits(pubid3, pubunit3, 100);

	BOOST_CHECK_EQUAL(pubunit3, "V");

	/*// getting publication id when using the C-API does not make sense, so these tests, which are valid in the C++ API, are being commented
	BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
	BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
	BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);*/

	status = helicsFinalize(vFed);

	BOOST_CHECK(status == helicsOK);

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	pubid = nullptr;
	pubid2 = nullptr;
	pubid3 = nullptr;
	delete broker, vFed, fi,pubid,pubid2,pubid3;
	//helicsFreeBroker(broker);

	//helicsFederateInfoFree(fi);

	//helicsFreeFederate(vFed);

	//helicsCloseLibrary();

}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types), core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_subscription subid, subid2, subid3;
	char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype3[100] = "n4", subunit3[100] = "n5";
	std::cout << "value_federate_subscription_registration - core_type:" << core_type << "\n";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	vFed = helicsCreateValueFederate(fi);
	
	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);
    //auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

	subid = helicsRegisterSubscription(vFed, "sub1", "double", "V");
	subid2 = helicsRegisterSubscription(vFed, "sub2", "int", "");
    //auto subid = vFed1->registerRequiredSubscription ("sub1", "double", "V");
    //auto subid2 = vFed1->registerRequiredSubscription<int> ("sub2");

	subid3= helicsRegisterSubscription(vFed, "sub3", "double", "V");
    //auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");

	status = helicsEnterExecutionMode(vFed);

	BOOST_CHECK(status == helicsOK);
    //vFed1->enterExecutionState ();

    // BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);

	status = helicsGetSubscriptionKey(subid,subname,100);
	status = helicsGetSubscriptionKey(subid2, subname2, 100);
	//auto sv = vFed1->getSubscriptionName(subid);
    //auto sv2 = vFed1->getSubscriptionName (subid2);
	BOOST_CHECK_EQUAL(subname, "sub1");
	BOOST_CHECK_EQUAL(subname2, "sub2");
    //BOOST_CHECK_EQUAL (sv, "sub1");
    //BOOST_CHECK_EQUAL (sv2, "sub2");
	status = helicsGetSubscriptionKey(subid3, subname3, 100);
    //auto sub3name = vFed1->getSubscriptionName (subid3);

    //vFed1->addSubscriptionShortcut (subid, "Shortcut"); //appears to be relevant for C++ API only 
	BOOST_CHECK_EQUAL(subname3, "sub3");
	//BOOST_CHECK_EQUAL (sub3name, "sub3");

	status = helicsGetSubscriptionType(subid3, subtype3, 100);
	BOOST_CHECK_EQUAL(subtype3, "double");
    //BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");

	status = helicsGetSubscriptionUnits(subid3, subunit3, 100);
	BOOST_CHECK_EQUAL(subunit3, "V");
    //BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

	// Similar to publications, IDs are not valid for subscriptions in C-API
    //BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    //BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    //BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid); //not relevant for C-API as subscription shortcuts and IDs are relevant for C++ API only

	status = helicsFinalize(vFed);

	BOOST_CHECK(status == helicsOK);
    //vFed1->finalize ();

    //BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);
	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	subid = nullptr;
	subid2 = nullptr;
	subid3 = nullptr;
	delete broker, vFed, fi, subid, subid2, subid3;
} 

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types),
                      core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_publication pubid, pubid2, pubid3;
	helics_subscription subid, subid2, subid3;
	char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4", pubunit3[100] = "n5";
	char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype3[100] = "n4", subunit3[100] = "n5";
	std::cout << "value_federate_subscription_and_publication_registration - core_type:" << core_type << "\n";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	vFed = helicsCreateValueFederate(fi);

	pubid = helicsRegisterPublication(vFed, "pub1", "", "");
	pubid2 = helicsRegisterGlobalPublication(vFed, "pub2", "", "");
	pubid3 = helicsRegisterPublication(vFed, "pub3", "double", "V");
	
	// replace these with optional subscription calls once these become available in the C API
	subid = helicsRegisterSubscription(vFed, "sub1", "double", "V");
	subid2 = helicsRegisterSubscription(vFed, "sub2", "int", "");
	subid3 = helicsRegisterSubscription(vFed, "sub3", "double", "V");

	status = helicsEnterExecutionMode(vFed);
	BOOST_CHECK(status == helicsOK);

	status = helicsGetSubscriptionKey(subid, subname, 100);
	status = helicsGetSubscriptionKey(subid2, subname2, 100);
	BOOST_CHECK_EQUAL(subname, "sub1");
	BOOST_CHECK_EQUAL(subname2, "sub2");

	status = helicsGetSubscriptionKey(subid3, subname3, 100);
	BOOST_CHECK_EQUAL(subname3, "sub3");

	status = helicsGetSubscriptionType(subid3, subtype3, 100);
	BOOST_CHECK_EQUAL(subtype3, "double");

	status = helicsGetSubscriptionUnits(subid3, subunit3, 100);
	BOOST_CHECK_EQUAL(subunit3, "V");

	status = helicsGetPublicationKey(pubid, pubname, 100); //the equivalent of getPublicationName is helicsGetPublicationKey in the C-API 
	status = helicsGetPublicationKey(pubid2, pubname2, 100);

	BOOST_CHECK_EQUAL(pubname, "fed0/pub1");
	BOOST_CHECK_EQUAL(pubname2, "pub2");

	status = helicsGetPublicationKey(pubid3, pubname3, 100);
	BOOST_CHECK_EQUAL(pubname3, "fed0/pub3");

	status = helicsGetPublicationType(pubid3, pubtype, 100); // in this function the publication type is returned in the char * argument of the function. The return type is just to check that the function execution was successful
	BOOST_CHECK_EQUAL(pubtype, "double");

	status = helicsGetPublicationUnits(pubid3, pubunit3, 100);
	BOOST_CHECK_EQUAL(pubunit3, "V");

	status = helicsFinalize(vFed);
	BOOST_CHECK(status == helicsOK);

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	subid = nullptr;
	subid2 = nullptr;
	subid3 = nullptr;
	pubid = nullptr;
	pubid2 = nullptr;
	pubid3 = nullptr;
	delete broker, fi, vFed, subid, subid2, subid3, pubid, pubid2, pubid3;
	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);
    //auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    //auto pubid = vFed1->registerPublication<std::string> ("pub1");
    //auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    //auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");

    //auto subid = vFed1->registerOptionalSubscription ("sub1", "double", "V");
    //auto subid2 = vFed1->registerOptionalSubscription<int> ("sub2");

    //auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    //vFed1->enterExecutionState ();

    //BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::execution);
    // check subscriptions
    //auto sv = vFed1->getSubscriptionName (subid);
    //auto sv2 = vFed1->getSubscriptionName (subid2);
    //BOOST_CHECK_EQUAL (sv, "sub1");
    //BOOST_CHECK_EQUAL (sv2, "sub2");
    //auto sub3name = vFed1->getSubscriptionName (subid3);
    //BOOST_CHECK_EQUAL (sub3name, "sub3");

    //BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    //BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    // check publications

    //sv = vFed1->getPublicationName (pubid);
    //sv2 = vFed1->getPublicationName (pubid2);
    //BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    //BOOST_CHECK_EQUAL (sv2, "pub2");
    //auto pub3name = vFed1->getPublicationName (pubid3);
    //BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    //BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    //BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");
    //vFed1->finalize ();

    //BOOST_CHECK (vFed1->currentState () == helics::Federate::op_states::finalize);

}


BOOST_DATA_TEST_CASE(value_federate_subscriber_and_publisher_registration,
    bdata::make(core_types),
    core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_publication pubid, pubid2, pubid3;
	helics_subscription subid, subid2, subid3;
	char pubname[100] = "n1", pubname2[100] = "n2", pubname3[100] = "n3", pubtype[100] = "n4", pubunit3[100] = "n5";
	char subname[100] = "n1", subname2[100] = "n2", subname3[100] = "n3", subtype[100] = "n4", subtype2[100] = "n5", subtype3[100] = "n6", subunit3[100] = "n7";
	std::cout << "value_federate_subscriber_and_publisher_registration - core_type:" << core_type << "\n";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	vFed = helicsCreateValueFederate(fi);

	// register the publications
	pubid = helicsRegisterPublication(vFed, "pub1", "", "");
	pubid2 = helicsRegisterGlobalPublication(vFed, "pub2", "int", "");
	pubid3 = helicsRegisterPublication(vFed, "pub3", "double", "V");

	//these aren't meant to match the publications
	subid = helicsRegisterSubscription(vFed, "sub1", "", "");
	subid2 = helicsRegisterSubscription(vFed, "sub2", "int", "");
	subid3 = helicsRegisterSubscription(vFed, "sub3", "", "V");

	// enter execution
	status = helicsEnterExecutionMode(vFed);
	BOOST_CHECK(status == helicsOK);

	// check subscriptions
	status = helicsGetSubscriptionKey(subid, subname, 100);
	status = helicsGetSubscriptionKey(subid2, subname2, 100);
	BOOST_CHECK_EQUAL(subname, "sub1");
	BOOST_CHECK_EQUAL(subname2, "sub2");
	status = helicsGetSubscriptionKey(subid3, subname3, 100);
	BOOST_CHECK_EQUAL(subname3, "sub3");

	status = helicsGetSubscriptionType(subid, subtype, 100);
	BOOST_CHECK_EQUAL(subtype, "def");
	status = helicsGetSubscriptionType(subid2, subtype2, 100);
	BOOST_CHECK_EQUAL(subtype2, "int32");
	status = helicsGetSubscriptionType(subid3, subtype3, 100);
	BOOST_CHECK_EQUAL(subtype3, "def");
	status = helicsGetSubscriptionUnits(subid3, subunit3, 100);
	BOOST_CHECK_EQUAL(subunit3, "V");

	// check publications
	status = helicsGetPublicationKey(pubid, pubname, 100); //the equivalent of getPublicationName is helicsGetPublicationKey in the C-API 
	status = helicsGetPublicationKey(pubid2, pubname2, 100);
	BOOST_CHECK_EQUAL(pubname, "fed0/pub1");
	BOOST_CHECK_EQUAL(pubname2, "pub2");
	status = helicsGetPublicationKey(pubid3, pubname3, 100);
	BOOST_CHECK_EQUAL(pubname3, "fed0/pub3");

	status = helicsGetPublicationType(pubid3, pubtype, 100); // in this function the publication type is returned in the char * argument of the function. The return type is just to check that the function execution was successful
	BOOST_CHECK_EQUAL(pubtype, "double");
	status = helicsGetPublicationUnits(pubid3, pubunit3, 100);
	BOOST_CHECK_EQUAL(pubunit3, "V");

	status = helicsFinalize(vFed);
	BOOST_CHECK(status == helicsOK);

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	subid = nullptr;
	subid2 = nullptr;
	subid3 = nullptr;
	pubid = nullptr;
	pubid2 = nullptr;
	pubid3 = nullptr;
	delete broker, fi, vFed, subid, subid2, subid3, pubid, pubid2, pubid3;
	//SetupSingleBrokerTest<helics::ValueFederate>(core_type, 1);
    //auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    //helics::Publication pubid(vFed1.get(), "pub1", helics::helicsType<std::string>());
    //helics::PublicationT<int> pubid2(helics::GLOBAL, vFed1.get(), "pub2");

    //helics::Publication pubid3(vFed1.get(), "pub3", helics::helicsType<double>(), "V");

    //these aren't meant to match the publications
    //helics::Subscription subid1(false, vFed1.get(), "sub1");

    //helics::SubscriptionT<int> subid2(false,vFed1.get(), "sub2");

    //helics::Subscription subid3(false, vFed1.get(), "sub3", "V");
    // enter execution
    //vFed1->enterExecutionState();

    //BOOST_CHECK(vFed1->currentState() == helics::Federate::op_states::execution);
    // check subscriptions
    //auto sv = subid1.getName();
    //auto sv2 = subid2.getName();
    //BOOST_CHECK_EQUAL(sv, "sub1");
    //BOOST_CHECK_EQUAL(sv2, "sub2");
    //auto sub3name = subid3.getKey();
    //BOOST_CHECK_EQUAL(sub3name, "sub3");

    //BOOST_CHECK_EQUAL(subid1.getType(), "def"); //def is the default type
    //BOOST_CHECK_EQUAL(subid2.getType(), "int32");
    //BOOST_CHECK_EQUAL(subid3.getType(), "def");
    //BOOST_CHECK_EQUAL(subid3.getUnits(), "V");

    // check publications

    //sv = pubid.getKey();
    //sv2 = pubid2.getKey();
    //BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    //BOOST_CHECK_EQUAL(sv2, "pub2");
    //auto pub3name = pubid3.getKey();
    //BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    //BOOST_CHECK_EQUAL(pubid3.getType(), "double");
    //BOOST_CHECK_EQUAL(pubid3.getUnits(), "V");
    //vFed1->finalize();

    //BOOST_CHECK(vFed1->currentState() == helics::Federate::op_states::finalize);
}


BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types), core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_publication pubid;
	helics_subscription subid;
	helics_time_t stime = 1.0;
	helics_time_t gtime;
	int retValue;
	char s[100] = "n2";
	
	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	status = helicsFederateInfoSetTimeDelta(fi, 1.0);
	vFed = helicsCreateValueFederate(fi);

	// register the publications
	pubid = helicsRegisterGlobalPublication(vFed, "pub1", "string", "");
	subid = helicsRegisterSubscription(vFed, "pub1", "string", "");


	status = helicsEnterExecutionMode(vFed);
	status= helicsPublishString(pubid, "string1");
	gtime = helicsRequestTime(vFed,1.0);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	// get the value
	retValue = helicsGetString(subid, s, 100);

	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");

	// publish a second string
	status = helicsPublishString(pubid, "string2");

	// make sure the value is still what we expect
	retValue = helicsGetValue(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string1");

	// advance time
	gtime = helicsRequestTime(vFed, 2.0);

	// make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);

	// make sure the string is what we expect
	retValue = helicsGetValue(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string2");

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	subid = nullptr;
	pubid = nullptr;
	delete broker, fi, vFed, subid, pubid;
	std::cout << "value_federate_single_transfer - core_type:" << core_type << "\n";
	// publish string1 at time=0.0;

	//SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);
    //auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    //auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    //auto subid = vFed1->registerRequiredSubscription<std::string> ("pub1");
    //vFed1->setTimeDelta (1.0);
    //vFed1->enterExecutionState ();
    // publish string1 at time=0.0;
    //vFed1->publish (pubid, "string1");
    //auto gtime = vFed1->requestTime (1.0);

    //BOOST_CHECK_EQUAL (gtime, 1.0);
    //std::string s;
    // get the value
    //vFed1->getValue (subid, s);
    // make sure the string is what we expect
    //BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    //vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    //vFed1->getValue (subid, s);

    //BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    //gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    //BOOST_CHECK_EQUAL (gtime, 2.0);
    //vFed1->getValue (subid, s);

    //BOOST_CHECK_EQUAL (s, "string2");
}


BOOST_DATA_TEST_CASE(value_federate_single_transfer_publisher, bdata::make(core_types), core_type)
{
	helicsStatus status;
	helics_federate_info_t fi;
	helics_broker broker;
	helics_federate vFed;
	helics_publication pubid;
	helics_subscription subid;
	helics_time_t stime = 1.0;
	helics_time_t gtime;
	int retValue;
	char s[100] = "n2";

	broker = helicsCreateBroker(core_type.c_str(), nullptr, "--federates=1");
	fi = helicsFederateInfoCreate();
	status = helicsFederateInfoSetFederateName(fi, "fed0");
	status = helicsFederateInfoSetCoreTypeFromString(fi, core_type.c_str());
	status = helicsFederateInfoSetTimeDelta(fi, 1.0);
	vFed = helicsCreateValueFederate(fi);

	// register the publications

	pubid = helicsRegisterGlobalPublication(vFed, "pub1", "string", "");
	subid = helicsRegisterSubscription(vFed, "pub1", "", "");
	status = helicsEnterExecutionMode(vFed);

	// publish string1 at time=0.0;
	status = helicsPublishString(pubid, "string1");
	gtime = helicsRequestTime(vFed, 1.0);
	BOOST_CHECK_EQUAL(gtime, 1.0);

	// get the value
	retValue = helicsGetString(subid, s, 100);

	// make sure the string is what we expect
	BOOST_CHECK_EQUAL(s, "string1");

	// publish a second string
	status = helicsPublishString(pubid, "string2");
	// make sure the value is still what we expect
	retValue = helicsGetValue(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string1");

	// advance time
	gtime = helicsRequestTime(vFed, 2.0);
	// make sure the value was updated
	BOOST_CHECK_EQUAL(gtime, 2.0);
	retValue = helicsGetValue(subid, s, 100);
	BOOST_CHECK_EQUAL(s, "string2");

	status = helicsFinalize(vFed);
	helicsFederateInfoFree(fi);
	helicsFreeFederate(vFed);
	helicsCloseLibrary();
	broker = nullptr;
	vFed = nullptr;
	fi = nullptr;
	subid = nullptr;
	pubid = nullptr;
	delete broker, fi, vFed, subid, pubid;

	std::cout << "value_federate_single_transfer_publisher - core_type:" << core_type << "\n";
	//SetupSingleBrokerTest<helics::ValueFederate>(core_type, 1);
    //auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    //helics::Publication pubid(helics::GLOBAL,vFed1.get(), "pub1",helics::helicsType_t::helicsString);

    //helics::Subscription subid(vFed1.get(),"pub1");
    //vFed1->setTimeDelta(1.0);
    //vFed1->enterExecutionState();
    // publish string1 at time=0.0;
    //pubid.publish("string1");
    //auto gtime = vFed1->requestTime(1.0);

    //BOOST_CHECK_EQUAL(gtime, 1.0);
    //std::string s;
    // get the value
    //subid.getValue(s);
    // make sure the string is what we expect
    //BOOST_CHECK_EQUAL(s, "string1");
    // publish a second string
   // pubid.publish("string2");
    // make sure the value is still what we expect
    //subid.getValue(s);

    //BOOST_CHECK_EQUAL(s, "string1");
    // advance time
    //gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    //BOOST_CHECK_EQUAL(gtime, 2.0);
    //subid.getValue(s);

    //BOOST_CHECK_EQUAL(s, "string2");
}

/*
template <class X>
void runFederateTest (const std::string &core_type_str,
                      const X &defaultValue,
                      const X &testValue1,
                      const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupSingleBrokerTest<helics::ValueFederate> (core_type_str, 1);
    auto vFed = fixture.GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed->registerGlobalPublication<X> ("pub1");

    auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed->publish<X> (pubid, testValue1);

    X val;
    vFed->getValue<X> (subid, val);
    BOOST_CHECK_EQUAL (val, defaultValue);

    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    vFed->getValue (subid, val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, testValue1);
    // publish a second string
    vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
    vFed->getValue (subid, val);
    BOOST_CHECK_EQUAL (val, testValue1);

    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed->getValue (subid, val);

    BOOST_CHECK_EQUAL (val, testValue2);

    vFed->finalize ();
}

template <class X>
void runFederateTestObj(const std::string &core_type_str,
    const X &defaultValue,
    const X &testValue1,
    const X &testValue2)
{
    FederateTestFixture fixture;

    fixture.SetupSingleBrokerTest<helics::ValueFederate>(core_type_str, 1);
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
    fixture.SetupSingleBrokerTest<helics::ValueFederate> (core_type_str, 1);    auto vFed =
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
BOOST_DATA_TEST_CASE (value_federate_single_transfer_types, bdata::make (core_types), core_type)
{
    runFederateTest<double> (core_type, 10.3, 45.3, 22.7);    runFederateTest<double> (core_type, 1.0, 0.0, 3.0);
runFederateTest<int> (core_type, 5, 8, 43);    runFederateTest<int> (core_type, -5, 1241515, -43);
runFederateTest<short> (core_type, -5, 23023, -43); runFederateTest<uint64_t> (core_type, 234252315,
0xFFF1'2345'7124'1412, 23521513412); runFederateTest<float> (core_type, 10.3f, 45.3f, 22.7f);
    runFederateTest<std::string> (core_type, "start", "inside of the functional relationship of helics",
std::string ("I am a string")); runFederateTestv2<std::vector<double>> (core_type, {34.3, 24.2}, {12.4,
14.7, 16.34, 18.17}, {9.9999, 8.8888, 7.7777}); std::vector<std::string> sv1{"hip", "hop"};
    std::vector<std::string> sv2{"this is the first string\n", "this is the second string",
                                 "this is the third\0"                                 " string"};
std::vector<std::string> sv3{"string1", "String2", "string3", "string4", "string5", "string6", "string8"};
    runFederateTestv2 (core_type, sv1, sv2, sv3);
    std::complex<double> def = {54.23233, 0.7};    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

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
    SetupSingleBrokerTest<helics::ValueFederate> (core_type, 2);
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

    fixture.SetupSingleBrokerTest<helics::ValueFederate> (core_type_str, 2);
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

    fixture.SetupSingleBrokerTest<helics::ValueFederate> (core_type_str, 2);
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
    fixture.SetupSingleBrokerTest<helics::ValueFederate>(core_type_str, 2);
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
    SetupSingleBrokerTest<helics::ValueFederate> (core_type, 1);
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
BOOST_AUTO_TEST_SUITE_END ()
