/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <future>

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"

/** these test cases test out the value federates with some additional tests
 */

BOOST_FIXTURE_TEST_SUITE (value_federate_additional_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getPublicationKey (pubid);
    auto sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");

    BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid);
    BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2);
    BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid);
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publisher_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    helics::Publication pubid (vFed1.get (), "pub1", helics::helicsType<std::string> ());
    helics::PublicationT<int> pubid2 (helics::GLOBAL, vFed1.get (), "pub2");

    helics::Publication pubid3 (vFed1.get (), "pub3", helics::helicsType<double> (), "V");
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = pubid.getKey ();
    auto sv2 = pubid2.getKey ();
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = pubid3.getKey ();
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (pubid3.getType (), "double");
    BOOST_CHECK_EQUAL (pubid3.getUnits (), "V");

    BOOST_CHECK (vFed1->getPublicationId ("pub1") == pubid.getID ());
    BOOST_CHECK (vFed1->getPublicationId ("pub2") == pubid2.getID ());
    BOOST_CHECK (vFed1->getPublicationId ("fed0/pub1") == pubid.getID ());
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto subid = vFed1->registerRequiredSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerRequiredSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    vFed1->enterExecutionState ();

    // BOOST_CHECK (vFed->getCurrentState () == helics::Federate::op_states::execution);

    auto sv = vFed1->getSubscriptionKey (subid);
    auto sv2 = vFed1->getSubscriptionKey (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionKey (subid3);

    vFed1->addSubscriptionShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    BOOST_CHECK (vFed1->getSubscriptionId ("sub1") == subid);
    BOOST_CHECK (vFed1->getSubscriptionId ("sub2") == subid2);

    BOOST_CHECK (vFed1->getSubscriptionId ("Shortcut") == subid);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (value_federate_subscription_and_publication_registration,
                      bdata::make (core_types_single),
                      core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");

    auto subid = vFed1->registerOptionalSubscription ("sub1", "double", "V");
    auto subid2 = vFed1->registerOptionalSubscription<int> ("sub2");

    auto subid3 = vFed1->registerOptionalSubscription ("sub3", "double", "V");
    // enter execution
    vFed1->enterExecutionState ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::execution);
    // check subscriptions
    auto sv = vFed1->getSubscriptionKey (subid);
    auto sv2 = vFed1->getSubscriptionKey (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getSubscriptionKey (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getSubscriptionType (subid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getSubscriptionUnits (subid3), "V");

    // check publications

    sv = vFed1->getPublicationKey (pubid);
    sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentState () == helics::Federate::op_states::finalize);
    helics::cleanupHelicsLibrary ();
}

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed1->registerRequiredSubscription<std::string> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed1->enterExecutionState ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    std::string s;
    // get the value
    vFed1->getValue (subid, s);
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    vFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    vFed1->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string1");
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    vFed1->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
}

BOOST_TEST_DECORATOR (*utf::timeout (10))
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types8, bdata::make (core_types), core_type)
{
    // this one is going to test really ugly strings
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data test case
    decltype (auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString (cstr, sizeof (cstr));
    runDualFederateTest<std::string> (core_type, std::string (86263, '\0'), specialString, std::string ());
}

BOOST_TEST_DECORATOR (*utf::timeout (10))
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types9, bdata::make (core_types), core_type)
{
    std::vector<double> defVec = {34.3, 24.2};
    std::vector<double> v1Vec = {12.4, 14.7, 16.34, 18.17};
    std::vector<double> v2Vec = {9.9999, 8.8888, 7.7777};
    runDualFederateTestv2<std::vector<double>> (core_type, defVec, v1Vec, v2Vec);
}

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types10, bdata::make (core_types), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj8, bdata::make (core_types), core_type)
{
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data test case
    decltype (auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString (cstr, sizeof (cstr));

    runDualFederateTestObj<std::string> (core_type, std::string (86263, '\0'), specialString, std::string ());
}

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj9, bdata::make (core_types), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTestObj<std::complex<double>> (core_type, def, v1, v2);
}

