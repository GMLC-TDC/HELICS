#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include "helics/application_api/ValueFederate.h"
#include "test_configuration.h"
#include "zmqBrokerRunner.h"
#include <future>
/** these test cases test out the value federates
 */

BOOST_AUTO_TEST_SUITE (zmq_value_federate_tests1)

static const BrokerRunner zmqRunner(HELICSINSTALL_LOCATION, HELICS_ZMQ_BROKER_LOCATION, "helics_broker");
#define ZMQ_CORE_TYPE "zmq"

/** test simple creation and destruction*/
BOOST_AUTO_TEST_CASE (value_federate_initialize_tests)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });

    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);

    vFed->enterExecutionState ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);

    vFed->finalize ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::finalize);

	auto val=fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}

BOOST_AUTO_TEST_CASE (value_federate_publication_registration)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);

    auto pubid = vFed->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed->registerPublication ("pub3", "double", "V");
    vFed->enterExecutionState ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);

    auto sv = vFed->getPublicationName (pubid);
    auto sv2 = vFed->getPublicationName (pubid2);
    BOOST_CHECK_EQUAL (sv, "test1/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed->getPublicationName (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "test1/pub3");

    BOOST_CHECK_EQUAL (vFed->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed->getPublicationUnits (pubid3), "V");

	BOOST_CHECK(vFed->getPublicationId("pub1") == pubid);
	BOOST_CHECK(vFed->getPublicationId("pub2") == pubid2);
	BOOST_CHECK(vFed->getPublicationId("test1/pub1") == pubid);
    vFed->finalize ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::finalize);

	auto val = fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}


BOOST_AUTO_TEST_CASE (value_federate_subscription_registration)
{

	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);

    auto subid = vFed->registerRequiredSubscription ("sub1", "double", "V");
    auto subid2 = vFed->registerRequiredSubscription<int> ("sub2");

    auto subid3 = vFed->registerOptionalSubscription ("sub3", "double", "V");
    vFed->enterExecutionState ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);

    auto sv = vFed->getSubscriptionName (subid);
    auto sv2 = vFed->getSubscriptionName (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed->getSubscriptionName (subid3);

	vFed->addSubscriptionShortcut(subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed->getSubscriptionUnits (subid3), "V");

	BOOST_CHECK(vFed->getSubscriptionId("sub1") == subid);
	BOOST_CHECK(vFed->getSubscriptionId("sub2") == subid2);

	BOOST_CHECK(vFed->getSubscriptionId("Shortcut") == subid);
	
    vFed->finalize ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::finalize);
	auto val = fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}


BOOST_AUTO_TEST_CASE (value_federate_subscription_and_publication_registration)
{
    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });

    auto vFed = std::make_shared<helics::ValueFederate> (fi);

    // register the publications
    auto pubid = vFed->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed->registerPublication ("pub3", "double", "V");


    auto subid = vFed->registerOptionalSubscription ("sub1", "double", "V");
    auto subid2 = vFed->registerOptionalSubscription<int> ("sub2");

    auto subid3 = vFed->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    vFed->enterExecutionState ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::execution);
    // check subscriptions
    auto sv = vFed->getSubscriptionName (subid);
    auto sv2 = vFed->getSubscriptionName (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed->getSubscriptionName (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed->getSubscriptionUnits (subid3), "V");


    // check publications

    sv = vFed->getPublicationName (pubid);
    sv2 = vFed->getPublicationName (pubid2);
    BOOST_CHECK_EQUAL (sv, "test1/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed->getPublicationName (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "test1/pub3");

    BOOST_CHECK_EQUAL (vFed->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed->getPublicationUnits (pubid3), "V");
    vFed->finalize ();

    BOOST_CHECK (vFed->currentState () == helics::Federate::op_states::finalize);
	auto val = fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}

BOOST_AUTO_TEST_CASE (value_federate_single_transfer)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });

    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubid = vFed->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed->registerRequiredSubscription<std::string> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed->publish (pubid, "string1");
    auto gtime = vFed->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s;
    // get the value
    vFed->getValue (subid, s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed->publish (pubid, "string2");
    // make sure the value is still what we expect
    vFed->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed->finalize ();
	auto val = fut.get();
	BOOST_CHECK_EQUAL(val, 0);
}


template <class X>
void runFederateTest (const X &defaultValue, const X &testValue1, const X &testValue2)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });

    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
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
	auto finalval = fut.get();
	BOOST_CHECK_EQUAL(finalval, 0);
}

