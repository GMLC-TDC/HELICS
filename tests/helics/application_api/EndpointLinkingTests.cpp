/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CombinationFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/Core.hpp"

#include <gtest/gtest.h>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif

#include <future>
#include <iostream>
#include <thread>

/** these test cases test out the message federates
*/
class mfed_tests: public ::testing::Test, public FederateTestFixture {};
/** test simple creation and destruction*/

static const auto testNamer = [](const ::testing::TestParamInfo<const char*>& parameter) {
    return std::string(parameter.param);
};

TEST_F(mfed_tests, regex1)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep2.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}



TEST_F(mfed_tests, regex2)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep2.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}


TEST_F(mfed_tests, regex3)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addSourceTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message1");
    ep2.send("test message2");
    ep3.send("test message3");
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ep1.pendingMessageCount(),3);
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message1");
    }

    m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message2");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message3");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}


TEST_F(mfed_tests, regex_data_sink)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    auto ds= mFed1->registerDataSink("ds1");

    ds.addSourceTarget("REGEX:.*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    
    ep3.send("test message3");
    ep1.send("test message1");
    ep2.send("test message2");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ds.pendingMessageCount(),3);
    EXPECT_FALSE(ep1.hasMessage());
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message1");
    }

    m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message2");
    }

    m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message3");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}

// test combinationFederate linking with value Fed
TEST_F(mfed_tests, regex_combo1)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep2.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}



TEST_F(mfed_tests, regex_combo2)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addDestinationTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_TRUE(ep1.hasMessage());
    EXPECT_TRUE(ep2.hasMessage());
    EXPECT_TRUE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep2.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}


TEST_F(mfed_tests, regex_combo3)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    ep1.addSourceTarget("REGEX:ep.");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();

    ep1.send("test message1");
    ep2.send("test message2");
    ep3.send("test message3");
    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ep1.pendingMessageCount(),3);
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message1");
    }

    m=ep1.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message2");
    }

    m=ep3.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message3");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}


TEST_F(mfed_tests, regex_combo_data_sink)
{
    SetupTest<helics::MessageFederate>("test", 3,1.0);
    auto mFed1 = GetFederateAs<helics::MessageFederate>(0);
    auto mFed2 = GetFederateAs<helics::MessageFederate>(1);
    auto mFed3 = GetFederateAs<helics::MessageFederate>(2);

    auto ep1 = mFed1->registerGlobalTargetedEndpoint("ep1");
    auto ep2 = mFed2->registerGlobalTargetedEndpoint("ep2");
    auto ep3 = mFed3->registerGlobalTargetedEndpoint("ep3");

    auto ds= mFed1->registerDataSink("ds1");

    ds.addSourceTarget("REGEX:.*");

    mFed1->enterExecutingModeAsync();
    mFed3->enterExecutingModeAsync();
    mFed2->enterExecutingMode();
    mFed1->enterExecutingModeComplete();
    mFed3->enterExecutingModeComplete();


    ep3.send("test message3");
    ep1.send("test message1");
    ep2.send("test message2");

    mFed1->requestTimeAsync(1.0);
    mFed2->requestTimeAsync(1.0);
    auto gtime = mFed3->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    gtime=mFed1->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);
    gtime=mFed2->requestTimeComplete();
    EXPECT_EQ(gtime, 1.0);

    EXPECT_EQ(ds.pendingMessageCount(),3);
    EXPECT_FALSE(ep1.hasMessage());
    EXPECT_FALSE(ep2.hasMessage());
    EXPECT_FALSE(ep3.hasMessage());

    auto m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message1");
    }

    m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message2");
    }

    m=ds.getMessage();
    if (m)
    {
        EXPECT_EQ(m->to_string(),"test message3");
    }


    mFed1->finalizeAsync();
    mFed2->finalizeAsync();
    mFed3->finalize();
    mFed2->finalizeComplete();
    mFed1->finalizeComplete();

}
