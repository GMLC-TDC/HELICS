/*
Copyright (c) 2017-2021,
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

#include <future>
#include <gtest/gtest.h>

class combofed_single_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class combofed_type_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

// const std::string core_types[] = {"udp" };
/** test simple creation and destruction*/
TEST_P(combofed_single_type_tests, initialize_tests)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(combofed_single_type_tests, publication_registration)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pubid = vFed1->registerPublication<std::string>("pub1");
    auto& pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pubid3 = vFed1->registerPublication("pub3", "double", "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    auto& sv = vFed1->getInterfaceName(pubid);
    auto& sv2 = vFed1->getInterfaceName(pubid2);
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    auto& pub3name = vFed1->getInterfaceName(pubid3);
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(vFed1->getExtractionType(pubid3), "double");
    EXPECT_EQ(vFed1->getInterfaceUnits(pubid3), "V");

    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pubid.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pubid2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pubid.getHandle());
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(combofed_single_type_tests, single_transfer)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pubid = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    vFed1->publish(pubid, "string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    // get the value
    std::string s = vFed1->getString(subid);
    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    vFed1->publish(pubid, "string2");
    // make sure the value is still what we expect
    s = vFed1->getString(subid);

    EXPECT_EQ(s, "string1");
    // advance time
    gtime = vFed1->requestTime(2.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    s = vFed1->getString(subid);

    EXPECT_EQ(s, "string2");
}

TEST_P(combofed_single_type_tests, endpoint_registration)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 1);
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
    EXPECT_EQ(mFed1->getInjectionType(epid2), "random");

    EXPECT_TRUE(mFed1->getEndpoint("ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("fed0/ep1").getHandle() == epid.getHandle());
    EXPECT_TRUE(mFed1->getEndpoint("ep2").getHandle() == epid2.getHandle());
    mFed1->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(combofed_type_tests, send_receive_2fed)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto mFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto mFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& epid = mFed1->registerEndpoint("ep1");
    auto& epid2 = mFed2->registerGlobalEndpoint("ep2", "random");

    mFed1->setProperty(helics_property_time_delta, 1.0);
    mFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { mFed1->enterExecutingMode(); });
    mFed2->enterExecutingMode();
    f1finish.wait();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    mFed1->sendMessage(epid, "ep2", data);
    mFed2->sendMessage(epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return mFed1->requestTime(1.0); });
    auto gtime = mFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

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
    mFed1->finalize();
    mFed2->finalize();

    EXPECT_TRUE(mFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(mFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(combofed_type_tests, multimode_transfer)
{
    SetupTest<helics::CombinationFederate>(GetParam(), 2);
    auto cFed1 = GetFederateAs<helics::CombinationFederate>(0);
    auto cFed2 = GetFederateAs<helics::CombinationFederate>(1);

    auto& epid = cFed1->registerEndpoint("ep1");
    auto& epid2 = cFed2->registerGlobalEndpoint("ep2", "random");

    // register the publications
    auto& pubid = cFed1->registerGlobalPublication<std::string>("pub1");

    auto& subid = cFed2->registerSubscription("pub1");

    cFed1->setProperty(helics_property_time_delta, 1.0);
    cFed2->setProperty(helics_property_time_delta, 1.0);

    auto f1finish = std::async(std::launch::async, [&]() { cFed1->enterExecutingMode(); });
    cFed2->enterExecutingMode();
    f1finish.wait();
    // publish string1 at time=0.0;
    cFed1->publish(pubid, "string1");

    EXPECT_TRUE(cFed1->getCurrentMode() == helics::Federate::modes::executing);
    EXPECT_TRUE(cFed2->getCurrentMode() == helics::Federate::modes::executing);

    helics::data_block data(500, 'a');
    helics::data_block data2(400, 'b');

    cFed1->sendMessage(epid, "ep2", data);
    cFed2->sendMessage(epid2, "fed0/ep1", data2);
    // move the time to 1.0
    auto f1time = std::async(std::launch::async, [&]() { return cFed1->requestTime(1.0); });
    auto gtime = cFed2->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time.get(), 1.0);

    std::string s = cFed2->getString(subid);
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    cFed1->publish(pubid, "string2");
    // make sure the value is still what we expect
    s = cFed2->getString(subid);

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

    // advance time
    f1time = std::async(std::launch::async, [&]() { return cFed1->requestTime(2.0); });
    gtime = cFed2->requestTime(2.0);

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time.get(), 2.0);
    // make sure the value was updated

    auto& ns = cFed2->getString(subid);

    EXPECT_EQ(ns, "string2");

    cFed1->finalize();
    cFed2->finalize();

    EXPECT_TRUE(cFed1->getCurrentMode() == helics::Federate::modes::finalize);
    EXPECT_TRUE(cFed2->getCurrentMode() == helics::Federate::modes::finalize);
}

INSTANTIATE_TEST_SUITE_P(combofed_tests,
                         combofed_single_type_tests,
                         ::testing::ValuesIn(core_types_simple));
INSTANTIATE_TEST_SUITE_P(combofed_tests, combofed_type_tests, ::testing::ValuesIn(core_types));

static constexpr const char* combo_config_files[] = {"example_combo_fed.json",
                                                     "example_combo_fed.toml"};

class combofed_file_load_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(combofed_file_load_tests, test_file_load)
{
    helics::CombinationFederate cFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(cFed.getName(), "comboFed");

    EXPECT_EQ(cFed.getEndpointCount(), 2);
    auto& id = cFed.getEndpoint("ept1");
    EXPECT_EQ(cFed.getExtractionType(id), "genmessage");

    EXPECT_EQ(cFed.getInputCount(), 2);
    EXPECT_EQ(cFed.getPublicationCount(), 2);

    EXPECT_TRUE(!cFed.getPublication(1).getInfo().empty());
    cFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(combofed_tests,
                         combofed_file_load_tests,
                         ::testing::ValuesIn(combo_config_files));

TEST(comboFederate, constructor2)
{
    auto cr = helics::CoreFactory::create(helics::core_type::TEST, "--name=mf --autobroker");
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.setProperty(helics_property_int_log_level, helics_log_level_error);
    helics::CombinationFederate mf1("fed1", cr, fi);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();

    cr.reset();
}

TEST(comboFederate, constructor3)
{
    helics::CoreApp cr(helics::core_type::TEST, "--name=mf2 --autobroker");
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.setProperty(helics_property_int_log_level, helics_log_level_error);
    helics::CombinationFederate mf1("fed1", cr, fi);

    mf1.registerGlobalFilter("filt1");
    mf1.registerGlobalFilter("filt2");

    EXPECT_NO_THROW(mf1.enterExecutingMode());
    mf1.finalize();
    EXPECT_TRUE(cr.waitForDisconnect(std::chrono::milliseconds(500)));
}
