/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "testFixtures.hpp"

#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
/** these test cases test out the message federates
 */

class mfed_add_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_add_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class mfed_add_all_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(mfed_add_single_type_tests, initialize_tests)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);

    mFed1->finalize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_add_single_type_tests, endpoint_registration)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);

    auto& sv = mFed1->getInterfaceName(epid);
    auto& sv2 = mFed1->getInterfaceName(epid2);
    EXPECT_EQ(sv, "fed0/ep1");
    EXPECT_EQ(sv2, "ep2");

    EXPECT_EQ(mFed1->getExtractionType(epid), "");
    EXPECT_EQ(mFed1->getExtractionType(epid2), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("fed0/ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == epid2.getHandle());
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

// same as previous test case but using endpoint objects
TEST_P(mfed_add_single_type_tests, endpoint_registration_objs)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint epid(mFed1.get(), "ep1");
    helics::Endpoint epid2(helics::GLOBAL, mFed1.get(), "ep2", "random");
    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);

    auto const& sv = epid.getName();
    auto const& sv2 = epid2.getName();
    EXPECT_EQ(sv, "fed0/ep1");
    EXPECT_EQ(sv2, "ep2");

    EXPECT_EQ(epid.getType(), "");
    EXPECT_EQ(epid2.getType(), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == epid2.getHandle());
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };

    mFed1->setMessageNotificationCallback(mend);

    mFed1->setProperty(helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');

    mFed1->sendMessage(epid, "ep2", data);

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res == false);
    res = mFed1->hasMessage(epid2);
    EXPECT_TRUE(res);

    EXPECT_TRUE(rxend == epid2.getHandle());
    EXPECT_EQ(timeRx, helics::Time(1.0));
    auto M = mFed1->getMessage(epid2);
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback_obj)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint ep1(mFed1, "ep1");
    helics::Endpoint ep2(helics::GLOBAL, mFed1, "ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };

    ep2.setCallback(mend);

    mFed1->setProperty(helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');

    ep1.send("ep2", data);

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = ep2.hasMessage();
    EXPECT_TRUE(res);
    res = ep1.hasMessage();
    EXPECT_TRUE(!res);

    EXPECT_TRUE(rxend == ep2.getHandle());
    EXPECT_EQ(timeRx, helics::Time(1.0));
    auto M = ep2.getMessage();
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback_obj2)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint ep1(mFed1, "ep1");
    helics::Endpoint ep2(helics::GLOBAL, mFed1, "ep2", "random");

    helics::interface_handle rxend;
    helics::Time timeRx;
    auto mend = [&](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };

    ep2.setCallback(mend);

    mFed1->setProperty(helics_property_time_delta, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    helics::data_block data(500, 'a');

    ep1.send("ep2", data);

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = ep2.hasMessage();
    EXPECT_TRUE(res);
    res = ep1.hasMessage();
    EXPECT_TRUE(!res);

    EXPECT_TRUE(rxend == ep2.getHandle());
    EXPECT_EQ(timeRx, helics::Time(1.0));
    auto M = ep2.getMessage();
    ASSERT_TRUE(M);
    ASSERT_EQ(M->data.size(), data.size());

    EXPECT_EQ(M->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(mfed_add_all_type_tests, send_receive_2fed_multisend_callback)
{
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed2->registerGlobalEndpoint("ep2", "random");
    std::atomic<int> e1cnt{0};
    std::atomic<int> e2cnt{0};
    mFed1->setMessageNotificationCallback(epid, [&](const helics::Endpoint&, helics::Time) {
        ++e1cnt;
    });
    mFed2->setMessageNotificationCallback(epid2, [&](const helics::Endpoint&, helics::Time) {
        ++e2cnt;
    });
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data1(500, 'a');
    helics::data_block data2(400, 'b');
    helics::data_block data3(300, 'c');
    helics::data_block data4(200, 'd');
    mFed1->sendMessage(epid, "ep2", data1);
    mFed1->sendMessage(epid, "ep2", data2);
    mFed1->sendMessage(epid, "ep2", data3);
    mFed1->sendMessage(epid, "ep2", data4);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    EXPECT_TRUE(!mFed1->hasMessage());

    EXPECT_TRUE(!mFed1->hasMessage(epid));
    auto cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 4);

    auto M1 = mFed2->getMessage(epid2);
    ASSERT_TRUE(M1);
    ASSERT_EQ(M1->data.size(), data1.size());

    EXPECT_EQ(M1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 3);
    auto M2 = mFed2->getMessage();
    ASSERT_TRUE(M2);
    ASSERT_EQ(M2->data.size(), data2.size());
    EXPECT_EQ(M2->data[245], data2[245]);
    cnt = mFed2->pendingMessages(epid2);
    EXPECT_EQ(cnt, 2);

    auto M3 = mFed2->getMessage();
    auto M4 = mFed2->getMessage(epid2);
    EXPECT_EQ(M3->data.size(), data3.size());
    EXPECT_EQ(M4->data.size(), data4.size());

    EXPECT_EQ(M4->source, "fed0/ep1");
    EXPECT_EQ(M4->dest, "ep2");
    EXPECT_EQ(M4->original_source, "fed0/ep1");
    EXPECT_EQ(M4->time, 0.0);

    EXPECT_EQ(e1cnt, 0);
    EXPECT_EQ(e2cnt, 4);
    mFed1->finalizeAsync();
    mFed2->finalize();
    mFed1->finalizeComplete();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

//#define ENABLE_OUTPUT
/**trivial Federate that sends Messages and echoes a ping with a pong
 */
class PingPongFed {
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Time delta; // the minimum time delta for the federate
    std::string name; //!< the name of the federate
    helics::core_type coreType;
    std::vector<std::pair<helics::Time, std::string>> triggers;
    helics::Endpoint* ep{nullptr};
    int index{0};

  public:
    int pings{0}; //!< the number of pings received
    int pongs{0}; //!< the number of pongs received
  public:
    PingPongFed(const std::string& fname, helics::Time tDelta, helics::core_type ctype):
        delta(tDelta), name(fname), coreType(ctype)
    {
        if (delta <= 0.0) {
            delta = 0.2;
        }
    }

  private:
    void initialize()
    {
        helics::FederateInfo fi(coreType);
        fi.coreName = "pptest";
        fi.coreInitString = "-f 3";
        fi.setProperty(helics_property_time_delta, delta);
#ifdef ENABLE_OUTPUT
        std::cout << std::string("about to create federate ") + name + "\n";
#endif
        mFed = std::make_unique<helics::MessageFederate>(name, fi);
#ifdef ENABLE_OUTPUT
        std::cout << std::string("registering federate ") + name + "\n";
#endif
        ep = &mFed->registerEndpoint("port");
    }

  private:
    void processMessages(helics::Time currentTime)
    {
        while (mFed->hasMessage(*ep)) {
            auto mess = mFed->getMessage(*ep);
            auto messString = mess->data.to_string();
            if (messString == "ping") {
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive ping from " << std::string(mess->source)
                          << " at time " << static_cast<double>(currentTime) << '\n';
#endif
                mess->data = "pong";
                mess->dest = mess->source;
                mess->source = name;
                mess->original_source = mess->source;
                mess->time = currentTime;
                mFed->sendMessage(*ep, std::move(mess));
                pings++;
            } else if (messString == "pong") {
                pongs++;
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive pong from " << std::string(mess->source)
                          << " at time " << static_cast<double>(currentTime) << '\n';
#endif
            }
        }
    }
    void mainLoop(helics::Time finish)
    {
        helics::Time nextTime = 0;
        while (nextTime <= finish) {
            processMessages(nextTime);
            if (index < static_cast<int>(triggers.size())) {
                while (triggers[index].first <= nextTime) {
#ifdef ENABLE_OUTPUT
                    std::cout << name << ": send ping to " << triggers[index].second << " at time "
                              << static_cast<double>(nextTime) << '\n';
#endif
                    mFed->sendMessage(*ep, triggers[index].second, "ping");
                    ++index;
                    if (index >= static_cast<int>(triggers.size())) {
                        break;
                    }
                }
            }
            nextTime += delta;
            nextTime = mFed->requestTime(nextTime);
        }
        mFed->finalize();
    }

  public:
    void run(helics::Time finish)
    {
        initialize();
        mFed->enterExecutingMode();
#ifdef ENABLE_OUTPUT
        std::cout << std::string("entering Execute Mode ") + name + "\n";
#endif
        mainLoop(finish);
    }
    void addTrigger(helics::Time triggerTime, const std::string& target)
    {
        triggers.emplace_back(triggerTime, target);
    }
};

TEST_P(mfed_add_type_tests, threefedPingPong)
{
    if (GetParam() != std::string("test")) {
        return;
    }
    auto brk = AddBroker(GetParam(), "-f 3");

    auto crtype = helics::core::coreTypeFromString(GetParam());
    PingPongFed p1("fedA", 0.5, crtype);
    PingPongFed p2("fedB", 0.5, crtype);
    PingPongFed p3("fedC", 0.5, crtype);

    p1.addTrigger(0.5, "fedB/port");
    p1.addTrigger(0.5, "fedC/port");
    p1.addTrigger(3.0, "fedB/port");
    p2.addTrigger(1.5, "fedA/port");
    p3.addTrigger(3.0, "fedB/port");
    p3.addTrigger(4.0, "fedA/port");

    auto t1 = std::thread([&p1]() { p1.run(6.0); });
    auto t2 = std::thread([&p2]() { p2.run(6.0); });
    auto t3 = std::thread([&p3]() { p3.run(6.0); });

    t1.join();
    t2.join();
    t3.join();
    EXPECT_EQ(p1.pings, 2);
    EXPECT_EQ(p2.pings, 3);
    EXPECT_EQ(p3.pings, 1);
    EXPECT_EQ(p1.pongs, 3);
    EXPECT_EQ(p2.pongs, 1);
    EXPECT_EQ(p3.pongs, 2);
}

INSTANTIATE_TEST_SUITE_P(
    mfed_add_tests,
    mfed_add_single_type_tests,
    ::testing::ValuesIn(core_types_single));
INSTANTIATE_TEST_SUITE_P(mfed_add_tests, mfed_add_type_tests, ::testing::ValuesIn(core_types));
INSTANTIATE_TEST_SUITE_P(
    mfed_add_tests,
    mfed_add_all_type_tests,
    ::testing::ValuesIn(core_types_all));

static constexpr const char* config_files[] = {"example_message_fed.json",
                                               "example_message_fed.toml"};

class mfed_file_config_files:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(mfed_file_config_files, test_file_load)
{
    helics::MessageFederate mFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(mFed.getName(), "messageFed");

    EXPECT_EQ(mFed.getEndpointCount(), 2);
    auto id = mFed.getEndpoint("ept1");
    EXPECT_EQ(mFed.getExtractionType(id), "genmessage");
    EXPECT_EQ(id.getInfo(), "this is an information string for use by the application");

    EXPECT_EQ(mFed.query("global", "global1"), "this is a global1 value");
    EXPECT_EQ(mFed.query("global", "global2"), "this is another global value");
    mFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(mfed_add_tests, mfed_file_config_files, ::testing::ValuesIn(config_files));

static constexpr const char* filter_config_files[] = {"example_filters.json",
                                                      "example_filters.toml"};

class mfed_file_filter_config_files:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(mfed_file_filter_config_files, test_file_load_filter)
{
    helics::MessageFederate mFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(mFed.getName(), "filterFed");

    EXPECT_EQ(mFed.getEndpointCount(), 3);
    auto id = mFed.getEndpoint("ept1");
    EXPECT_EQ(mFed.getExtractionType(id), "genmessage");

    EXPECT_EQ(mFed.filterCount(), 3);

    auto filt = &mFed.getFilter(2);

    auto cloneFilt = dynamic_cast<helics::CloningFilter*>(filt);
    EXPECT_TRUE(cloneFilt != nullptr);

    EXPECT_EQ(
        mFed.getFilter(0).getInfo(), "this is an information string for use by the application");
    mFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(
    mfed_add_tests,
    mfed_file_filter_config_files,
    ::testing::ValuesIn(filter_config_files));
