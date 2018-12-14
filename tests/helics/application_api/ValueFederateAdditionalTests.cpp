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
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (value_federate_additional_tests, FederateTestFixture)

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (value_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

// BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (core_types_single), core_type)
BOOST_DATA_TEST_CASE (value_federate_publication_registration, bdata::make (ztypes), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "double", "V");
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    auto sv = vFed1->getPublicationKey (pubid);
    auto sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");

    BOOST_CHECK (vFed1->getPublication ("pub1").getHandle () == pubid.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("pub2").getHandle () == pubid2.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("fed0/pub1").getHandle () == pubid.getHandle ());
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_publisher_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    helics::Publication pubid (vFed1.get (), "pub1", helics::helicsType<std::string> ());
    helics::PublicationT<int> pubid2 (helics::GLOBAL, vFed1.get (), "pub2");

    helics::Publication pubid3 (vFed1.get (), "pub3", helics::helicsType<double> (), "V");
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    auto sv = pubid.getKey ();
    auto sv2 = pubid2.getKey ();
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto pub3name = pubid3.getKey ();
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (pubid3.getType (), "double");
    BOOST_CHECK_EQUAL (pubid3.getUnits (), "V");

    BOOST_CHECK (vFed1->getPublication ("pub1").getHandle () == pubid.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("pub2").getHandle () == pubid2.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("fed0/pub1").getHandle () == pubid.getHandle ());
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (value_federate_subscription_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto &subid = vFed1->registerSubscription ("sub1", "V");
    auto &subid2 = vFed1->registerSubscription ("sub2");

    auto &subid3 = vFed1->registerSubscription ("sub3", "V");
    vFed1->enterExecutingMode ();

    // BOOST_CHECK (vFed->getCurrentMode () == helics::Federate::modes::executing);

    auto &sv = vFed1->getTarget (subid);
    auto &sv2 = vFed1->getTarget (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto &sub3name = vFed1->getTarget (subid3);

    vFed1->addShortcut (subid, "Shortcut");
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getInputUnits (subid3), "V");

    BOOST_CHECK (vFed1->getSubscription ("sub1").getHandle () == subid.getHandle ());
    BOOST_CHECK (vFed1->getSubscription ("sub2").getHandle () == subid2.getHandle ());

    BOOST_CHECK (vFed1->getSubscription ("Shortcut").getHandle () == subid.getHandle ());

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
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

    // optional
    auto subid = vFed1->registerSubscription ("sub1", "V");
    auto subid2 = vFed1->registerSubscription ("sub2");

    auto subid3 = vFed1->registerSubscription ("sub3", "V");
    // enter execution
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);
    // check subscriptions
    auto sv = vFed1->getTarget (subid);
    auto sv2 = vFed1->getTarget (subid2);
    BOOST_CHECK_EQUAL (sv, "sub1");
    BOOST_CHECK_EQUAL (sv2, "sub2");
    auto sub3name = vFed1->getTarget (subid3);
    BOOST_CHECK_EQUAL (sub3name, "sub3");

    BOOST_CHECK_EQUAL (vFed1->getInputUnits (subid3), "V");

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

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary ();
}

BOOST_DATA_TEST_CASE (value_federate_single_transfer, bdata::make (core_types_single), core_type)
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
    // advance time
    gtime = vFed1->requestTime (2.0);
    // make sure the value was updated
    BOOST_CHECK_EQUAL (gtime, 2.0);
    s = vFed1->getString (subid);

    BOOST_CHECK_EQUAL (s, "string2");
}


