/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/flagOperations.hpp"
#include "testFixtures.hpp"

#include <gmlc/libguarded/guarded.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** these test cases test out the message federates
 */

class mfed_add_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_add_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_add_all_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class mfed_tests: public ::testing::Test, public FederateTestFixture {};

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_P(mfed_add_single_type_tests, initialize_tests)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    mFed1->finalize();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_F(mfed_tests, endpoint_registration)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& ep1 = mFed1->registerEndpoint("ep1");
    auto& ep2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto& ep1Name = ep1.getName();
    auto& ep2Name = ep2.getName();
    EXPECT_EQ(ep1Name, "fed0/ep1");
    EXPECT_EQ(ep2Name, "ep2");

    EXPECT_EQ(ep1.getExtractionType(), "");
    EXPECT_EQ(ep2.getExtractionType(), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == ep1.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("fed0/ep1").getHandle() == ep1.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == ep2.getHandle());
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

// same as previous test case but using endpoint objects
TEST_P(mfed_add_single_type_tests, endpoint_registration_objs)
{
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint epid(mFed1.get(), "ep1");
    helics::Endpoint epid2(helics::InterfaceVisibility::GLOBAL, mFed1.get(), "ep2", "random");
    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto const& endpoint1Name = epid.getName();
    auto const& endpoint2Name = epid2.getName();
    EXPECT_EQ(endpoint1Name, "fed0/ep1");
    EXPECT_EQ(endpoint2Name, "ep2");

    EXPECT_EQ(epid.getType(), "");
    EXPECT_EQ(epid2.getType(), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == epid2.getHandle());
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback)
{
    debugDiagnostic = true;
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    helics::InterfaceHandle rxend;
    helics::Time timeRx;
    auto mend = [&rxend, &timeRx](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };

    mFed1->setMessageNotificationCallback(mend);

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    epid.sendTo(data, "ep2");

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
    auto message = mFed1->getMessage(epid2);
    ASSERT_TRUE(message);
    ASSERT_EQ(message->data.size(), data.size());

    EXPECT_EQ(message->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback_obj)
{
    debugDiagnostic = true;
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint ep1(mFed1, "ep1");
    helics::Endpoint ep2(helics::InterfaceVisibility::GLOBAL, mFed1, "ep2", "random");

    helics::InterfaceHandle rxend;
    helics::Time timeRx;
    auto mend = [&rxend, &timeRx](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };

    ep2.setCallback(mend);

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    ep1.sendTo(data, "ep2");

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = ep2.hasMessage();
    EXPECT_TRUE(res);
    res = ep1.hasMessage();
    EXPECT_TRUE(!res);

    EXPECT_TRUE(rxend == ep2.getHandle());
    EXPECT_EQ(timeRx, helics::Time(1.0));
    auto message = ep2.getMessage();
    ASSERT_TRUE(message);
    ASSERT_EQ(message->data.size(), data.size());

    EXPECT_EQ(message->data[245], data[245]);
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_add_single_type_tests, send_receive_callback_obj2)
{
    debugDiagnostic = true;
    SetupTest<helics::MessageFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    helics::Endpoint ep1(mFed1, "ep1");
    helics::Endpoint ep2(helics::InterfaceVisibility::GLOBAL, mFed1, "ep2", "random");

    helics::InterfaceHandle rxend;
    helics::Time timeRx;
    auto mend = [&rxend, &timeRx](const helics::Endpoint& ept, helics::Time rtime) {
        rxend = ept.getHandle();
        timeRx = rtime;
    };
    ep2.setCallback(mend);
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingMode();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    helics::SmallBuffer data(500, 'a');

    ep1.sendTo(data, "ep2");

    auto time = mFed1->requestTime(1.0);
    EXPECT_EQ(time, 1.0);

    auto res = ep2.hasMessage();
    EXPECT_TRUE(res);
    res = ep1.hasMessage();
    EXPECT_TRUE(!res);
    EXPECT_TRUE(rxend == ep2.getHandle());
    EXPECT_EQ(timeRx, helics::Time(1.0));
    auto message = ep2.getMessage();
    ASSERT_TRUE(message);
    ASSERT_EQ(message->data.size(), data.size());

    EXPECT_EQ(message->data[245], data[245]);
    mFed1->finalize();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(mfed_add_all_type_tests, send_receive_2fed_multisend_callback)
{
    debugDiagnostic = true;
    SetupTest<helics::MessageFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerEndpoint("ep1");
    auto& ep2 = mFed2->registerGlobalEndpoint("ep2", "random");
    std::atomic<int> e1cnt{0};
    std::atomic<int> e2cnt{0};
    mFed1->setMessageNotificationCallback(ep1,
                                          [&](const helics::Endpoint& /*unused*/,
                                              helics::Time /*unused*/) { ++e1cnt; });
    mFed2->setMessageNotificationCallback(ep2,
                                          [&](const helics::Endpoint& /*unused*/,
                                              helics::Time /*unused*/) { ++e2cnt; });
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data1(500, std::byte{'a'});
    helics::SmallBuffer data2(400, std::byte{'b'});
    helics::SmallBuffer data3(300, std::byte{'c'});
    helics::SmallBuffer data4(200, std::byte{'d'});
    ep1.sendTo(data1, "ep2");
    ep1.sendTo(data2, "ep2");
    ep1.sendTo(data3, "ep2");
    ep1.sendTo(data4, "ep2");
    // move the time to 1.0
    mFed1->requestTimeAsync(1.0);
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(mFed1->requestTimeComplete(), 1.0);
    EXPECT_TRUE(!mFed1->hasMessage());

    EXPECT_TRUE(!mFed1->hasMessage(ep1));
    auto cnt = mFed2->pendingMessageCount(ep2);
    EXPECT_EQ(cnt, 4);

    cnt = mFed2->pendingMessageCount();
    EXPECT_EQ(cnt, 4);

    auto message1 = mFed2->getMessage(ep2);
    ASSERT_TRUE(message1);
    ASSERT_EQ(message1->data.size(), data1.size());

    EXPECT_EQ(message1->data[245], data1[245]);
    // check the count decremented
    cnt = mFed2->pendingMessageCount(ep2);
    EXPECT_EQ(cnt, 3);
    auto message2 = mFed2->getMessage();
    ASSERT_TRUE(message2);
    ASSERT_EQ(message2->data.size(), data2.size());
    EXPECT_EQ(message2->data[245], data2[245]);
    cnt = mFed2->pendingMessageCount(ep2);
    EXPECT_EQ(cnt, 2);

    auto message3 = mFed2->getMessage();
    auto message4 = mFed2->getMessage(ep2);
    EXPECT_EQ(message3->data.size(), data3.size());
    EXPECT_EQ(message4->data.size(), data4.size());

    EXPECT_EQ(message4->source, "fed0/ep1");
    EXPECT_EQ(message4->dest, "ep2");
    EXPECT_EQ(message4->original_source, "fed0/ep1");
    EXPECT_EQ(message4->time, 0.0);
    EXPECT_EQ(e1cnt, 0);
    EXPECT_EQ(e2cnt, 4);
    mFed1->finalizeAsync();
    mFed2->disconnect();
    mFed1->finalizeComplete();
    mFed1->disconnect();
    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

// #define ENABLE_OUTPUT
/**trivial Federate that sends Messages and echoes a ping with a pong
 */
class PingPongFed {
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Time delta;  // the minimum time delta for the federate
    std::string name;  //!< the name of the federate
    helics::CoreType coreType;
    std::vector<std::pair<helics::Time, std::string>> triggers;
    helics::Endpoint* ept{nullptr};
    int index{0};

  public:
    int pings{0};  //!< the number of pings received
    int pongs{0};  //!< the number of pongs received
    PingPongFed(const std::string& fname, helics::Time tDelta, helics::CoreType ctype):
        delta(tDelta), name(fname), coreType(ctype)
    {
        if (delta <= 0.0) {
            delta = 0.2;
        }
    }

  private:
    void initialize()
    {
        helics::FederateInfo fedInfo(coreType);
        fedInfo.coreName = "pptest";
        fedInfo.coreInitString = "-f 3";
        fedInfo.setProperty(HELICS_PROPERTY_TIME_DELTA, delta);
#ifdef ENABLE_OUTPUT
        std::cout << std::string("about to create federate ") + name + "\n";
#endif
        mFed = std::make_unique<helics::MessageFederate>(name, fedInfo);
#ifdef ENABLE_OUTPUT
        std::cout << std::string("registering federate ") + name + "\n";
#endif
        ept = &mFed->registerEndpoint("port");
    }

    void processMessages(helics::Time currentTime)
    {
        while (mFed->hasMessage(*ept)) {
            auto mess = mFed->getMessage(*ept);
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
                ept->send(std::move(mess));
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
                    ept->sendTo("ping", triggers[index].second);
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
    PingPongFed ping1("fedA", 0.5, crtype);
    PingPongFed ping2("fedB", 0.5, crtype);
    PingPongFed ping3("fedC", 0.5, crtype);

    ping1.addTrigger(0.5, "fedB/port");
    ping1.addTrigger(0.5, "fedC/port");
    ping1.addTrigger(3.0, "fedB/port");
    ping2.addTrigger(1.5, "fedA/port");
    ping3.addTrigger(3.0, "fedB/port");
    ping3.addTrigger(4.0, "fedA/port");

    auto thread1 = std::thread([&ping1]() { ping1.run(6.0); });
    auto thread2 = std::thread([&ping2]() { ping2.run(6.0); });
    auto thread3 = std::thread([&ping3]() { ping3.run(6.0); });

    thread1.join();
    thread2.join();
    thread3.join();
    EXPECT_EQ(ping1.pings, 2);
    EXPECT_EQ(ping2.pings, 3);
    EXPECT_EQ(ping3.pings, 1);
    EXPECT_EQ(ping1.pongs, 3);
    EXPECT_EQ(ping2.pongs, 1);
    EXPECT_EQ(ping3.pongs, 2);
}

INSTANTIATE_TEST_SUITE_P(mfed_add_tests,
                         mfed_add_single_type_tests,
                         ::testing::ValuesIn(CoreTypes_single),
                         testNamer);
INSTANTIATE_TEST_SUITE_P(mfed_add_tests,
                         mfed_add_type_tests,
                         ::testing::ValuesIn(CoreTypes),
                         testNamer);
INSTANTIATE_TEST_SUITE_P(mfed_add_tests,
                         mfed_add_all_type_tests,
                         ::testing::ValuesIn(CoreTypes_all),
                         testNamer);

static constexpr const char* config_files[] = {"example_message_fed.json",
                                               "example_message_fed.toml",
                                               "example_message_fed_helics.json",
                                               "example_message_fed_helics2.json"};

class mfed_file_config_files:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(mfed_file_config_files, info_file_load)
{
    helics::FederateInfo fedInfo = helics::loadFederateInfo(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(fedInfo.coreInitString, " --autobroker");
}

TEST_P(mfed_file_config_files, test_file_load)
{
    helics::MessageFederate mFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(mFed.getName(), "messageFed");

    EXPECT_EQ(mFed.getEndpointCount(), 2);
    auto& ept1 = mFed.getEndpoint("ept1");
    EXPECT_EQ(ept1.getExtractionType(), "genmessage");
    EXPECT_EQ(ept1.getInfo(), "this is an information string for use by the application");

    EXPECT_EQ(mFed.query("global_value", "global1"), "this is a global1 value");
    EXPECT_EQ(mFed.query("global_value", "global2"), "this is another global value");
    mFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(mfed_add_tests, mfed_file_config_files, ::testing::ValuesIn(config_files));

static constexpr const char* filter_config_files[] = {"example_filters.json",
                                                      "example_filters.toml"};

class mfed_file_filter_config_files:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(mfed_file_filter_config_files, test_file_load_filter)
{
    helics::MessageFederate mFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(mFed.getName(), "filterFed");

    EXPECT_EQ(mFed.getEndpointCount(), 3);
    auto& ept1 = mFed.getEndpoint("ept1");
    EXPECT_EQ(ept1.getExtractionType(), "genmessage");

    EXPECT_EQ(mFed.getFilterCount(), 3);

    auto filt = &mFed.getFilter(2);

    auto cloneFilt = dynamic_cast<helics::CloningFilter*>(filt);
    EXPECT_TRUE(cloneFilt != nullptr);

    EXPECT_EQ(mFed.getFilter(0).getInfo(),
              "this is an information string for use by the application");
    auto core = mFed.getCorePointer();
    mFed.disconnect();
    core->disconnect();
    core.reset();
}

INSTANTIATE_TEST_SUITE_P(mfed_add_tests,
                         mfed_file_filter_config_files,
                         ::testing::ValuesIn(filter_config_files));

TEST_F(mfed_tests, send_message1)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");
    auto& ep2 = mFed1->registerGlobalEndpoint("ep2");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingMode();

    ep1.sendTo(message1.c_str(), 26, "ep2");

    mFed1->requestNextStep();

    auto message = ep2.getMessage();
    EXPECT_EQ(message->data.size(), 26U);

    ep1.sendToAt(message1.c_str(), 31, "ep2", 1.7);

    auto res = mFed1->requestTime(2.0);
    EXPECT_EQ(res, 1.7);
    message = ep2.getMessage();
    EXPECT_EQ(message->data.size(), 31U);

    mFed1->finalize();
}

TEST(messageFederate, constructor1)
{
    helics::MessageFederate mf1("fed1", "--coretype=test --autobroker --corename=mfc");
    // try out loading a file
    EXPECT_THROW(helics::MessageFederate mf2(std::string("not_available.json")),
                 helics::HelicsException);
    helics::MessageFederate mf2;
    // test move assignment
    mf2 = std::move(mf1);

    EXPECT_FALSE(mf2.hasMessage());
    EXPECT_EQ(mf2.pendingMessageCount(), 0);

    EXPECT_FALSE(mf2.getMessage());

    auto& ept1 = mf2.registerEndpoint();
    EXPECT_FALSE(mf2.hasMessage(ept1));
    EXPECT_EQ(mf2.pendingMessageCount(ept1), 0);
    auto message = mf2.getMessage(ept1);
    EXPECT_FALSE(message);

    EXPECT_THROW(ept1.send(std::move(message)), helics::InvalidFunctionCall);

    mf2.enterExecutingMode();
    mf2.finalize();

    EXPECT_THROW(mf2.registerInterfaces("invalid.toml"), helics::InvalidParameter);
}

TEST(messageFederate, constructor2)
{
    auto core = helics::CoreFactory::create(helics::CoreType::TEST, "--name=cb --autobroker");
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);
    helics::MessageFederate mf1("fed1", core, fedInfo);

    mf1.registerInterfaces(std::string(TEST_DIR) + "example_message_fed_testb.json");

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();

    core.reset();
}

TEST(messageFederate, constructor3)
{
    helics::CoreApp core("--coretype=test --name=cb2 --autobroker");

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);
    helics::MessageFederate mf1("fed1", core, fedInfo);

    mf1.registerInterfaces(std::string(TEST_DIR) + "example_message_fed_testb.json");
    EXPECT_TRUE(core.isConnected());

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();
    EXPECT_NO_THROW(core.getCopyofCorePointer()->waitForDisconnect());
}

TEST(messageFederate, constructor4)
{
    helics::MessageFederate mf1("fed1", std::string(TEST_DIR) + "example_message_fed_testb.json");

    mf1.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_THROW(mf1.registerInterfaces(std::string(TEST_DIR) + "example_message_fed_bad.toml"),
                 helics::HelicsException);
    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();
}

TEST(messageFederate, constructorWithSpaceInFilename)
{
    helics::MessageFederate mf1("fed1",
                                std::string(TEST_DIR) + "example message fed with space.json");

    mf1.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();
}

TEST(messageFederate, constructor5)
{
    helics::MessageFederate mf1("--coretype=test --autobroker --corename=mfc5 --name=fedmd");
    // try out loading a file

    EXPECT_EQ(mf1.getName(), "fedmd");
    EXPECT_EQ(mf1.getCorePointer()->getIdentifier(), "mfc5");
    mf1.enterExecutingMode();
    mf1.finalize();
}

TEST_F(mfed_tests, message_warnings)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    std::atomic<int> warnings{0};

    mFed1->setLoggingCallback(
        [&warnings](int level, std::string_view /*ignored*/, std::string_view /*ignored*/) {
            if (level <= HELICS_LOG_LEVEL_WARNING) {
                ++warnings;
            }
        });

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingMode();

    ep1.sendTo(message1.c_str(), 26, "unknown");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(2.0);
    EXPECT_EQ(warnings.load(), 1);

    ep1.sendTo(message1.c_str(), 26, "unknown2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(3.0);
    EXPECT_EQ(warnings.load(), 2);

    mFed1->finalize();
}

TEST_F(mfed_tests, message_warnings_ignore)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    std::atomic<int> warnings{0};

    mFed1->setLoggingCallback(
        [&warnings](int level, std::string_view /*ignored*/, std::string_view /*ignored*/) {
            if (level <= HELICS_LOG_LEVEL_WARNING) {
                ++warnings;
            }
        });

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");

    helics::Message mess1;
    mess1.data = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    mess1.dest = "unknown";

    mFed1->enterExecutingMode();

    ep1.send(mess1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(2.0);
    EXPECT_EQ(warnings.load(), 1);
    setActionFlag(mess1, helics::optional_flag);
    // it should cause the unknown destination to be ignored
    ep1.send(mess1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(3.0);
    EXPECT_EQ(warnings.load(), 1);

    mFed1->finalize();
}

TEST_F(mfed_tests, message_error)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    std::atomic<int> errors{0};

    mFed1->setLoggingCallback(
        [&errors](int level, std::string_view /*ignored*/, std::string_view /*ignored*/) {
            if (level <= HELICS_LOG_LEVEL_ERROR) {
                ++errors;
            }
        });

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");

    helics::Message mess1;
    mess1.data = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    mess1.dest = "unknown";

    mFed1->enterExecutingMode();

    ep1.send(mess1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(2.0);
    EXPECT_EQ(errors.load(), 0);
    setActionFlag(mess1, helics::required_flag);
    // it should cause the unknown destination to be to generate an error
    ep1.send(mess1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int err_count{0};
    try {
        mFed1->requestTime(3.0);
    }
    catch (...) {
        ++err_count;
    }
    EXPECT_EQ(errors.load(), 1);
    EXPECT_GT(err_count, 0);
    mFed1->finalize();
}

TEST_F(mfed_tests, default_endpoint_required)
{
    SetupTest<helics::MessageFederate>("test", 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    mFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED, true);
    std::atomic<int> errors{0};

    mFed1->setLoggingCallback(
        [&errors](int level, std::string_view /*ignored*/, std::string_view /*ignored*/) {
            if (level <= HELICS_LOG_LEVEL_ERROR) {
                ++errors;
            }
        });

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");

    mFed1->enterExecutingMode();

    ep1.sendTo("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "unknown");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int err_count{0};
    try {
        mFed1->requestTime(1.0);
    }
    catch (...) {
        ++err_count;
    }
    EXPECT_EQ(errors.load(), 1);
    EXPECT_GT(err_count, 0);
    mFed1->finalize();
}

TEST_F(mfed_tests, message_init_iteration)
{
    SetupTest<helics::MessageFederate>("test", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");

    auto& ep2 = mFed2->registerGlobalEndpoint("ep2");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterInitializingModeAsync();
    mFed2->enterInitializingMode();
    mFed1->enterInitializingModeComplete();

    ep1.sendTo(message1.c_str(), 26, "ep2");
    mFed1->enterExecutingModeAsync();

    auto result = mFed2->enterExecutingMode(helics::IterationRequest::ITERATE_IF_NEEDED);
    EXPECT_EQ(result, helics::IterationResult::ITERATING);

    EXPECT_TRUE(ep2.hasMessage());

    auto message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_LT(message->time, helics::timeZero);
    }
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed2->finalize();

    mFed1->finalize();
}

TEST_F(mfed_tests, missing_endpoint)
{
    using ::testing::HasSubstr;
    SetupTest<helics::MessageFederate>("test", 2);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalEndpoint("ep1");
    auto& ep2 = mFed1->registerGlobalEndpoint("ep2");
    auto& ep3 = mFed1->registerGlobalEndpoint("ep3");
    ep2.setOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);
    ep3.setOption(HELICS_HANDLE_OPTION_CONNECTION_REQUIRED);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    mFed1->setLoggingCallback(
        [&mlog](int level, std::string_view /*unused*/, std::string_view message) {
            mlog.lock()->emplace_back(level, message);
        });

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingModeAsync();

    mFed2->enterExecutingMode();

    mFed1->enterExecutingModeComplete();

    ep1.sendTo(message1.c_str(), 26, "ep92");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(1.0);

    auto logs = mlog.lock();
    EXPECT_EQ(logs->size(), 1U);
    if (!logs->empty()) {
        EXPECT_EQ(logs->front().first, HELICS_LOG_LEVEL_WARNING);
        EXPECT_THAT(logs->front().second, HasSubstr("ep92"));
        EXPECT_THAT(logs->front().second, HasSubstr("ep1"));
    }
    logs.unlock();
    ep2.sendTo(message1.c_str(), 26, "ep92");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mFed1->requestTime(2.0);
    // no warning from this one
    logs = mlog.lock();
    EXPECT_EQ(logs->size(), 1U);
    logs.unlock();
    ep3.sendTo(message1.c_str(), 26, "ep92");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    try {
        mFed1->requestTime(3.0);
    }
    catch (const helics::HelicsException&) {
        ;
    }
    logs = mlog.lock();
    EXPECT_EQ(logs->size(), 2U);
    if (!logs->empty()) {
        EXPECT_EQ(logs->back().first, HELICS_LOG_LEVEL_ERROR);
        EXPECT_THAT(logs->back().second, HasSubstr("ep92"));
        EXPECT_THAT(logs->back().second, HasSubstr("ep3"));
    }
    logs.unlock();
    mFed1->finalize();
}

TEST_F(mfed_tests, targeted_endpoint_send_error1)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");

    auto& ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto& ep3 = mFed2->registerGlobalTargetedEndpoint("ep3");
    auto& ep4 = mFed2->registerGlobalTargetedEndpoint("ep4");

    ep1.addDestinationTarget("ep2");
    ep1.addDestinationTarget("ep4");
    ep2.addDestinationTarget("ep1");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_NO_THROW(ep1.sendTo(message1.c_str(), 26, "ep2"));
    EXPECT_THROW(ep1.sendTo(message1.c_str(), 26, "ep3"), helics::InvalidParameter);
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAdvance(1.0);
    mFed1->requestTimeComplete();

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_FALSE(ep4.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());
    auto message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    mFed2->finalize();
    mFed1->finalize();
}

TEST_F(mfed_tests, targeted_endpoint_send_error2)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");

    auto& ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto& ep3 = mFed2->registerGlobalTargetedEndpoint("ep3");
    auto& ep4 = mFed2->registerGlobalTargetedEndpoint("ep4");

    ep1.addDestinationTarget("ep2");
    ep1.addDestinationTarget("ep4");
    ep2.addDestinationTarget("ep1");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_NO_THROW(ep1.sendToAt(message1.c_str(), 26, "ep2", 0.0));
    EXPECT_THROW(ep1.sendToAt(message1.c_str(), 26, "ep3", 0.0), helics::InvalidParameter);
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAdvance(1.0);
    mFed1->requestTimeComplete();

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_FALSE(ep4.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());
    auto message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    mFed2->finalize();
    mFed1->finalize();
}

TEST_F(mfed_tests, targeted_endpoint_send_error3)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");

    auto& ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto& ep3 = mFed2->registerGlobalTargetedEndpoint("ep3");
    auto& ep4 = mFed2->registerGlobalTargetedEndpoint("ep4");

    ep1.addDestinationTarget("ep2");
    ep1.addDestinationTarget("ep4");
    ep2.addDestinationTarget("ep1");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    helics::Message val;
    val.dest = "ep2";
    val.data = std::string_view(message1.c_str(), 26);
    val.source = "ep1";

    EXPECT_NO_THROW(ep1.send(val));
    val.dest = "ep3";
    EXPECT_THROW(ep1.send(val), helics::InvalidParameter);
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAdvance(1.0);
    mFed1->requestTimeComplete();

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_FALSE(ep4.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());
    auto message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    mFed2->finalize();
    mFed1->finalize();
}

TEST_F(mfed_tests, targeted_endpoint_send_all)
{
    SetupTest<helics::MessageFederate>("test", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");

    auto& ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto& ep3 = mFed2->registerGlobalTargetedEndpoint("ep3");
    auto& ep4 = mFed2->registerGlobalTargetedEndpoint("ep4");

    ep1.addDestinationTarget("ep2");
    ep1.addDestinationTarget("ep4");

    const std::string message1{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    helics::Message val;
    val.data = std::string_view(message1.c_str(), 26);
    val.source = "ep1";

    EXPECT_NO_THROW(ep1.send(val));

    EXPECT_NO_THROW(ep1.sendToAt(message1.c_str(), 26, "", 0.0));
    EXPECT_NO_THROW(ep1.sendTo(message1.c_str(), 26, ""));
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAdvance(1.0);
    mFed1->requestTimeComplete();

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep4.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());
    auto message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    auto message2 = ep4.getMessage();
    if (message2) {
        EXPECT_EQ(message2->data.size(), 26U);
        EXPECT_EQ(message2->time, helics::timeZero);
    }

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep4.hasMessage());
    message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    message2 = ep4.getMessage();
    if (message2) {
        EXPECT_EQ(message2->data.size(), 26U);
        EXPECT_EQ(message2->time, helics::timeZero);
    }

    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep4.hasMessage());
    message = ep2.getMessage();
    if (message) {
        EXPECT_EQ(message->data.size(), 26U);
        EXPECT_EQ(message->time, helics::timeZero);
    }

    message2 = ep4.getMessage();
    if (message2) {
        EXPECT_EQ(message2->data.size(), 26U);
        EXPECT_EQ(message2->time, helics::timeZero);
    }

    mFed2->finalize();
    mFed1->finalize();
}

TEST_F(mfed_tests, reentrant_fed_endpoint)
{
    extraBrokerArgs = "--dynamic";
    extraFederateArgs = " --reentrant";
    SetupTest<helics::MessageFederate>("test_2", 2, 1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);

    auto& ept1 = mFed1->registerGlobalTargetedEndpoint("ept1");

    auto& ept2 = mFed2->registerGlobalTargetedEndpoint("ept2");
    ept1.setOption(HELICS_HANDLE_OPTION_RECONNECTABLE);

    ept1.addDestinationEndpoint("ept2");
    ept1.addSourceEndpoint("ept2");

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    mFed1->requestTimeAsync(helics::timeZero);
    mFed2->requestNextStep();
    auto tres = mFed1->requestTimeComplete();
    EXPECT_EQ(tres, 1.0);

    ept1.send("test_from1");
    ept2.send("test_from2");
    mFed1->requestTimeAsync(helics::timeZero);
    mFed2->requestNextStep();
    tres = mFed1->requestTimeComplete();
    EXPECT_EQ(tres, 2.0);
    // check the messages
    EXPECT_TRUE(ept1.hasMessage());
    EXPECT_TRUE(ept2.hasMessage());
    auto message = ept1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test_from2");
    }

    message = ept2.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test_from1");
    }
    // close and reconnect federate
    std::string mFed2Name = mFed2->getName();
    mFed2->disconnect();
    tres = mFed1->requestTime(helics::timeZero);
    EXPECT_LE(tres, 3.0);

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.loadInfoFromArgs(std::string("--reentrant --force_new_core --dynamic --broker=") +
                             brokers[0]->getIdentifier() + " --period=1.0s");

    helics::MessageFederate mFed2Redo(mFed2Name, fedInfo);
    EXPECT_EQ(mFed2Redo.getTimeProperty(HELICS_PROPERTY_TIME_PERIOD), 1.0);
    auto& ept2redo = mFed2Redo.registerGlobalTargetedEndpoint("ept2");

    mFed2Redo.enterExecutingMode();
    tres = mFed2Redo.getCurrentTime();
    EXPECT_LE(tres, 3.0);
    ept2redo.send("test_from2_part2");

    mFed2Redo.requestTimeAsync(helics::timeZero);
    mFed1->requestTimeAsync(helics::timeZero);

    tres = mFed2Redo.requestTimeComplete();

    EXPECT_EQ(tres, 3.0);

    mFed2Redo.requestTimeAsync(helics::timeZero);
    tres = mFed1->requestTimeComplete();

    ept1.send("test_from1_part2");

    EXPECT_EQ(tres, 4.0);
    // check the new messages
    EXPECT_TRUE(ept1.hasMessage());

    message = ept1.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test_from2_part2");
    }

    mFed1->disconnect();

    tres = mFed2Redo.requestTimeComplete();
    EXPECT_EQ(tres, 4.0);

    tres = mFed2Redo.requestNextStep();
    EXPECT_EQ(tres, 5.0);
    EXPECT_TRUE(ept2redo.hasMessage());
    message = ept2redo.getMessage();
    if (message) {
        EXPECT_EQ(message->to_string(), "test_from1_part2");
    }
    mFed2Redo.disconnect();
}