template <class X>
void runFederateTestv2 (const X &defaultValue, const X &testValue1, const X &testValue2)
{
	//launch the broker
	auto fut = std::async(std::launch::async, [&]() {return zmqRunner.run("1"); });
    helics::FederateInfo fi ("test1");
    fi.coreType = ZMQ_CORE_TYPE;
    fi.coreInitString = "1";

    auto vFed = std::make_shared<helics::ValueFederate> (fi);
    // register the publications
    auto pubid = vFed->registerGlobalPublication<X> ("pub1");

    auto subid = vFed->registerRequiredSubscription<X> ("pub1");
    vFed->setTimeDelta (1.0);
    vFed->setDefaultValue<X> (subid, defaultValue);
    vFed->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed->publish<X> (pubid, testValue1);

    X val;
	try
	{
		vFed->getValue<X>(subid, val);
		BOOST_CHECK(val == defaultValue);
	}
	catch (const std::invalid_argument &)
	{
		BOOST_CHECK(false);
	}
    

    auto gtime = vFed->requestTime (1.0);
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
	try
	{
		vFed->getValue(subid, val);
		BOOST_CHECK(val == testValue1);
	}
	catch (const std::invalid_argument &)
	{
		BOOST_CHECK_MESSAGE(false, "ERROR thrown wrong data size");
   }
    // make sure the string is what we expect

    
    // publish a second string
    vFed->publish (pubid, testValue2);
    // make sure the value is still what we expect
	try
	{
		vFed->getValue(subid, val);
		BOOST_CHECK(val == testValue1);
	}
	catch (const std::invalid_argument &)
	{
		BOOST_CHECK_MESSAGE(false, "ERROR thrown wrong data size");
	}
    // advance time
    gtime = vFed->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
	try
	{
		vFed->getValue(subid, val);
		BOOST_CHECK(val == testValue2);
	}
	catch (std::invalid_argument &)
	{
		BOOST_CHECK_MESSAGE(false,"ERROR thrown wrong data size");
	}
    
    vFed->finalize ();
	auto finalval = fut.get();
	BOOST_CHECK_EQUAL(finalval, 0);
}


BOOST_AUTO_TEST_CASE(value_federate_single_transfer_double)
{
	runFederateTest<double>(10.3, 45.3, 22.7);
	runFederateTest<double>(1.0, 0.0, 3.0);
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_int)
{
	runFederateTest<int>(5, 8, 43);
	runFederateTest<int>(-5, 1241515, -43);
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_short)
{
	runFederateTest<short>(-5, 23023, -43);
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_uint64)
{
	runFederateTest<uint64_t>(234252315, 0xFFF1'2345'7124'1412, 23521513412);

}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_float)
{
	runFederateTest<float>(10.3f, 45.3f, 22.7f);
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_string)
{
	runFederateTest<std::string>("start", "inside of the functional relationship of helics",
		std::string("I am a string"));

	runFederateTest<std::string>("\n\n\n\n\n\n\n\n\n\n\0\0  \0\0\t\t\t\n\n\n\r\0\r\0", R"(\0\n\0\b\t\r234#$%*(&!@*#&!_%!#%!#*&!(#*$!)",
		std::string("this is\n\r an awkward string"));
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_double_vector)
{
	runFederateTestv2<std::vector<double>>({ 34.3, 24.2 }, { 12.4, 14.7, 16.34, 18.17 }, { 9.9999, 8.8888, 7.7777 });
	runFederateTestv2<std::vector<double>>({ 0.0, 2.0 }, { 3.0, std::nan(nullptr), 1e-245, 5.0 }, { std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), 50.0 });
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_string_vector)
{
	std::vector<std::string> sv1{ "hip", "hop" };
	std::vector<std::string> sv2{ "this is the first string\n", "this is the second string",
		"this is the third\0"
		" string" };
	std::vector<std::string> sv3{ "string1", "String2", "string3", "string4", "string5", "string6", "string8" };
	runFederateTestv2(sv1, sv2, sv3);
}

BOOST_AUTO_TEST_CASE(value_federate_single_transfer_complex)
{
	std::complex<double> def = { 54.23233, 0.7 };
	std::complex<double> v1 = std::polar(10.0, 0.43);
	std::complex<double> v2 = { -3e45, 1e-23 };
	runFederateTest<std::complex<double>>(def, v1, v2);
}

BOOST_AUTO_TEST_SUITE_END ()