BOOST_DATA_TEST_CASE (value_federate_dual_transfer_string, bdata::make (core_types_all), core_type)
{
    // this one is going to test really ugly strings
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data test case
    decltype (auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString (cstr, sizeof (cstr));
    runDualFederateTest<std::string> (core_type, std::string (86263, '\0'), specialString, std::string ());
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_vector, bdata::make (core_types), core_type)
{
    std::vector<double> defVec = {34.3, 24.2};
    std::vector<double> v1Vec = {12.4, 14.7, 16.34, 18.17};
    std::vector<double> v2Vec = {9.9999, 8.8888, 7.7777};
    runDualFederateTestv2<std::vector<double>> (core_type, defVec, v1Vec, v2Vec);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_complex, bdata::make (core_types), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> (core_type, def, v1, v2);
}

BOOST_AUTO_TEST_CASE (value_federate_dual_transfer_complex_long)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>> ("test_7", def, v1, v2);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj8, bdata::make (core_types), core_type)
{
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data test case
    decltype (auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString (cstr, sizeof (cstr));

    runDualFederateTestObj<std::string> (core_type, std::string (86263, '\0'), specialString, std::string ());
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj9, bdata::make (core_types_all), core_type)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar (10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTestObj<std::complex<double>> (core_type, def, v1, v2);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj10, bdata::make (core_types), core_type)
{
    helics::named_point def{"trigger", 0.7};
    helics::named_point v1{"response", -1e-12};
    helics::named_point v2{"variance", 45.23};
    runDualFederateTestObjv2<helics::named_point> (core_type, def, v1, v2);
}

BOOST_DATA_TEST_CASE (value_federate_dual_transfer_types_obj11, bdata::make (core_types), core_type)
{
    runDualFederateTestObj<bool> (core_type, true, false, true);
}

/** test the callback specification with a vector list*/

BOOST_DATA_TEST_CASE (test_vector_callback_lists, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto pubid3 = vFed1->registerPublication ("pub3", "");

    auto sub1 = vFed1->registerSubscription ("fed0/pub1", "");
    auto sub2 = vFed1->registerSubscription ("pub2", "");
    auto sub3 = vFed1->registerSubscription ("fed0/pub3", "");

    helics::data_block db (547, ';');
    int ccnt = 0;
    // set subscriptions 1 and 2 to have callbacks
    vFed1->setInputNotificationCallback (sub1, [&](helics::Input &, helics::Time) { ++ccnt; });
    vFed1->setInputNotificationCallback (sub2, [&](helics::Input &, helics::Time) { ++ccnt; });
    vFed1->enterExecutingMode ();
    vFed1->publishRaw (pubid3, db);
    vFed1->requestTime (1.0);
    // callbacks here
    BOOST_CHECK_EQUAL (ccnt, 0);

    vFed1->publish (pubid1, "this is a test");
    vFed1->requestTime (3.0);
    BOOST_CHECK_EQUAL (ccnt, 1);

    ccnt = 0;  // reset the counter
    vFed1->publishRaw (pubid3, db);
    vFed1->publish (pubid2, 4);
    vFed1->publish (pubid1, "test string2");
    vFed1->requestTime (5.0);
    BOOST_CHECK_EQUAL (ccnt, 2);

    BOOST_CHECK_CLOSE (static_cast<double> (vFed1->getLastUpdateTime (sub3)), 3.0, 0.000001);
    vFed1->finalize ();
}

/** test the publish/subscribe to a vectorized array*/

BOOST_DATA_TEST_CASE (test_indexed_pubs_subs, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublicationIndexed<double> ("pub1", 0);
    auto pubid2 = vFed1->registerPublicationIndexed<double> ("pub1", 1);

    auto pubid3 = vFed1->registerPublicationIndexed<double> ("pub1", 2);

    auto sub1 = vFed1->registerSubscriptionIndexed ("pub1", 0);
    auto sub2 = vFed1->registerSubscriptionIndexed ("pub1", 1);
    auto sub3 = vFed1->registerSubscriptionIndexed ("pub1", 2);
    vFed1->enterExecutingMode ();

    vFed1->publish (pubid1, 10.0);
    vFed1->publish (pubid2, 20.0);
    vFed1->publish (pubid3, 30.0);
    vFed1->requestTime (2.0);
    auto v1 = vFed1->getDouble (sub1);
    auto v2 = vFed1->getDouble (sub2);
    auto v3 = vFed1->getDouble (sub3);

    BOOST_CHECK_CLOSE (10.0, v1, 0.00000001);
    BOOST_CHECK_CLOSE (20.0, v2, 0.00000001);
    BOOST_CHECK_CLOSE (30.0, v3, 0.00000001);
}

/** test the publish/subscribe to a vectorized array*/

BOOST_DATA_TEST_CASE (test_async_calls, bdata::make (core_types), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    auto vFed2 = GetFederateAs<helics::ValueFederate> (1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto subid = vFed2->registerSubscription ("pub1");
    vFed1->setProperty (helics_property_time_delta, 1.0);
    vFed2->setProperty (helics_property_time_delta, 1.0);

    vFed1->enterExecutingModeAsync ();
    BOOST_CHECK (!vFed1->isAsyncOperationCompleted ());
    vFed2->enterExecutingModeAsync ();
    vFed1->enterExecutingModeComplete ();
    vFed2->enterExecutingModeComplete ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    vFed1->requestTimeAsync (1.0);
    vFed2->requestTimeAsync (1.0);

    auto f1time = vFed1->requestTimeComplete ();
    auto gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time, 1.0);
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
    vFed1->requestTimeAsync (2.0);
    vFed2->requestTimeAsync (2.0);
    f1time = vFed1->requestTimeComplete ();
    gtime = vFed2->requestTimeComplete ();

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time, 2.0);

    // make sure the value was updated

    s = subid.getValue<std::string> ();
    BOOST_CHECK_EQUAL (s, "string2");
    vFed1->finalizeAsync ();
    vFed2->finalize ();
    vFed1->finalizeComplete ();
}

