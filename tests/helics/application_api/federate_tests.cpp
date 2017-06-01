#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include "helics/core/core.h"
#include "helics/application_api/Federate.h"
#include "helics/application_api/coreInstantiation.h"
#include "test_configuration.h"
#include <future>
/** these test cases test out the value converters
*/

BOOST_AUTO_TEST_SUITE(federate_tests)

BOOST_AUTO_TEST_CASE(federate_initialize_tests)
{

	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";
	
	auto Fed = std::make_shared<helics::Federate>(fi);

	auto core = Fed->getCorePointer();
	BOOST_REQUIRE((core));

	auto name = std::string(core->getFederateName(Fed->getID()));

	BOOST_CHECK_EQUAL(name, Fed->getName());
	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::startup);
	Fed->enterInitializationState();
	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::initialization);
	Fed->enterExecutionState();
	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::execution);
	Fed = nullptr;
	closeCore("");
}


BOOST_AUTO_TEST_CASE(federate_time_step_tests)
{

	helics::FederateInfo fi("test1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "1";

	auto Fed = std::make_shared<helics::Federate>(fi);


	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::startup);
	Fed->enterInitializationState();
	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::initialization);
	Fed->enterExecutionState();
	BOOST_CHECK(Fed->currentState() == helics::Federate::op_states::execution);

	auto res = Fed->requestTime(1.0);
	BOOST_CHECK_EQUAL(res, 1.0);
	res = Fed->requestTime(2.0);
	BOOST_CHECK_EQUAL(res, 2.0);

	res = Fed->requestTime(3.0);
	BOOST_CHECK_EQUAL(res, 3.0);


	Fed = nullptr;
}


BOOST_AUTO_TEST_CASE(federate_multiple_federates)
{

	helics::FederateInfo fi("fed1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto Fed1 = std::make_shared<helics::Federate>(fi);

	fi.name = "fed2";
	auto Fed2 = std::make_shared<helics::Federate>(fi);

	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::startup);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::startup);

	BOOST_CHECK_NE(Fed1->getID(), Fed2->getID());

	auto f1finish = std::async(std::launch::async, [&]() {Fed1->enterInitializationState(); });
	Fed2->enterInitializationState();

	f1finish.wait();
	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::initialization);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::initialization);


	f1finish = std::async(std::launch::async, [&]() {Fed1->enterExecutionState(); });
	Fed2->enterExecutionState();
	f1finish.wait();
	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::execution);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::execution);

	auto f1step = std::async(std::launch::async, [&]() {return Fed1->requestTime(1.0); });
	auto f2step = Fed2->requestTime(1.0);

	auto f1stepVal = f1step.get();
	BOOST_CHECK_EQUAL(f2step, 1.0);
	BOOST_CHECK_EQUAL(f1stepVal, 1.0);

	BOOST_CHECK_EQUAL(Fed1->getCurrentTime(), 1.0);

	f1step = std::async(std::launch::async, [&]() {return Fed1->requestTime(3.0); });
	f2step = Fed2->requestTime(3.0);

	f1stepVal = f1step.get();
	BOOST_CHECK_EQUAL(f2step, 3.0);
	BOOST_CHECK_EQUAL(f1stepVal, 3.0);

	BOOST_CHECK_THROW(Fed1->enterInitializationState(), helics::InvalidStateTransition);
	Fed1->finalize();
	Fed2->finalize();
}


BOOST_AUTO_TEST_CASE(federate_multiple_federates_async_calls)
{

	helics::FederateInfo fi("fed1");
	fi.coreType = CORE_TYPE_TO_TEST;
	fi.coreInitString = "2";

	auto Fed1 = std::make_shared<helics::Federate>(fi);

	fi.name = "fed2";
	auto Fed2 = std::make_shared<helics::Federate>(fi);

	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::startup);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::startup);

	BOOST_CHECK_NE(Fed1->getID(), Fed2->getID());

	Fed1->enterInitializationStateAsync();
	Fed2->enterInitializationState();

	Fed1->enterInitializationStateFinalize();

	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::initialization);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::initialization);


	Fed1->enterExecutionStateAsync();
	Fed2->enterExecutionState();
	Fed1->enterExecutionStateFinalize();
	BOOST_CHECK(Fed1->currentState() == helics::Federate::op_states::execution);
	BOOST_CHECK(Fed2->currentState() == helics::Federate::op_states::execution);

	Fed1->requestTimeAsync(1.0);
	auto f2step = Fed2->requestTime(1.0);

	auto f1stepVal = Fed1->requestTimeFinalize();
	BOOST_CHECK_EQUAL(f2step, 1.0);
	BOOST_CHECK_EQUAL(f1stepVal, 1.0);

	BOOST_CHECK_EQUAL(Fed1->getCurrentTime(), 1.0);

	Fed1->requestTimeAsync(3.0);
	f2step = Fed2->requestTime(3.0);

	f1stepVal = Fed1->requestTimeFinalize();
	BOOST_CHECK_EQUAL(f2step, 3.0);
	BOOST_CHECK_EQUAL(f1stepVal, 3.0);

	BOOST_CHECK_THROW(Fed1->enterInitializationState(), helics::InvalidStateTransition);
	BOOST_CHECK_THROW(Fed1->requestTimeFinalize(), helics::InvalidFunctionCall);
	Fed1->finalize();
	Fed2->finalize();
}
BOOST_AUTO_TEST_SUITE_END()