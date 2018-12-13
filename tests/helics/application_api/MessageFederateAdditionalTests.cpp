/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "testFixtures.hpp"

#include <future>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */
namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE (message_federate_additional_tests, FederateTestFixture, *utf::label ("ci"))

BOOST_DATA_TEST_CASE (message_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);

    mFed1->finalize ();
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_endpoint_registration, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    auto &epid = mFed1->registerEndpoint ("ep1");
    auto &epid2 = mFed1->registerGlobalEndpoint ("ep2", "random");

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);

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

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

// same as previous test case but using endpoint objects
BOOST_DATA_TEST_CASE (message_federate_endpoint_registration_objs, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    helics::Endpoint epid (mFed1.get (), "ep1");
    helics::Endpoint epid2 (helics::GLOBAL, mFed1.get (), "ep2", "random");
    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);

    auto const &sv = epid.getName ();
    auto const &sv2 = epid2.getName ();
    BOOST_CHECK_EQUAL (sv, "fed0/ep1");
    BOOST_CHECK_EQUAL (sv2, "ep2");

    BOOST_CHECK_EQUAL (epid.getType (), "");
    BOOST_CHECK_EQUAL (epid2.getType (), "random");

    BOOST_CHECK (mFed1->getEndpoint ("ep1").getHandle () == epid.getHandle ());
    BOOST_CHECK (mFed1->getEndpoint ("ep2").getHandle () == epid2.getHandle ());
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_callback, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    auto &epid = mFed1->registerEndpoint ("ep1");
    auto &epid2 = mFed1->registerGlobalEndpoint ("ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint &ept, helics::Time rtime) {
        rxend = ept.getHandle ();
        timeRx = rtime;
    };

    mFed1->setMessageNotificationCallback (mend);

    mFed1->setProperty (helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');

    mFed1->sendMessage (epid, "ep2", data);

    auto time = mFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = mFed1->hasMessage ();
    BOOST_CHECK (res);
    res = mFed1->hasMessage (epid);
    BOOST_CHECK (res == false);
    res = mFed1->hasMessage (epid2);
    BOOST_CHECK (res);

    BOOST_CHECK (rxend == epid2.getHandle ());
    BOOST_CHECK_EQUAL (timeRx, helics::Time (1.0));
    auto M = mFed1->getMessage (epid2);
    BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M->data[245], data[245]);
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_callback_obj, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    helics::Endpoint ep1 (mFed1, "ep1");
    helics::Endpoint ep2 (helics::GLOBAL, mFed1, "ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint &ept, helics::Time rtime) {
        rxend = ept.getHandle ();
        timeRx = rtime;
    };

    ep2.setCallback (mend);

    mFed1->setProperty (helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');

    ep1.send ("ep2", data);

    auto time = mFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = ep2.hasMessage ();
    BOOST_CHECK (res);
    res = ep1.hasMessage ();
    BOOST_CHECK (!res);

    BOOST_CHECK (rxend == ep2.getHandle ());
    BOOST_CHECK_EQUAL (timeRx, helics::Time (1.0));
    auto M = ep2.getMessage ();
    BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M->data[245], data[245]);
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_callback_obj2, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);

    helics::Endpoint ep1 (mFed1, "ep1");
    helics::Endpoint ep2 (helics::GLOBAL, mFed1, "ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint &ept, helics::Time rtime) {
        rxend = ept.getHandle ();
        timeRx = rtime;
    };

    ep2.setCallback (mend);

    mFed1->setProperty (helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);
    helics::data_block data (500, 'a');

    ep1.send ("ep2", data);

    auto time = mFed1->requestTime (1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = ep2.hasMessage ();
    BOOST_CHECK (res);
    res = ep1.hasMessage ();
    BOOST_CHECK (!res);

    BOOST_CHECK (rxend == ep2.getHandle ());
    BOOST_CHECK_EQUAL (timeRx, helics::Time (1.0));
    auto M = ep2.getMessage ();
    BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M->data.size (), data.size ());

    BOOST_CHECK_EQUAL (M->data[245], data[245]);
    mFed1->finalize ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
}

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_multisend_callback,
                      bdata::make (core_types_all),
                      core_type)
{
    SetupTest<helics::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helics::MessageFederate> (1);

    auto &epid = mFed1->registerEndpoint ("ep1");
    auto &epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");
    std::atomic<int> e1cnt{0};
    std::atomic<int> e2cnt{0};
    mFed1->setMessageNotificationCallback (epid, [&](const helics::Endpoint &, helics::Time) { ++e1cnt; });
    mFed2->setMessageNotificationCallback (epid2, [&](const helics::Endpoint &, helics::Time) { ++e2cnt; });
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty (helics_property_time_delta, 1.0);
    mFed2->setProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::execution);
    BOOST_CHECK (mFed2->getCurrentState () == helics::Federate::states::execution);

    helics::data_block data1 (500, 'a');
    helics::data_block data2 (400, 'b');
    helics::data_block data3 (300, 'c');
    helics::data_block data4 (200, 'd');
    mFed1->sendMessage (epid, "ep2", data1);
    mFed1->sendMessage (epid, "ep2", data2);
    mFed1->sendMessage (epid, "ep2", data3);
    mFed1->sendMessage (epid, "ep2", data4);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (f1time.get (), 1.0);

    BOOST_CHECK (!mFed1->hasMessage ());

    BOOST_CHECK (!mFed1->hasMessage (epid));
    auto cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 4);

    auto M1 = mFed2->getMessage (epid2);
    BOOST_REQUIRE (M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data1.size ());

    BOOST_CHECK_EQUAL (M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 3);
    auto M2 = mFed2->getMessage ();
    BOOST_REQUIRE (M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data2.size ());
    BOOST_CHECK_EQUAL (M2->data[245], data2[245]);
    cnt = mFed2->pendingMessages (epid2);
    BOOST_CHECK_EQUAL (cnt, 2);

    auto M3 = mFed2->getMessage ();
    auto M4 = mFed2->getMessage (epid2);
    BOOST_CHECK_EQUAL (M3->data.size (), data3.size ());
    BOOST_CHECK_EQUAL (M4->data.size (), data4.size ());

    BOOST_CHECK_EQUAL (M4->source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4->dest, "ep2");
    BOOST_CHECK_EQUAL (M4->original_source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4->time, 0.0);

    BOOST_CHECK_EQUAL (e1cnt, 0);
    BOOST_CHECK_EQUAL (e2cnt, 4);
    mFed1->finalize ();
    mFed2->finalize ();

    BOOST_CHECK (mFed1->getCurrentState () == helics::Federate::states::finalize);
    BOOST_CHECK (mFed2->getCurrentState () == helics::Federate::states::finalize);
}