/** test info field for multiple publications */
BOOST_DATA_TEST_CASE (test_info_field, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublication<std::string> ("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int> ("pub2");
    pubid1.setInfo (std::string ("test1"));
    pubid2.setInfo (std::string ("test2"));
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    auto info1 = vFed1->getInfo (pubid1.getHandle ());
    auto info2 = vFed1->getInfo (pubid2.getHandle ());
    BOOST_CHECK_EQUAL (info1, "test1");
    BOOST_CHECK_EQUAL (info2, "test2");

    vFed1->finalize ();
}

/** test the pub/sub info field*/
BOOST_DATA_TEST_CASE (test_info_pubs_subs, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto pubid1 = vFed1->registerPublicationIndexed<double> ("pub1", 0);
    pubid1.setInfo (std::string ("pub_test1"));

    auto sub1 = vFed1->registerSubscriptionIndexed ("pub1", 0);
    auto sub2 = vFed1->registerSubscriptionIndexed ("pub1", 1);
    auto sub3 = vFed1->registerSubscriptionIndexed ("pub1", 2);

    sub1.setInfo (std::string ("sub_test1"));
    sub2.setInfo (std::string ("sub_test2"));
    sub3.setInfo (std::string ("sub_test3"));

    vFed1->enterExecutingMode ();

    // Check all values can be accessed and returned through the federate.
    auto info1 = vFed1->getInfo (pubid1.getHandle ());
    auto info2 = vFed1->getInfo (sub1.getHandle ());
    auto info3 = vFed1->getInfo (sub2.getHandle ());
    auto info4 = vFed1->getInfo (sub3.getHandle ());

    BOOST_CHECK_EQUAL (info1, "pub_test1");
    BOOST_CHECK_EQUAL (info2, "sub_test1");
    BOOST_CHECK_EQUAL (info3, "sub_test2");
    BOOST_CHECK_EQUAL (info4, "sub_test3");

    // Check all values can be accessed and returned directly from their subscriptions.
    auto sub_info2 = sub1.getInfo ();
    auto sub_info3 = sub2.getInfo ();
    auto sub_info4 = sub3.getInfo ();

    BOOST_CHECK_EQUAL (sub_info2, "sub_test1");
    BOOST_CHECK_EQUAL (sub_info3, "sub_test2");
    BOOST_CHECK_EQUAL (sub_info4, "sub_test3");

    vFed1->finalize ();
}

/** test the default constructor and move constructor and move assignment*/
BOOST_AUTO_TEST_CASE (test_move_calls)
{
    helics::ValueFederate vFed;

    helics::FederateInfo fi (helics::core_type::TEST);
    fi.coreInitString = "-f 3 --autobroker";
    vFed = helics::ValueFederate ("test1", fi);
    BOOST_CHECK_EQUAL (vFed.getName (), "test1");

    helics::ValueFederate vFedMoved (std::move (vFed));
    BOOST_CHECK_EQUAL (vFedMoved.getName (), "test1");
    // verify that this was moved so this does produce a warning on some systems about use after move
    BOOST_CHECK_NE (vFed.getName (), "test1");
}

BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics::ValueFederate vFed (std::string (TEST_DIR) + "/example_value_fed.json");

    BOOST_CHECK_EQUAL (vFed.getName (), "valueFed");

    BOOST_CHECK_EQUAL (vFed.getInputCount (), 2);
    BOOST_CHECK_EQUAL (vFed.getPublicationCount (), 2);
    auto &id = vFed.getInput ("pubshortcut");

    auto key = vFed.getTarget (id);
    BOOST_CHECK_EQUAL (key, "fedName/pub2");

    BOOST_CHECK_EQUAL (id.getInfo (), "this is an information string for use by the application");
    auto pub2name = vFed.getPublicationKey (vFed.getPublication (1));
    BOOST_CHECK_EQUAL (key, "fedName/pub2");
    // test the info from a file
    BOOST_CHECK_EQUAL (vFed.getPublication (0).getInfo (),
                       "this is an information string for use by the application");

    BOOST_CHECK_EQUAL (vFed.query ("global", "global1"), "this is a global1 value");
    BOOST_CHECK_EQUAL (vFed.query ("global", "global2"), "this is another global value");
    vFed.disconnect ();
}

BOOST_AUTO_TEST_CASE (test_file_load_toml)
{
    helics::ValueFederate vFed (std::string (TEST_DIR) + "/example_value_fed.toml");

    BOOST_CHECK_EQUAL (vFed.getName (), "valueFed");

    BOOST_CHECK_EQUAL (vFed.getInputCount (), 2);
    BOOST_CHECK_EQUAL (vFed.getPublicationCount (), 2);

    auto id = vFed.getInput ("pubshortcut");
    auto key = vFed.getTarget (id);
    BOOST_CHECK_EQUAL (key, "fedName:pub2");

    BOOST_CHECK_EQUAL (id.getInfo (), "this is an information string for use by the application");

    auto pub2name = vFed.getPublicationKey (vFed.getPublication (1));
    BOOST_CHECK_EQUAL (key, "fedName:pub2");

    // test the info from a file
    BOOST_CHECK_EQUAL (vFed.getPublication (0).getInfo (),
                       "this is an information string for use by the application");
    BOOST_CHECK_EQUAL (vFed.query ("global", "global1"), "this is a global1 value");
    BOOST_CHECK_EQUAL (vFed.query ("global", "global2"), "this is another global value");
    vFed.disconnect ();
}
BOOST_AUTO_TEST_SUITE_END ()
