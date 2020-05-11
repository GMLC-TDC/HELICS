/*
Copyright (c) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle
Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for
Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence
Livermore National Security, LLC.
*/

#include "ctestFixtures.hpp"

#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>
#include <future>
/** these test cases test out the message federates
 */

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE(filter_tests_cpp, FederateTestFixture, *utf::label("ci"))

/** test registration of filters*/

BOOST_TEST_DECORATOR(*utf::timeout(12))
BOOST_DATA_TEST_CASE(message_filter_registration, bdata::make(core_types), core_type)
{
    auto broker = AddBroker(core_type, 2);
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, helics_time_zero, "filter");
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, helics_time_zero, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    helicsFederateRegisterGlobalEndpoint(mFed, "port1", "");
    helicsFederateRegisterGlobalEndpoint(mFed, "port2", NULL);

    auto f1 = helicsFederateRegisterSourceFilter(fFed, helics_custom_filter, "filter1", "port1");
    BOOST_CHECK(f1 != NULL);
    auto f2 =
        helicsFederateRegisterDestinationFilter(fFed, helics_custom_filter, "filter2", "port2");
    BOOST_CHECK(f2 != f1);
    auto ep1 = helicsFederateRegisterEndpoint(fFed, "fout", "");
    BOOST_CHECK(ep1 != NULL);
    auto f3 = helicsFederateRegisterSourceFilter(fFed, helics_custom_filter, "", "filter0/fout");
    BOOST_CHECK(f3 != f2);
    CE(helicsFederateFinalize(mFed));
    CE(helicsFederateFinalize(fFed));
    federate_state state;
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

