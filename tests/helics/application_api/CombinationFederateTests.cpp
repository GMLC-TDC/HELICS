/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"
#include <future>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (combo_federate_tests, FederateTestFixture)

// const std::string core_types[] = {"udp" };
/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (combo_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (combo_federate_publication_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    auto &pubid = vFed1->registerPublication<std::string> ("pub1");
    auto &pubid2 = vFed1->registerGlobalPublication<int> ("pub2");

    auto &pubid3 = vFed1->registerPublication ("pub3", "double", "V");
    vFed1->enterExecutingMode ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::executing);

    auto &sv = vFed1->getPublicationKey (pubid);
    auto &sv2 = vFed1->getPublicationKey (pubid2);
    BOOST_CHECK_EQUAL (sv, "fed0/pub1");
    BOOST_CHECK_EQUAL (sv2, "pub2");
    auto &pub3name = vFed1->getPublicationKey (pubid3);
    BOOST_CHECK_EQUAL (pub3name, "fed0/pub3");

    BOOST_CHECK_EQUAL (vFed1->getPublicationType (pubid3), "double");
    BOOST_CHECK_EQUAL (vFed1->getPublicationUnits (pubid3), "V");

    BOOST_CHECK (vFed1->getPublication ("pub1").getHandle () == pubid.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("pub2").getHandle () == pubid2.getHandle ());
    BOOST_CHECK (vFed1->getPublication ("fed0/pub1").getHandle () == pubid.getHandle ());
    vFed1->finalize ();

    BOOST_CHECK (vFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (combo_federate_single_transfer, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);

    // register the publications
    auto &pubid = vFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = vFed1->registerSubscription ("pub1");
    vFed1->setTimeProperty (helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode ();
    // publish string1 at time=0.0;
    vFed1->publish (pubid, "string1");
    auto gtime = vFed1->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    // get the value
    std::string s = vFed1->getString (subid);
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

BOOST_DATA_TEST_CASE (combo_federate_endpoint_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    auto &epid = mFed1->registerEndpoint ("ep1");
    auto &epid2 = mFed1->registerGlobalEndpoint ("ep2", "random");

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);

    auto &sv = mFed1->getEndpointName (epid);
    auto &sv2 = mFed1->getEndpointName (epid2);
    BOOST_CHECK_EQUAL (sv, "fed0/ep1");
    BOOST_CHECK_EQUAL (sv2, "ep2");

    BOOST_CHECK_EQUAL (mFed1->getEndpointType (epid), "");
    BOOST_CHECK_EQUAL (mFed1->getEndpointType (epid2), "random");

    BOOST_CHECK (mFed1->getEndpoint ("ep1").getHandle () == epid.getHandle ());
    BOOST_CHECK (mFed1->getEndpoint ("fed0/ep1").getHandle () == epid.getHandle ());
    BOOST_CHECK (mFed1->getEndpoint ("ep2").getHandle () == epid2.getHandle ());
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (combination_federate_send_receive_2fed, bdata::make (core_types), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helics::CombinationFederate> (0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate> (1);

    auto &epid = mFed1->registerEndpoint ("ep1");
    auto &epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");

    mFed1->setTimeProperty (helics_property_time_delta, 1.0);
    mFed2->setTimeProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    mFed1->sendMessage (epid, "ep2", data);
    mFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = mFed1->hasMessage (epid);
    BOOST_CHECK (res);
    res = mFed2->hasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = mFed1->getMessage (epid);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage (epid2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);
    mFed1->finalize ();
    mFed2->finalize ();

    BOOST_CHECK (mFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (mFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_DATA_TEST_CASE (combination_federate_multimode_transfer, bdata::make (core_types), core_type)
{
    SetupTest<helics::CombinationFederate> (core_type, 2);
    auto cFed1 = GetFederateAs<helics::CombinationFederate> (0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate> (1);

    auto &epid = cFed1->registerEndpoint ("ep1");
    auto &epid2 = cFed2->registerGlobalEndpoint ("ep2", "random");

    // register the publications
    auto &pubid = cFed1->registerGlobalPublication<std::string> ("pub1");

    auto &subid = cFed2->registerSubscription ("pub1");

    cFed1->setTimeProperty (helics_property_time_delta, 1.0);
    cFed2->setTimeProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { cFed1->enterExecutingMode (); });
    cFed2->enterExecutingMode ();
    f1finish.wait ();
    // publish string1 at time=0.0;
    cFed1->publish (pubid, "string1");

    BOOST_CHECK (cFed1->getCurrentMode () == helics::Federate::modes::executing);
    BOOST_CHECK (cFed2->getCurrentMode () == helics::Federate::modes::executing);

    helics::data_block data (500, 'a');
    helics::data_block data2 (400, 'b');

    cFed1->sendMessage (epid, "ep2", data);
    cFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return cFed1->requestTime (1.0); });
    auto gtime = cFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    std::string s = cFed2->getString (subid);
    // get the value
    // make sure the string is what we expect
    BOOST_CHECK_EQUAL (s, "string1");
    // publish a second string
    cFed1->publish (pubid, "string2");
    // make sure the value is still what we expect
    s = cFed2->getString (subid);

    BOOST_CHECK_EQUAL (s, "string1");

    auto res = cFed1->hasMessage ();
    BOOST_CHECK (res);
    res = cFed1->hasMessage (epid);
    BOOST_CHECK (res);
    res = cFed2->hasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = cFed1->getMessage (epid);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data2[245]);

    auto M2 = cFed2->getMessage (epid2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M2->data[245], data[245]);

    // advance time
    f1time = std::async (std::launch::async, [&]() { return cFed1->requestTime (2.0); });
    gtime = cFed2->requestTime (2.0);

    BOOST_CHECK_EQUAL (gtime, 2.0);
    BOOST_CHECK_EQUAL (f1time.get (), 2.0);
    // make sure the value was updated

    auto &ns = cFed2->getString (subid);

    BOOST_CHECK_EQUAL (ns, "string2");

    cFed1->finalize ();
    cFed2->finalize ();

    BOOST_CHECK (cFed1->getCurrentMode () == helics::Federate::modes::finalize);
    BOOST_CHECK (cFed2->getCurrentMode () == helics::Federate::modes::finalize);
}

BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics::CombinationFederate cFed (std::string (TEST_DIR) + "/test_files/example_combo_fed.json");

    BOOST_CHECK_EQUAL (cFed.getName (), "comboFed");

    BOOST_CHECK_EQUAL (cFed.getEndpointCount (), 2);
    auto &id = cFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (cFed.getEndpointType (id), "genmessage");

    BOOST_CHECK_EQUAL (cFed.getInputCount (), 2);
    BOOST_CHECK_EQUAL (cFed.getPublicationCount (), 2);

	BOOST_CHECK(!cFed.getPublication(1).getInfo().empty());
    cFed.disconnect ();
}

BOOST_AUTO_TEST_CASE (test_file_load_toml)
{
    helics::CombinationFederate cFed (std::string (TEST_DIR) + "/test_files/example_combo_fed.toml");

    BOOST_CHECK_EQUAL (cFed.getName (), "comboFed");

    BOOST_CHECK_EQUAL (cFed.getEndpointCount (), 2);
    auto &id = cFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (cFed.getEndpointType (id), "genmessage");

    BOOST_CHECK_EQUAL (cFed.getInputCount (), 2);
    BOOST_CHECK_EQUAL (cFed.getPublicationCount (), 2);

    cFed.disconnect ();
}
BOOST_AUTO_TEST_SUITE_END ()
