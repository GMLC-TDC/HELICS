/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/MessageFederate.hpp"
#include "cpptestFixtures.hpp"

#include "gtest/gtest.h"
#include <future>
#include <iostream>
#include <thread>
// these test cases test out the message federates

struct mfed_tests: public FederateTestFixture_cpp, public ::testing::Test {
};

class mfed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture_cpp {
};

/** test simple creation and destruction*/
TEST_P(mfed_type_tests, message_federate_initialize_tests)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    mFed1->enterExecutingMode();

    helics_federate_state mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_state_execution);

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_type_tests, message_federate_endpoint_registration)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    helics_federate_state mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_state_execution);

    EXPECT_EQ(std::string(epid.getName()), "fed0/ep1");
    EXPECT_EQ(std::string(epid2.getName()), "ep2");

    EXPECT_EQ(std::string(epid.getType()), "");
    EXPECT_EQ(std::string(epid2.getType()), "random");

    mFed1->finalize();

    mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

TEST_P(mfed_type_tests, message_federate_send_receive)
{
    SetupTest<helicscpp::MessageFederate>(GetParam(), 1, 1.0);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate>(0);

    auto epid = mFed1->registerEndpoint("ep1");
    auto epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    std::string data(500, 'a');

    epid.sendMessage("ep2", data, 0.0);
    helics_time time = mFed1->requestTime(1.0);

    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = epid.hasMessage();
    EXPECT_TRUE(res == false);
    res = epid2.hasMessage();
    EXPECT_TRUE(res);

    auto M = epid2.getMessage();
    // BOOST_REQUIRE (M);
    ASSERT_EQ(M.length, 500);
    EXPECT_NE(M.data, nullptr);
    if (M.data != nullptr) {
        EXPECT_EQ(M.data[245], 'a');
    }
    mFed1->finalize();

    auto mFed1State = mFed1->getCurrentMode();
    EXPECT_TRUE(mFed1State == helics_federate_state::helics_state_finalize);
}

INSTANTIATE_TEST_SUITE_P(mfed_tests, mfed_type_tests, ::testing::ValuesIn(core_types_simple));