//#define ENABLE_OUTPUT
/**trivial Federate that sends Messages and echoes a ping with a pong
 */
class PingPongFed
{
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Time delta;  // the minimum time delta for the federate
    std::string name;  //!< the name of the federate
    helics::core_type coreType;
    std::vector<std::pair<helics::Time, std::string>> triggers;
    helics::Endpoint *ep;
    int index = 0;

  public:
    int pings = 0;  //!< the number of pings received
    int pongs = 0;  //!< the number of pongs received
  public:
    PingPongFed (const std::string &fname, helics::Time tDelta, helics::core_type ctype)
        : delta (tDelta), name (fname), coreType (ctype)
    {
        if (delta <= 0.0)
        {
            delta = 0.2;
        }
    }

  private:
    void initialize ()
    {
        helics::FederateInfo fi (coreType);
        fi.coreName = "pptest";
        fi.coreInitString = "-f 3";
        fi.setProperty (helics_property_time_delta, delta);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("about to create federate ") + name + "\n";
#endif
        mFed = std::make_unique<helics::MessageFederate> (name, fi);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("registering federate ") + name + "\n";
#endif
        ep = &mFed->registerEndpoint ("port");
    }

  private:
    void processMessages (helics::Time currentTime)
    {
        while (mFed->hasMessage (*ep))
        {
            auto mess = mFed->getMessage (*ep);
            auto messString = mess->data.to_string ();
            if (messString == "ping")
            {
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive ping from " << std::string (mess->source) << " at time "
                          << static_cast<double> (currentTime) << '\n';
#endif
                mess->data = "pong";
                mess->dest = mess->source;
                mess->source = name;
                mess->original_source = mess->source;
                mess->time = currentTime;
                mFed->sendMessage (*ep, std::move (mess));
                pings++;
            }
            else if (messString == "pong")
            {
                pongs++;
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive pong from " << std::string (mess->source) << " at time "
                          << static_cast<double> (currentTime) << '\n';
#endif
            }
        }
    }
    void mainLoop (helics::Time finish)
    {
        helics::Time nextTime = 0;
        while (nextTime <= finish)
        {
            processMessages (nextTime);
            if (index < static_cast<int> (triggers.size ()))
            {
                while (triggers[index].first <= nextTime)
                {
#ifdef ENABLE_OUTPUT
                    std::cout << name << ": send ping to " << triggers[index].second << " at time "
                              << static_cast<double> (nextTime) << '\n';
#endif
                    mFed->sendMessage (*ep, triggers[index].second, "ping");
                    ++index;
                    if (index >= static_cast<int> (triggers.size ()))
                    {
                        break;
                    }
                }
            }
            nextTime += delta;
            nextTime = mFed->requestTime (nextTime);
        }
        mFed->finalize ();
    }

