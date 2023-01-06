/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"

#include <gtest/gtest.h>

class combofed_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class combofed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class combofed: public ::testing::Test, public FederateTestFixture {};

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

// const std::string CoreTypes[] = {"udp" };
/** test simple creation and destruction*/
TEST_P(combofed_single_type_tests, initialize_tests)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    vFed1->disconnect();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(combofed_single_type_tests, publication_registration)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pubid = vFed1->registerPublication<std::string>("pub1");
    auto& pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pubid3 = vFed1->registerPublication("pub3", "double", "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto& sv = pubid.getName();
    auto& sv2 = pubid2.getName();
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    auto& pub3name = pubid3.getName();
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(pubid3.getExtractionType(), "double");
    EXPECT_EQ(pubid3.getUnits(), "V");

    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pubid.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pubid2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pubid.getHandle());
    vFed1->disconnect();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(combofed_single_type_tests, single_transfer)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pubid = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pubid.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string s = subid.getString();
    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    pubid.publish("string2");
    // make sure the value is still what we expect
    s = subid.getString();

    EXPECT_EQ(s, "string1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    s = subid.getString();

    EXPECT_EQ(s, "string2");
    vFed1->disconnect();
}

TEST_P(combofed_single_type_tests, endpoint_registration)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed1->registerGlobalEndpoint("ep2", "random");

    mFed1->enterExecutingMode();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto& sv = epid.getName();
    auto& sv2 = epid2.getName();
    EXPECT_EQ(sv, "fed0/ep1");
    EXPECT_EQ(sv2, "ep2");

    EXPECT_EQ(epid.getExtractionType(), "");
    EXPECT_EQ(epid2.getInjectionType(), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("fed0/ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == epid2.getHandle());
    mFed1->disconnect();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(combofed_type_tests, send_receive_2fed)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    mFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    mFed1->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    mFed1->requestTimeAsync(1.0);
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(mFed1->requestTimeComplete(), 1.0);

    auto res = mFed1->hasMessage();
    EXPECT_TRUE(res);
    res = mFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = mFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = mFed1->getMessage(epid);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = mFed2->getMessage(epid2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);
    mFed1->disconnect();
    mFed2->disconnect();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(combofed_type_tests, multimode_transfer)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    cFed1->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);
    cFed2->setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, 1.0);

    auto& epid = cFed1->registerEndpoint("ep1");
    auto& epid2 = cFed2->registerGlobalEndpoint("ep2", "random");

    // register the publications
    auto& pubid = cFed1->registerGlobalPublication<std::string>("pub1");

    auto& subid = cFed2->registerSubscription("pub1");

    cFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    cFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    cFed1->enterExecutingModeAsync();
    cFed2->enterExecutingMode();
    cFed1->enterExecutingModeComplete();
    // publish string1 at time=0.0;
    pubid.publish("string1");

    EXPECT_TRUE(cFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    EXPECT_TRUE(cFed2->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    helics::SmallBuffer data(500, 'a');
    helics::SmallBuffer data2(400, 'b');

    epid.sendTo(data, "ep2");
    epid2.sendTo(data2, "fed0/ep1");
    // move the time to 1.0
    cFed1->requestTimeAsync(1.0);
    auto gtime = cFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(cFed1->requestTimeComplete(), 1.0);

    std::string s = subid.getString();
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    pubid.publish("string2");
    // make sure the value is still what we expect
    s = subid.getString();

    EXPECT_EQ(s, "string1");

    auto res = cFed1->hasMessage();
    EXPECT_TRUE(res);
    res = cFed1->hasMessage(epid);
    EXPECT_TRUE(res);
    res = cFed2->hasMessage(epid2);
    EXPECT_TRUE(res);

    auto M1 = cFed1->getMessage(epid);
    ASSERT_EQ(M1->data.size(), data2.size());

    EXPECT_EQ(M1->data[245], data2[245]);

    auto M2 = cFed2->getMessage(epid2);
    ASSERT_EQ(M2->data.size(), data.size());

    EXPECT_EQ(M2->data[245], data[245]);

    cFed1->requestTimeAsync(1.0);

    gtime = cFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(cFed1->requestTimeComplete(), 2.0);
    // make sure the value was updated

    const auto& ns = subid.getString();

    EXPECT_EQ(ns, "string2");

    cFed1->disconnect();
    cFed2->disconnect();

    EXPECT_TRUE(cFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    EXPECT_TRUE(cFed2->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(combofed_tests,
                         combofed_single_type_tests,
                         ::testing::ValuesIn(CoreTypes_simple),
                         testNamer);
INSTANTIATE_TEST_SUITE_P(combofed_tests,
                         combofed_type_tests,
                         ::testing::ValuesIn(CoreTypes),
                         testNamer);

static constexpr const char* combo_config_files[] = {"example_combo_fed.json",
                                                     "example_combo_fed.toml"};

class combofed_file_load_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(combofed_file_load_tests, test_file_load)
{
    helics::CombinationFederate cFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(cFed.getName(), "comboFed");

    EXPECT_EQ(cFed.getEndpointCount(), 2);
    auto& id = cFed.getEndpoint("ept1");
    EXPECT_EQ(id.getExtractionType(), "genmessage");

    EXPECT_EQ(cFed.getInputCount(), 3);
    EXPECT_EQ(cFed.getPublicationCount(), 2);

    EXPECT_TRUE(!cFed.getPublication(1).getInfo().empty());
    cFed.getCorePointer()->disconnect();
    cFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(combofed_tests,
                         combofed_file_load_tests,
                         ::testing::ValuesIn(combo_config_files));

TEST(comboFederate, constructor2)
{
    auto cr = helics::CoreFactory::create(helics::CoreType::TEST, "--name=mf --autobroker");
    helics::FederateInfo fi(helics::CoreType::TEST);
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);
    helics::CombinationFederate mf1("fed1", cr, fi);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.disconnect();

    cr.reset();
}

TEST(comboFederate, constructor3)
{
    helics::CoreApp cr(helics::CoreType::TEST, "--name=mf2 --autobroker");
    helics::FederateInfo fi(helics::CoreType::TEST);
    fi.setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, HELICS_LOG_LEVEL_ERROR);
    helics::CombinationFederate mf1("fed1", cr, fi);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.disconnect();
    EXPECT_TRUE(cr.waitForDisconnect(std::chrono::milliseconds(500)));
}

TEST_F(combofed, regex_link_anon_inp)
{
    SetupTest<helics::CombinationFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CombinationFederate>(0);
    ASSERT_TRUE(vFed1);
    // register the publications
    vFed1->addAlias("pub1", "publisher");

    auto& inp1 = vFed1->registerDataSink("");

    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerPublication<std::string>("pub2");
    auto& pub3 = vFed1->registerPublication<std::string>("pub3");
    auto& pub4 = vFed1->registerPublication<std::string>("pub4");

    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    inp1.subscribe("REGEX:fed0/.*");
    inp1.setOption(HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD,
                   HELICS_MULTI_INPUT_SUM_OPERATION);

    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    pub2.publish("string2");
    pub3.publish("string3");
    pub4.publish("string4");

    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    // get the value
    EXPECT_EQ(inp1.pendingMessageCount(), 4);

    const auto& str = inp1.getSourceTargets();
    EXPECT_NE(str.find("pub2"), std::string::npos);

    vFed1->finalize();
}
