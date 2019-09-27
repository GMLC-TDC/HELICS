/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

#include <future>

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"

/** these test cases test out the value federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_key_tests, FederateTestFixture, *utf::label ("ci"))

BOOST_DATA_TEST_CASE (value_federate_subscriber_and_publisher_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
    using namespace helics;
    SetupTest<ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<ValueFederate> (0);

    vFed1->setFlagOption (helics_handle_option_connection_optional);

    // register the publications
    Publication pubid (vFed1.get (), "pub1", helicsType<std::string> ());
    PublicationT<int> pubid2 (GLOBAL, vFed1, "pub2");

    Publication pubid3 (vFed1, "pub3", helicsType<double> (), "V");

    // these aren't meant to match the publications
    auto &subid1 = make_subscription (*vFed1, "sub1");

    auto subid2 = make_subscription<int> (*vFed1, "sub2");

    auto &subid3 = make_subscription (*vFed1, "sub3", "V");
    // enter execution
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == Federate::modes::executing);
    // check subscriptions
    const auto &sv = subid1.getTarget ();
    const auto &sv2 = subid2.getTarget ();
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    const auto &sub3name = subid3.getTarget ();
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK (subid1.getType ().empty ());  // def is the default type
    BOOST_CHECK_EQUAL (subid2.getType (), "int32");
    BOOST_CHECK (subid3.getType ().empty ());
    BOOST_CHECK_EQUAL (subid3.getUnits (), "V");

    // check publications

    auto pk = pubid.getKey ();
    auto pk2 = pubid2.getKey ();
    BOOST_CHECK_EQUAL (pk, "fed0/pub1");
    BOOST_CHECK_EQUAL (pk2, "pub2");
    auto pub3name = pubid3.getKey ();
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (pubid3.getType (), "double");
    BOOST_CHECK_EQUAL (pubid3.getUnits (), "V");
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_publisher, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    BOOST_REQUIRE (vFed1);
    // register the publications
    helics::Publication pubid (helics::GLOBAL, vFed1.get (), "pub1", helics::data_type::helics_string);

    auto &subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode ();
    // publish string1 at time=0.0;
    pubid.publish ("string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s;
    // get the value
    subid.getValue (s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    pubid.publish ("string2");
    // make sure the value is still what we expect
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
}

static bool dual_transfer_test (std::shared_ptr<helics::ValueFederate> &vFed1,
                                std::shared_ptr<helics::ValueFederate> &vFed2,
                                helics::Publication &pubid,
                                helics::Input &subid)
{
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    bool correct = true;

    auto f1finish = std::async (std::launch::async, [&] () { vFed1->enterExecutingMode (); });
    vFed2->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    if (gtime != 1.0)
    {
        correct = false;
    }
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 1.0);
    if (gtime != 1.0)
    {
        correct = false;
    }
    // get the value
    std::string s = vFed2->getString (subid);

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    if (s != "string1")
    {
        correct = false;
    }
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue (s);
    BOOST_CHECK_EQUAL (s, "string1");
    if (s != "string1")
    {
        correct = false;
    }
    // advance time
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    if (gtime != 2.0)
    {
        correct = false;
    }
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 2.0);
    if (gtime != 2.0)
    {
        correct = false;
    }
    // make sure the value was updated

    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");
    if (s != "string2")
    {
        correct = false;
    }
    vFed1->finalizeAsync ();
    vFed2->finalize ();
    vFed1->finalizeComplete ();
    return correct;
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed2->registerSubscription ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, subid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_inputs, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerInput<std::string> ("inp1");

    vFed2->addTarget (inpid, "pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_pubtarget, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    vFed1->addTarget (pubid, "inp1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_nameless_pub, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerPublication<std::string> ("");
    vFed1->addTarget (pubid, "inp1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_broker_link, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto &broker = brokers[0];
    broker->dataLink ("pub1", "inp1");
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_broker_link_late, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto &broker = brokers[0];

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    broker->dataLink ("pub1", "inp1");
    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_broker_link_direct, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto &broker = brokers[0];

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    broker->dataLink ("pub1", "inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

static constexpr const char *simple_connection_files[] = {"example_connections1.json",
                                                          "example_connections2.json",
                                                          "example_connections1.toml",
                                                          "example_connections2.toml",
                                                          "example_connections3.toml",
                                                          "example_connections4.toml"};

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_broker_link_file,
                      bdata::make (simple_connection_files),
                      file_name)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto &broker = brokers[0];

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    auto testFile = std::string (TEST_DIR) + file_name;
    broker->makeConnections (testFile);
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_AUTO_TEST_CASE (value_federate_dual_transfer_broker_link_json_string)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto &broker = brokers[0];

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    broker->makeConnections ("{\"connections\":[[\"pub1\", \"inp1\"]]}");

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();
    core->dataLink ("pub1", "inp1");
    core = nullptr;
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link_late, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    core->dataLink ("pub1", "inp1");
    core = nullptr;
    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link_late_switch, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    core->dataLink ("pub1", "inp1");
    core = nullptr;
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link_direct1, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    core->dataLink ("pub1", "inp1");
    core = nullptr;
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link_direct2, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed2->getCorePointer ();

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (200));
    core->dataLink ("pub1", "inp1");
    core = nullptr;
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_core_link_file,
                      bdata::make (simple_connection_files),
                      file_name)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    auto testFile = std::string (TEST_DIR) + file_name;
    core->makeConnections (testFile);
    core = nullptr;
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_AUTO_TEST_CASE (value_federate_dual_transfer_core_link_json_string)
{
    SetupTest<helics::ValueFederate> ("test", 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    auto core = vFed1->getCorePointer ();

    auto &inpid = vFed2->registerGlobalInput<std::string> ("inp1");
    std::this_thread::sleep_for (std::chrono::milliseconds (50));
    core->makeConnections ("{\"connections\":[[\"pub1\", \"inp1\"]]}");
    core = nullptr;
    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");
    bool res = dual_transfer_test (vFed1, vFed2, pubid, inpid);
    BOOST_CHECK (res);
}

BOOST_DATA_TEST_CASE (value_federate_single_init_publish, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<double> ("pub1");

    auto &subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterInitializingMode ();
    vFed1->publish (pubid, 1.0);

    vFed1->enterExecutingMode ();
    // get the value set at initialization
    double val = vFed1->getDouble (subid);

    BOOST_CHECK_EQUAL (val, 1.0);
    // publish string1 at time=0.0;
    vFed1->publish (pubid, 2.0);
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);

    // get the value
    subid.getValue (val);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (val, 2.0);
    // publish a second string
    vFed1->publish (pubid, 3.0);
    // make sure the value is still what we expect
    val = vFed1->getDouble (subid);

    BOOST_CHECK_EQUAL (val, 2.0);
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    subid.getValue (val);
    BOOST_CHECK_EQUAL (val, 3.0);
    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (test_block_send_receive, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->registerPublication<std::string> ("pub1");
    vFed1->registerGlobalPublication<int> ("pub2");

    auto &pubid3 = vFed1->registerPublication ("pub3", "");

    auto &sub1 = vFed1->registerSubscription ("fed0/pub3", "");

    helics::data_block db (547, ';');

    vFed1->enterExecutingMode ();
    vFed1->publishRaw (pubid3, db);
    vFed1->requestTime (1.0);
    BOOST_CHECK (vFed1->isUpdated (sub1));
    auto res = vFed1->getValueRaw (sub1);
    BOOST_CHECK_EQUAL (res.size (), db.size ());
    BOOST_CHECK (vFed1->isUpdated (sub1) == false);
}

/** test the all callback*/