/*

BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed, bdata::make (core_types), core_type)
{
   // extraBrokerArgs = "--logleve=4";
    SetupTest(helicsCreateMessageFederate,core_type, 2);
    auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);
    //mFed1->setLoggingLevel(4);
    //mFed2->setLoggingLevel(4);
    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random");

    CE(helicsFederateSetTimeDelta (mFed1, 1.0,&err));
     CE(helicsFederateSetTimeDelta (mFed2, 1.0,&err));

     CE(helicsFederateEnterExecutingModeAsync (mFed1,&err));
    CE(helicsFederateEnterExecutingMode (mFed2,&err));
    CE(helicsFederateEnterExecutingModeComplete (mFed1,&err));

    helics_federate_state mFed1State = helics_state_execution;
    CE(helicsFederateGetState(mFed1, &mFed1State,&err));
    EXPECT_TRUE (mFed1State == helics_state_execution);
    helics_federate_state mFed2State = helics_state_execution;
    CE(helicsFederateGetState(mFed2, &mFed2State,&err));
    EXPECT_TRUE (mFed2State == helics_state_execution);

    std::string data(500, 'a');
    std::string data2(400, 'b');

    CE(helicsEndpointSendEventRaw (epid, "ep2", data.c_str(,&err), 500, 0.0));
    CE(helicsEndpointSendEventRaw (epid2, "fed0/ep1", data2.c_str(,&err), 400, 0.0));
    // move the time to 1.0
    helics_time time;
    CE(helicsFederateRequestTimeAsync (mFed1, 1.0,&err));
    helics_time gtime;
    CE(helicsFederateRequestTime (mFed2, 1.0, &gtime,&err));
    CE(helicsFederateRequestTimeComplete (mFed1, &time,&err));

    EXPECT_EQ (gtime, 1.0);
    EXPECT_EQ (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);;
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid);
    EXPECT_TRUE (res);
    res = helicsEndpointHasMessage (epid2);
    EXPECT_TRUE (res);

    auto M1 = helicsEndpointGetMessage (epid);
    //BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1.length, 400);

    EXPECT_EQ (M1.data[245], 'b');

    auto M2 = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2.length, 500);

    EXPECT_EQ (M2.data[245], 'a');
    CE(helicsFederateFinalize(mFed1,&err));
    CE(helicsFederateFinalize(mFed2,&err));

    mFed1State = helics_federate_state::helics_state_finalize;
    CE(helicsFederateGetState(mFed1, &mFed1State,&err));
    EXPECT_TRUE (mFed1State == helics_federate_state::helics_state_finalize);
    mFed2State = helics_federate_state::helics_state_finalize;
    CE(helicsFederateGetState(mFed2, &mFed2State,&err));
    EXPECT_TRUE (mFed2State == helics_federate_state::helics_state_finalize);
}
*/
/*


BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_obj, bdata::make (core_types), core_type)
{
    using namespace helics;
    SetupTest<MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<MessageFederate> (0);
    auto mFed2 = GetFederateAs<MessageFederate> (1);

    Endpoint epid (mFed1.get (), "ep1");

    Endpoint epid2 (GLOBAL, mFed2.get (), "ep2", "random");

    mFed1->setTimeProperty (helics_property_time_delta, 1.0);
    mFed2->setTimeProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::executing);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::executing);

    helicscpp::data_block data (500, 'a');
    helicscpp::data_block data2 (400, 'b');

    epid.send ("ep2", data);
    epid2.send ("fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    EXPECT_EQ (gtime, 1.0);
    EXPECT_EQ (f1time.get (), 1.0);

    auto res = mFed1->hasMessage ();
    EXPECT_TRUE (res);
    res = epid.hasMessage ();
    EXPECT_TRUE (res);
    epid2.hasMessage ();
    EXPECT_TRUE (res);

    auto M1 = epid.getMessage ();
    BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data2.size ());

    EXPECT_EQ (M1->data[245], data2[245]);

    auto M2 = epid2.getMessage ();
    BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    EXPECT_EQ (M2->data[245], data[245]);
    mFed1->finalize ();
    mFed2->finalize ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::finalize);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::finalize);
}



BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_multisend, bdata::make (core_types), core_type)
{
    SetupTest<helicscpp::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helicscpp::MessageFederate> (1);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setTimeProperty (helics_property_time_delta, 1.0);
    mFed2->setTimeProperty (helics_property_time_delta, 1.0);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::executing);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::executing);

    helicscpp::data_block data1 (500, 'a');
    helicscpp::data_block data2 (400, 'b');
    helicscpp::data_block data3 (300, 'c');
    helicscpp::data_block data4 (200, 'd');
    mFed1->sendMessage (epid, "ep2", data1);
    mFed1->sendMessage (epid, "ep2", data2);
    mFed1->sendMessage (epid, "ep2", data3);
    mFed1->sendMessage (epid, "ep2", data4);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    EXPECT_EQ (gtime, 1.0);
    EXPECT_EQ (f1time.get (), 1.0);

    EXPECT_TRUE (!mFed1->hasMessage ());

    EXPECT_TRUE (!mFed1->hasMessage (epid));
    auto cnt = mFed2->receiveCount (epid2);
    EXPECT_EQ (cnt, 4);

    auto M1 = mFed2->getMessage (epid2);
    BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1->data.size (), data1.size ());

    EXPECT_EQ (M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->receiveCount (epid2);
    EXPECT_EQ (cnt, 3);
    auto M2 = mFed2->getMessage ();
    BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data2.size ());
    EXPECT_EQ (M2->data[245], data2[245]);
    cnt = mFed2->receiveCount (epid2);
    EXPECT_EQ (cnt, 2);

    auto M3 = mFed2->getMessage ();
    auto M4 = mFed2->getMessage (epid2);
    BOOST_REQUIRE(M3);
    BOOST_REQUIRE(M4);
    EXPECT_EQ (M3->data.size (), data3.size ());
    EXPECT_EQ (M4->data.size (), data4.size ());

    EXPECT_EQ (M4->source, "fed0/ep1");
    EXPECT_EQ (M4->dest, "ep2");
    EXPECT_EQ (M4->original_source, "fed0/ep1");
    EXPECT_EQ (M4->time, 0.0);
    mFed1->finalize ();
    mFed2->finalize ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::finalize);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::finalize);
}
//#define ENABLE_OUTPUT
//trivial Federate that sends Messages and echoes a ping with a pong

class PingPongFed
{
  private:
    std::unique_ptr<helicscpp::MessageFederate> mFed;
    helicscpp::Time delta;  // the minimum time delta for the federate
    std::string name;  //!< the name of the federate
    helicscpp::core_type coreType;
    std::vector<std::pair<helicscpp::Time, std::string>> triggers;
    helicscpp::endpoint_id_t ep;
    int index = 0;

  public:
    int pings = 0;  //!< the number of pings received
    int pongs = 0;  //!< the number of pongs received
  public:
    PingPongFed (std::string fname, helicscpp::Time tDelta, helicscpp::core_type ctype)
        : delta (tDelta), name (std::move (fname)), coreType (ctype)
    {
        if (delta <= 0.0)
        {
            delta = 0.2;
        }
    }

  private:
    void initialize ()
    {
        helicscpp::FederateInfo fi (name);
        fi.coreName = "pptest";
        fi.coreType = coreType;
        fi.coreInitString = "3";
        fi.timeDelta = delta;
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("about to create federate ") + name + "\n";
#endif
        mFed = std::make_unique<helicscpp::MessageFederate> (fi);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("registering federate ") + name + "\n";
#endif
        ep = mFed->registerEndpoint ("port");
    }

  private:
    void processMessages (helicscpp::Time currentTime)
    {
        while (mFed->hasMessage (ep))
        {
            auto mess = mFed->getMessage (ep);
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
                mFed->sendMessage (ep, std::move (mess));
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
    void mainLoop (helicscpp::Time finish)
    {
        helicscpp::Time nextTime = 0;
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
                    mFed->sendMessage (ep, triggers[index].second, "ping");
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
    void run (helicscpp::Time finish)
    {
        initialize ();
        mFed->enterExecutingMode ();
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("entering Execute Mode ") + name + "\n";
#endif
        mainLoop (finish);
    }
    void addTrigger (helicscpp::Time triggerTime, const std::string &target)
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
    AddBroker (core_type, "3");

    auto ctype = helicscpp::coreTypeFromString (core_type);
    PingPongFed p1 ("fedA", 0.5, ctype);
    PingPongFed p2 ("fedB", 0.5, ctype);
    PingPongFed p3 ("fedC", 0.5, ctype);

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
    EXPECT_EQ (p1.pings, 2);
    EXPECT_EQ (p2.pings, 3);
    EXPECT_EQ (p3.pings, 1);
    EXPECT_EQ (p1.pongs, 3);
    EXPECT_EQ (p2.pongs, 1);
    EXPECT_EQ (p3.pongs, 2);
}



BOOST_DATA_TEST_CASE (test_time_interruptions, bdata::make (core_types), core_type)
{
    SetupTest<helicscpp::MessageFederate> (core_type, 2);
    auto mFed1 = GetFederateAs<helicscpp::MessageFederate> (0);
    auto mFed2 = GetFederateAs<helicscpp::MessageFederate> (1);

    auto epid = mFed1->registerEndpoint ("ep1");
    auto epid2 = mFed2->registerGlobalEndpoint ("ep2", "random");
    mFed1->setTimeProperty (helics_property_time_delta, 1.0);
    mFed2->setTimeProperty (helics_property_time_delta, 0.5);

    auto f1finish = std::async (std::launch::async, [&]() { mFed1->enterExecutingMode (); });
    mFed2->enterExecutingMode ();
    f1finish.wait ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::executing);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::executing);

    helicscpp::data_block data (500, 'a');
    helicscpp::data_block data2 (400, 'b');

    mFed1->sendMessage (epid, "ep2", data);
    mFed2->sendMessage (epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async (std::launch::async, [&]() { return mFed1->requestTime (1.0); });
    auto gtime = mFed2->requestTime (1.0);

    BOOST_REQUIRE_EQUAL (gtime, 0.5);

    BOOST_REQUIRE (mFed2->hasMessage (epid));

    auto M2 = mFed2->getMessage (epid2);
    BOOST_REQUIRE_EQUAL (M2->data.size (), data.size ());

    EXPECT_EQ (M2->data[245], data[245]);

    gtime = mFed2->requestTime (1.0);
    EXPECT_EQ (gtime, 1.0);

    EXPECT_EQ (f1time.get (), 1.0);
    auto M1 = mFed1->getMessage (epid);
    EXPECT_TRUE (M1);
    if (M1)
    {
        EXPECT_EQ (M1->data.size (), data2.size ());
        if (M1->data.size () > 245)
        {
            EXPECT_EQ (M1->data[245], data2[245]);
        }
    }

    EXPECT_TRUE (mFed1->hasMessage () == false);
    mFed1->finalize ();
    mFed2->finalize ();

    EXPECT_TRUE (mFed1->getCurrentMode () == helicscpp::Federate::modes::finalize);
    EXPECT_TRUE (mFed2->getCurrentMode () == helicscpp::Federate::modes::finalize);
}

BOOST_AUTO_TEST_CASE (test_file_load)
{
    helicscpp::MessageFederate mFed (std::string (TEST_DIR) + "/test_files/example_message_fed.json");

    EXPECT_EQ (mFed.getName (), "messageFed");

    EXPECT_EQ (mFed.getEndpointCount (), 2);
    auto id = mFed.getEndpointId ("ept1");
    EXPECT_EQ (mFed.getEndpointType (id), "genmessage");

    mFed.disconnect ();
}
*/