  public:
    void run (helics::Time finish)
    {
        initialize ();
        mFed->enterExecutingMode ();
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("entering Execute Mode ") + name + "\n";
#endif
        mainLoop (finish);
    }
    void addTrigger (helics::Time triggerTime, const std::string &target)
    {
        triggers.emplace_back (triggerTime, target);
    }
};

BOOST_DATA_TEST_CASE (threefedPingPong, bdata::make (core_types), core_type)
{
    if (core_type != "test")
    {
        return;
    }
    AddBroker (core_type, "-f 3");

    auto crtype = helics::coreTypeFromString (core_type);
    PingPongFed p1 ("fedA", 0.5, crtype);
    PingPongFed p2 ("fedB", 0.5, crtype);
    PingPongFed p3 ("fedC", 0.5, crtype);

    p1.addTrigger (0.5, "fedB/port");
    p1.addTrigger (0.5, "fedC/port");
    p1.addTrigger (3.0, "fedB/port");
    p2.addTrigger (1.5, "fedA/port");
    p3.addTrigger (3.0, "fedB/port");
    p3.addTrigger (4.0, "fedA/port");

    auto t1 = std::thread ([&p1]() { p1.run (6.0); });
    auto t2 = std::thread ([&p2]() { p2.run (6.0); });
    auto t3 = std::thread ([&p3]() { p3.run (6.0); });

    t1.join ();
    t2.join ();
    t3.join ();
    BOOST_CHECK_EQUAL (p1.pings, 2);
    BOOST_CHECK_EQUAL (p2.pings, 3);
    BOOST_CHECK_EQUAL (p3.pings, 1);
    BOOST_CHECK_EQUAL (p1.pongs, 3);
    BOOST_CHECK_EQUAL (p2.pongs, 1);
    BOOST_CHECK_EQUAL (p3.pongs, 2);
}

BOOST_AUTO_TEST_CASE (test_file_load)
{
    helics::MessageFederate mFed (std::string (TEST_DIR) + "/example_message_fed.json");

    BOOST_CHECK_EQUAL (mFed.getName (), "messageFed");

    BOOST_CHECK_EQUAL (mFed.getEndpointCount (), 2);
    auto id = mFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (mFed.getEndpointType (id), "genmessage");
    BOOST_CHECK_EQUAL (id.getInfo (), "this is an information string for use by the application");

    BOOST_CHECK_EQUAL (mFed.query ("global", "global1"), "this is a global1 value");
    BOOST_CHECK_EQUAL (mFed.query ("global", "global2"), "this is another global value");
    mFed.disconnect ();
}

BOOST_AUTO_TEST_CASE (test_file_load_toml)
{
    helics::MessageFederate mFed (std::string (TEST_DIR) + "/example_message_fed.toml");

    BOOST_CHECK_EQUAL (mFed.getName (), "messageFed");

    BOOST_CHECK_EQUAL (mFed.getEndpointCount (), 2);
    auto id = mFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (mFed.getEndpointType (id), "genmessage");
    BOOST_CHECK_EQUAL (id.getInfo (), "this is an information string for use by the application");
    mFed.disconnect ();
}

BOOST_AUTO_TEST_CASE (test_file_load_filter)
{
    helics::MessageFederate mFed (std::string (TEST_DIR) + "/example_filters.json");

    BOOST_CHECK_EQUAL (mFed.getName (), "filterFed");

    BOOST_CHECK_EQUAL (mFed.getEndpointCount (), 3);
    auto id = mFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (mFed.getEndpointType (id), "genmessage");

    BOOST_CHECK_EQUAL (mFed.filterCount (), 3);

    auto filt = &mFed.getFilter (2);

    auto cloneFilt = dynamic_cast<helics::CloningFilter *> (filt);
    BOOST_CHECK (cloneFilt != nullptr);

    BOOST_CHECK_EQUAL (mFed.getFilter (0).getInfo (), "this is an information string for use by the application");
    mFed.disconnect ();
}

BOOST_AUTO_TEST_CASE (test_file_load_filter_toml)
{
    helics::MessageFederate mFed (std::string (TEST_DIR) + "/example_filters.toml");

    BOOST_CHECK_EQUAL (mFed.getName (), "filterFed");

    BOOST_CHECK_EQUAL (mFed.getEndpointCount (), 3);
    auto &id = mFed.getEndpoint ("ept1");
    BOOST_CHECK_EQUAL (mFed.getEndpointType (id), "genmessage");

    BOOST_CHECK_EQUAL (mFed.filterCount (), 3);

    auto filt = &mFed.getFilter (2);

    auto cloneFilt = dynamic_cast<helics::CloningFilter *> (filt);
    BOOST_CHECK (cloneFilt != nullptr);

    BOOST_CHECK_EQUAL (mFed.getFilter (0).getInfo (), "this is an information string for use by the application");
    mFed.disconnect ();
}

BOOST_AUTO_TEST_SUITE_END ()