/** test the callback specification with a vector list*/

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (test_vector_callback_lists, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "");

    auto sub1 = vFed1->registerOptionalSubscription ("fed0/pub1", "");
    auto sub2 = vFed1->registerOptionalSubscription ("pub2", "");
    auto sub3 = vFed1->registerOptionalSubscription ("fed0/pub3", "");

    helics::data_block db (547, ';');
    helics::subscription_id_t lastId;
    int ccnt = 0;
    // set subscriptions 1 and 2 to have callbacks
    vFed1->registerSubscriptionNotificationCallback ({sub1, sub2},
                                                     [&](helics::subscription_id_t, helics::Time) { ++ccnt; });
    vFed1->enterExecutionState ();
    vFed1->publish (pubid3, db);
    vFed1->requestTime (1.0);
    // callbacks here
    BOOST_CHECK_EQUAL (ccnt, 0);

    vFed1->publish (pubid1, "this is a test");
    vFed1->requestTime (3.0);
    BOOST_CHECK_EQUAL (ccnt, 1);

    ccnt = 0;  // reset the counter
    vFed1->publish (pubid3, db);
    vFed1->publish (pubid2, 4);
    vFed1->publish (pubid1, "test string2");
    vFed1->requestTime (5.0);
    BOOST_CHECK_EQUAL (ccnt, 2);

    BOOST_CHECK_CLOSE (static_cast<double> (vFed1->getLastUpdateTime (sub3)), 3.0, 0.000001);
    vFed1->finalize ();
}

/** test the publish/subscribe to a vectorized array*/

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (test_indexed_pubs_subs, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublicationIndexed<double> ("pub1", 0);
    auto pubid2 = vFed1->registerPublicationIndexed<double> ("pub1", 1);

    auto pubid3 = vFed1->registerPublicationIndexed<double> ("pub1", 2);

    auto sub1 = vFed1->registerOptionalSubscriptionIndexed<double> ("pub1", 0);
    auto sub2 = vFed1->registerOptionalSubscriptionIndexed<double> ("pub1", 1);
    auto sub3 = vFed1->registerOptionalSubscriptionIndexed<double> ("pub1", 2);
    vFed1->enterExecutionState ();

    vFed1->publish (pubid1, 10.0);
    vFed1->publish (pubid2, 20.0);
    vFed1->publish (pubid3, 30.0);
    vFed1->requestTime (2.0);
    auto v1 = vFed1->getValue<double> (sub1);
    auto v2 = vFed1->getValue<double> (sub2);
    auto v3 = vFed1->getValue<double> (sub3);

    BOOST_CHECK_CLOSE (10.0, v1, 0.00000001);
    BOOST_CHECK_CLOSE (20.0, v2, 0.00000001);
    BOOST_CHECK_CLOSE (30.0, v3, 0.00000001);
}

/** test the publish/subscribe to a vectorized array*/

BOOST_TEST_DECORATOR (*utf::timeout (12))
BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed2->registerRequiredSubscription<std::string> ("pub1");
    vFed1->setTimeDelta (1.0);
    vFed2->setTimeDelta (1.0);

    vFed1->enterExecutionStateAsync ();
    BOOST_CHECK (!vFed1->isAsyncOperationCompleted ());
    vFed2->enterExecutionStateAsync ();
    vFed1->enterExecutionStateComplete ();
    vFed2->enterExecutionStateComplete ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    vFed1->requestTimeAsync (1.0);
    vFed2->requestTimeAsync (1.0);

    auto f1time = vFed1->requestTimeComplete ();
    auto gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time, 1.0);
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
    vFed1->requestTimeAsync (2.0);
    vFed2->requestTimeAsync (2.0);
    f1time = vFed1->requestTimeComplete ();
    gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time, 2.0);

    // make sure the value was updated

    vFed2->getValue (subid, s);

    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalize ();
    vFed2->finalize ();
}

/** test the default constructor and move constructor and move assignment*/
BOOST_AUTO_TEST_CASE (test_move_calls)
{
    helics::ValueFederate vFed;

    helics::FederateInfo fi ("test1", helics::core_type::TEST);
    fi.coreInitString = "3";
    vFed = helics::ValueFederate (fi);
    BOOST_CHECK_EQUAL (vFed.getName (), "test1");

    helics::ValueFederate vFedMoved (std::move (vFed));
    BOOST_CHECK_EQUAL (vFedMoved.getName (), "test1");
    BOOST_CHECK_NE (vFed.getName (), "test1");
}

BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics::ValueFederate vFed (std::string (TEST_DIR) + "/test_files/example_value_fed.json");

    BOOST_CHECK_EQUAL (vFed.getName (), "valueFed");

    BOOST_CHECK_EQUAL (vFed.getSubscriptionCount (), 2);
    BOOST_CHECK_EQUAL (vFed.getPublicationCount (), 2);
    vFed.disconnect ();
}
BOOST_AUTO_TEST_SUITE_END ()