BOOST_TEST_DECORATOR(*utf::timeout(12))
BOOST_DATA_TEST_CASE(message_filter_function, bdata::make(core_types), core_type)
{
    auto broker = AddBroker(core_type, 2);
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "");
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "");

    auto f1 = helicsFederateRegisterSourceFilter(fFed, helics_delay_filter, "port1", "filter1");
    BOOST_CHECK(f1 != NULL);
    CE(helicsFilterSet(f1, "delay", 2.5));

    CE(helicsFederateEnterExecutionModeAsync(fFed));
    CE(helicsFederateEnterExecutionMode(mFed));
    CE(helicsFederateEnterExecutionModeComplete(fFed));

    federate_state state;
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_execution_state);
    std::string data(500, 'a');
    CE(helicsEndpointSendMessageRaw(p1, "port2", data.c_str(), static_cast<int>(data.size())));

    helics_time_t timeOut;
    CE(helicsFederateRequestTimeAsync(mFed, 1.0));
    CE(helicsFederateRequestTime(fFed, 1.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));

    auto res = helicsFederateHasMessage(mFed);
    BOOST_CHECK(!res);

    CE(helicsFederateRequestTimeAsync(mFed, 2.0));
    CE(helicsFederateRequestTime(fFed, 2.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));
    BOOST_REQUIRE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0));
    CE(helicsFederateRequestTime(mFed, 3.0, &timeOut));

    BOOST_REQUIRE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    BOOST_CHECK_EQUAL(m2.source, "port1");
    BOOST_CHECK_EQUAL(m2.original_source, "port1");
    BOOST_CHECK_EQUAL(m2.dest, "port2");
    BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
    BOOST_CHECK_EQUAL(m2.time, 2.5);

    CE(helicsFederateRequestTime(mFed, 3.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(fFed, &timeOut));
    CE(helicsFederateFinalize(mFed));
    CE(helicsFederateFinalize(fFed));
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

/** test a filter operator
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

BOOST_TEST_DECORATOR(*utf::timeout(20))
BOOST_DATA_TEST_CASE(message_filter_function_two_stage, bdata::make(core_types), core_type)
{
    auto broker = AddBroker(core_type, 3);
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "filter2");
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto fFed2 = GetFederateAt(1);
    auto mFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "");
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "");

    auto f1 = helicsFederateRegisterSourceFilter(fFed, helics_delay_filter, "port1", "filter1");
    BOOST_CHECK(f1 != NULL);
    CE(helicsFilterSet(f1, "delay", 1.25));

    auto f2 = helicsFederateRegisterSourceFilter(fFed, helics_delay_filter, "port1", "filter2");
    BOOST_CHECK(f2 != NULL);
    CE(helicsFilterSet(f2, "delay", 1.25));

    CE(helicsFederateEnterExecutionModeAsync(fFed));
    CE(helicsFederateEnterExecutionModeAsync(fFed2));
    CE(helicsFederateEnterExecutionMode(mFed));
    CE(helicsFederateEnterExecutionModeComplete(fFed));
    CE(helicsFederateEnterExecutionModeComplete(fFed2));

    federate_state state;
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_execution_state);
    std::string data(500, 'a');
    CE(helicsEndpointSendMessageRaw(p1, "port2", data.c_str(), static_cast<int>(data.size())));

    helics_time_t timeOut;
    CE(helicsFederateRequestTimeAsync(mFed, .0));
    CE(helicsFederateRequestTimeAsync(fFed, 1.0));
    CE(helicsFederateRequestTime(fFed2, 1.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));
    CE(helicsFederateRequestTimeComplete(fFed, &timeOut));
    auto res = helicsFederateHasMessage(mFed);
    BOOST_CHECK(!res);

    CE(helicsFederateRequestTimeAsync(mFed, .0));
    CE(helicsFederateRequestTimeAsync(fFed2, 2.0));
    CE(helicsFederateRequestTime(fFed, 2.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));
    CE(helicsFederateRequestTimeComplete(fFed2, &timeOut));
    BOOST_REQUIRE(!helicsEndpointHasMessage(p2));

    CE(helicsFederateRequestTimeAsync(fFed, 3.0));
    CE(helicsFederateRequestTimeAsync(fFed2, 3.0));
    CE(helicsFederateRequestTime(mFed, 3.0, &timeOut));
    if (!helicsEndpointHasMessage(p2)) {
        printf("missing message\n");
    }
    BOOST_REQUIRE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    BOOST_CHECK_EQUAL(m2.source, "port1");
    BOOST_CHECK_EQUAL(m2.original_source, "port1");
    BOOST_CHECK_EQUAL(m2.dest, "port2");
    BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
    BOOST_CHECK_EQUAL(m2.time, 2.5);

    CE(helicsFederateRequestTimeComplete(fFed, &timeOut));
    CE(helicsFederateRequestTimeComplete(fFed2, &timeOut));
    CE(helicsFederateFinalize(mFed));
    CE(helicsFederateFinalize(fFed));
    CE(helicsFederateFinalize(fFed2));
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

/** test two filter operators
The filter operator delays the message by 2.5 seconds meaning it should arrive by 3 sec into the
simulation
*/

BOOST_TEST_DECORATOR(*utf::timeout(12))
BOOST_DATA_TEST_CASE(message_filter_function2, bdata::make(core_types), core_type)
{
    auto broker = AddBroker(core_type, 2);
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "filter");
    AddFederates(helicsCreateMessageFederate, core_type, 1, broker, 1.0, "message");

    auto fFed = GetFederateAt(0);
    auto mFed = GetFederateAt(1);

    auto p1 = helicsFederateRegisterGlobalEndpoint(mFed, "port1", "");
    auto p2 = helicsFederateRegisterGlobalEndpoint(mFed, "port2", "");

    auto f1 = helicsFederateRegisterSourceFilter(fFed, helics_delay_filter, "port1", "filter1");
    BOOST_CHECK(f1 != NULL);
    CE(helicsFilterSet(f1, "delay", 2.5));

    auto f2 = helicsFederateRegisterSourceFilter(fFed, helics_delay_filter, "port2", "filter2");
    BOOST_CHECK(f2 != NULL);
    CE(helicsFilterSet(f2, "delay", 2.5));

    CE(helicsFederateEnterExecutionModeAsync(fFed));
    CE(helicsFederateEnterExecutionMode(mFed));
    CE(helicsFederateEnterExecutionModeComplete(fFed));

    federate_state state;
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_execution_state);
    std::string data(500, 'a');
    CE(helicsEndpointSendMessageRaw(p1, "port2", data.c_str(), static_cast<int>(data.size())));

    helics_time_t timeOut;
    CE(helicsFederateRequestTimeAsync(mFed, 1.0));
    CE(helicsFederateRequestTime(fFed, 1.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));

    auto res = helicsFederateHasMessage(mFed);
    BOOST_CHECK(!res);
    CE(helicsEndpointSendMessageRaw(p2, "port1", data.c_str(), static_cast<int>(data.size())));
    CE(helicsFederateRequestTimeAsync(mFed, 2.0));
    CE(helicsFederateRequestTime(fFed, 2.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(mFed, &timeOut));
    BOOST_REQUIRE(!helicsEndpointHasMessage(p2));
    // there may be something wrong here yet but this test isn't the one to find it and
    // this may prevent spurious errors for now.
    std::this_thread::yield();
    CE(helicsFederateRequestTime(mFed, 3.0, &timeOut));

    BOOST_REQUIRE(helicsEndpointHasMessage(p2));

    auto m2 = helicsEndpointGetMessage(p2);
    BOOST_CHECK_EQUAL(m2.source, "port1");
    BOOST_CHECK_EQUAL(m2.original_source, "port1");
    BOOST_CHECK_EQUAL(m2.dest, "port2");
    BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
    BOOST_CHECK_EQUAL(m2.time, 2.5);

    BOOST_CHECK(!helicsEndpointHasMessage(p1));
    CE(helicsFederateRequestTime(mFed, 4.0, &timeOut));
    BOOST_CHECK(helicsEndpointHasMessage(p1));
    CE(helicsFederateFinalize(mFed));
    CE(helicsFederateFinalize(fFed));
    CE(helicsFederateGetState(fFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_AUTO_TEST_CASE(message_clone_test)
{
    auto broker = AddBroker("test", 3);
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto dFed = GetFederateAt(1);
    auto dcFed = GetFederateAt(2);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "");
    auto p2 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "");
    auto p3 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "");

    auto f1 = helicsFederateRegisterCloningFilter(dcFed, "cm");
    CE(helicsFilterAddSourceTarget(f1, "src"));

    CE(helicsFederateEnterExecutionModeAsync(sFed));
    CE(helicsFederateEnterExecutionModeAsync(dcFed));
    CE(helicsFederateEnterExecutionMode(dFed));
    CE(helicsFederateEnterExecutionModeComplete(sFed));
    CE(helicsFederateEnterExecutionModeComplete(dcFed));

    federate_state state;
    CE(helicsFederateGetState(sFed, &state));
    BOOST_CHECK(state == helics_execution_state);
    std::string data(500, 'a');
    CE(helicsEndpointSendMessageRaw(p1, "dest", data.c_str(), static_cast<int>(data.size())));

    helics_time_t timeOut;
    CE(helicsFederateRequestTimeAsync(sFed, 1.0));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0));
    CE(helicsFederateRequestTime(dFed, 1.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(sFed, &timeOut));
    CE(helicsFederateRequestTimeComplete(dcFed, &timeOut));

    auto res = helicsFederateHasMessage(dFed);
    BOOST_CHECK(res);

    if (res) {
        auto m2 = helicsEndpointGetMessage(p2);
        BOOST_CHECK_EQUAL(m2.source, "src");
        BOOST_CHECK_EQUAL(m2.original_source, "src");
        BOOST_CHECK_EQUAL(m2.dest, "dest");
        BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
    }

    // now check the message clone
    res = helicsFederateHasMessage(dcFed);
    BOOST_CHECK(res);

    if (res) {
        auto m2 = helicsEndpointGetMessage(p3);
        BOOST_CHECK_EQUAL(m2.source, "src");
        BOOST_CHECK_EQUAL(m2.original_source, "src");
        BOOST_CHECK_EQUAL(m2.dest, "cm");
        BOOST_CHECK_EQUAL(m2.original_dest, "dest");
        BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
    }

    CE(helicsFederateFinalize(sFed));
    CE(helicsFederateFinalize(dFed));
    CE(helicsFederateFinalize(dcFed));
    CE(helicsFederateGetState(sFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_AUTO_TEST_CASE(message_multi_clone_test)
{
    auto broker = AddBroker("test", 4);
    AddFederates(helicsCreateMessageFederate, "test", 2, broker, 1.0, "source");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest");
    AddFederates(helicsCreateMessageFederate, "test", 1, broker, 1.0, "dest_clone");

    auto sFed = GetFederateAt(0);
    auto sFed2 = GetFederateAt(1);
    auto dFed = GetFederateAt(2);
    auto dcFed = GetFederateAt(3);

    auto p1 = helicsFederateRegisterGlobalEndpoint(sFed, "src", "");
    auto p2 = helicsFederateRegisterGlobalEndpoint(sFed2, "src2", "");
    auto p3 = helicsFederateRegisterGlobalEndpoint(dFed, "dest", "");
    auto p4 = helicsFederateRegisterGlobalEndpoint(dcFed, "cm", "");

    auto f1 = helicsFederateRegisterCloningFilter(dcFed, "cm");
    CE(helicsFilterAddSourceTarget(f1, "src"));
    CE(helicsFilterAddSourceTarget(f1, "src2"));

    CE(helicsFederateEnterExecutionModeAsync(sFed));
    CE(helicsFederateEnterExecutionModeAsync(sFed2));
    CE(helicsFederateEnterExecutionModeAsync(dcFed));
    CE(helicsFederateEnterExecutionMode(dFed));
    CE(helicsFederateEnterExecutionModeComplete(sFed));
    CE(helicsFederateEnterExecutionModeComplete(sFed2));
    CE(helicsFederateEnterExecutionModeComplete(dcFed));

    federate_state state;
    CE(helicsFederateGetState(sFed, &state));
    BOOST_CHECK(state == helics_execution_state);
    std::string data(500, 'a');
    std::string data2(400, 'b');
    CE(helicsEndpointSendMessageRaw(p1, "dest", data.c_str(), static_cast<int>(data.size())));
    CE(helicsEndpointSendMessageRaw(p2, "dest", data2.c_str(), static_cast<int>(data2.size())));

    helics_time_t timeOut;
    CE(helicsFederateRequestTimeAsync(sFed, 1.0));
    CE(helicsFederateRequestTimeAsync(sFed2, 1.0));
    CE(helicsFederateRequestTimeAsync(dcFed, 1.0));
    CE(helicsFederateRequestTime(dFed, 1.0, &timeOut));
    CE(helicsFederateRequestTimeComplete(sFed, &timeOut));
    CE(helicsFederateRequestTimeComplete(sFed2, &timeOut));
    CE(helicsFederateRequestTimeComplete(dcFed, &timeOut));

    auto mcnt = helicsEndpointReceiveCount(p3);
    BOOST_CHECK_EQUAL(mcnt, 2);
    auto res = helicsFederateHasMessage(dFed);
    BOOST_CHECK(res);

    if (res) {
        auto m2 = helicsEndpointGetMessage(p3);
        BOOST_CHECK_EQUAL(m2.source, "src");
        BOOST_CHECK_EQUAL(m2.original_source, "src");
        BOOST_CHECK_EQUAL(m2.dest, "dest");
        BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dFed);
        BOOST_CHECK(res);

        if (res) {
            m2 = helicsFederateGetMessage(dFed);
            BOOST_CHECK_EQUAL(m2.source, "src2");
            BOOST_CHECK_EQUAL(m2.original_source, "src2");
            BOOST_CHECK_EQUAL(m2.dest, "dest");
            BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data2.size()));
        }
    }

    // now check the message clone
    mcnt = helicsEndpointReceiveCount(p4);
    BOOST_CHECK_EQUAL(mcnt, 2);
    res = helicsFederateHasMessage(dcFed);
    BOOST_CHECK(res);

    if (res) {
        auto m2 = helicsFederateGetMessage(dcFed);
        BOOST_CHECK_EQUAL(m2.source, "src");
        BOOST_CHECK_EQUAL(m2.original_source, "src");
        BOOST_CHECK_EQUAL(m2.dest, "cm");
        BOOST_CHECK_EQUAL(m2.original_dest, "dest");
        BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data.size()));
        res = helicsFederateHasMessage(dcFed);
        BOOST_CHECK(res);

        if (res) {
            m2 = helicsFederateGetMessage(dcFed);
            BOOST_CHECK_EQUAL(m2.source, "src2");
            BOOST_CHECK_EQUAL(m2.original_source, "src2");
            BOOST_CHECK_EQUAL(m2.dest, "cm");
            BOOST_CHECK_EQUAL(m2.original_dest, "dest");
            BOOST_CHECK_EQUAL(m2.length, static_cast<int64_t>(data2.size()));
        }
    }

    CE(helicsFederateFinalize(sFed));
    CE(helicsFederateFinalize(sFed2));
    CE(helicsFederateFinalize(dFed));
    CE(helicsFederateFinalize(dcFed));
    CE(helicsFederateGetState(sFed, &state));
    BOOST_CHECK(state == helics_finalize_state);
}

BOOST_AUTO_TEST_CASE(test_file_load)
{
    std::string filename = std::string(TEST_DIR) + "/test_files/example_filters.json";
    auto mFed = helicsCreateMessageFederateFromJson(filename.c_str());

    char name[HELICS_SIZE_MAX];
    CE(helicsFederateGetName(mFed, name, HELICS_SIZE_MAX));
    BOOST_CHECK_EQUAL(name, "filterFed");

    BOOST_CHECK_EQUAL(helicsFederateGetEndpointCount(mFed), 3);
    helicsFederateFinalize(mFed);
    helicsFederateFree(mFed);
    // auto id = mFed.getEndpointId ("ept1");
    // BOOST_CHECK_EQUAL (mFed.getEndpointType (id), "genmessage");

    // BOOST_CHECK_EQUAL (mFed.filterObjectCount (), 3);

    // auto filt = mFed.getFilterObject (2);

    // auto cloneFilt = std::dynamic_pointer_cast<helics::CloningFilter> (filt);
    // BOOST_CHECK (cloneFilt);
    // mFed.disconnect ();
}
BOOST_AUTO_TEST_SUITE_END()