BOOST_DATA_TEST_CASE (test_all_callback, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto &pubid1 = vFed1->registerPublication<std::string> ("pub1");
    auto &pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto &pubid3 = vFed1->registerPublication ("pub3", "");

    auto &sub1 = vFed1->registerSubscription ("fed0/pub1", "");
    auto &sub2 = vFed1->registerSubscription ("pub2", "");
    auto &sub3 = vFed1->registerSubscription ("fed0/pub3", "");

    helics::data_block db (547, ';');
    helics::interface_handle lastId;
    helics::Time lastTime;
    vFed1->setInputNotificationCallback ([&] (const helics::Input &subid, helics::Time callTime) {
        lastTime = callTime;
        lastId = subid.getHandle ();
    });
    vFed1->enterExecutingMode ();
    vFed1->publishRaw (pubid3, db);
    vFed1->requestTime (1.0);
    // the callback should have occurred here
    BOOST_CHECK (lastId == sub3.getHandle ());
    if (lastId == sub3.getHandle ())
    {
        BOOST_CHECK_EQUAL (lastTime, 1.0);
        BOOST_CHECK_EQUAL (vFed1->getLastUpdateTime (sub3), lastTime);
    }
    else
    {
        BOOST_FAIL (" missed callback\n");
    }

    vFed1->publish (pubid2, 4);
    vFed1->requestTime (2.0);
    // the callback should have occurred here
    BOOST_CHECK (lastId == sub2.getHandle ());
    BOOST_CHECK_EQUAL (lastTime, 2.0);
    vFed1->publish (pubid1, "this is a test");
    vFed1->requestTime (3.0);
    // the callback should have occurred here
    BOOST_CHECK (lastId == sub1.getHandle ());
    BOOST_CHECK_EQUAL (lastTime, 3.0);

    int ccnt = 0;
    vFed1->setInputNotificationCallback ([&] (const helics::Input &, helics::Time) { ++ccnt; });

    vFed1->publishRaw (pubid3, db);
    vFed1->publish (pubid2, 4);
    vFed1->requestTime (4.0);
    // the callback should have occurred here
    BOOST_CHECK_EQUAL (ccnt, 2);
    ccnt = 0;  // reset the counter
    vFed1->publishRaw (pubid3, db);
    vFed1->publish (pubid2, 4);
    vFed1->publish (pubid1, "test string2");
    vFed1->requestTime (5.0);
    // the callback should have occurred here
    BOOST_CHECK_EQUAL (ccnt, 3);
    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_close, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s = vFed1->getString (subid);
    // get the value
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    s = vFed1->getString (subid);
    BOOST_CHECK_EQUAL (s, "string1");

    vFed1->closeInterface (pubid.getHandle ());
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    s = vFed1->getString (subid);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->publish (pubid, "string3");
    // make sure the value is still what we expect

    // advance time
    gtime = vFed1->requestTime (3.0);
    s = vFed1->getString (subid);
    // make sure we didn't get the last publish
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer_remove_target, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed1->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s = vFed1->getString (subid);
    // get the value
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    s = vFed1->getString (subid);
    BOOST_CHECK_EQUAL (s, "string1");

    subid.removeTarget ("pub1");
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    s = vFed1->getString (subid);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->publish (pubid, "string3");
    // make sure the value is still what we expect

    // advance time
    gtime = vFed1->requestTime (3.0);
    s = vFed1->getString (subid);
    // make sure we didn't get the last publish
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_close, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed2->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&] () { vFed1->enterExecutingMode (); });
    vFed2->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    std::string s = vFed2->getString (subid);

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue (s);
    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    vFed1->closeInterface (pubid.getHandle ());
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 2.0);
    // make sure the value was updated

    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");

    vFed1->publish (pubid, "string3");
    // make sure the value is still what we expect

    // advance time
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (3.0); });
    gtime = vFed2->requestTime (3.0);
    s = vFed2->getString (subid);
    // make sure we didn't get the last publish
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
    vFed2->finalize ();
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_remove_target, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed2->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&] () { vFed1->enterExecutingMode (); });
    vFed2->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    std::string s = vFed2->getString (subid);

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue (s);
    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    subid.removeTarget ("pub1");
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 2.0);
    // make sure the value was updated

    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");

    // so in theory the remove target could take a little while since it needs to route through the core on
    // occasion
    // and this is an asynchronous operation so there is no guarantees the remove will stop the next broadcast
    // but it should do it within the next timestep so we have an extra loop here
    f1time = std::async (std::launch::async, [&] () {
        vFed1->requestTime (3.0);
        return vFed1->requestTime (4.0);
    });
    gtime = vFed2->requestTime (3.0);
    gtime = vFed2->requestTime (4.0);
    BOOST_CHECK_EQUAL (gtime, 4.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 4.0);
    vFed1->publish (pubid, "string3");
    // make sure the value is still what we expect

    // advance time
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (5.0); });
    gtime = vFed2->requestTime (5.0);
    s = vFed2->getString (subid);
    // make sure we didn't get the last publish
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
    vFed2->finalize ();
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_remove_target_input, bdata::make (core_types_all), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed2->registerGlobalInput<std::string> ("sub1");
    pubid.addTarget ("sub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&] () { vFed1->enterExecutingMode (); });
    vFed2->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (1.0); });
    auto gtime = vFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    std::string s = vFed2->getString (subid);

    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue (s);
    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    pubid.removeTarget ("sub1");
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (2.0); });
    gtime = vFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    gtime = f1time.get ();
    BOOST_CHECK_EQUAL (gtime, 2.0);
    // make sure the value was updated

    subid.getValue (s);

    BOOST_CHECK_EQUAL (s, "string2");

    vFed1->publish (pubid, "string3");
    // make sure the value is still what we expect

    // advance time
    f1time = std::async (std::launch::async, [&] () { return vFed1->requestTime (3.0); });
    gtime = vFed2->requestTime (3.0);
    s = vFed2->getString (subid);
    // make sure we didn't get the last publish
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
    vFed2->finalize ();
}
BOOST_AUTO_TEST_SUITE_END ()
