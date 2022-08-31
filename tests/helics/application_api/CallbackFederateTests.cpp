/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/CallbackFederate.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/core/core-exceptions.hpp"
#include "testFixtures.hpp"

#include <gtest/gtest.h>
#include <future>

class callbackFed: public ::testing::Test, public FederateTestFixture {};

TEST_F(callbackFed, create)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);

   EXPECT_TRUE(vFed1->getFlagOption(HELICS_FLAG_CALLBACK_FEDERATE));
   vFed1->disconnect();
}

// test that the initialize callback gets executed and handles termination properly
TEST_F(callbackFed, initialize)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb{0};
    vFed1->setInitializeCallback([&cb](){++cb;return helics::IterationRequest::HALT_OPERATIONS;});

    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(5); });
    
    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb,1);
    EXPECT_EQ(res,5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}


// test that the initialize callback gets executed and handles termination properly
TEST_F(callbackFed, execute)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb{0};

    vFed1->setExecutingEntryCallback([&cb](){++cb;});
    vFed1->setNextTimeIterativeCallback([](auto /*t*/) {
        return std::make_pair(helics::Time::maxVal(), helics::IterationRequest::HALT_OPERATIONS); }
    );
    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(5); });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb,1);
    EXPECT_EQ(res,5);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

// test that the pre/post time request work properly
TEST_F(callbackFed, timeSteps)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{ 0 };
    int cb2{0};
    helics::Time mTime=3.0;
    vFed1->setTimeRequestReturnCallback([&cb1](auto,bool){++cb1;});
    vFed1->setTimeRequestEntryCallback([&cb2](auto,auto,bool){++cb2;});

    vFed1->setNextTimeIterativeCallback([](auto t) {
        if (t.grantedTime >= 3.0)
        {
            return std::make_pair(helics::Time::maxVal(), helics::IterationRequest::HALT_OPERATIONS);
        }
        else
        {
            return std::make_pair(t.grantedTime+1.0, helics::IterationRequest::NO_ITERATIONS); }
        }
    );
    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(7); });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb1,4);
    EXPECT_EQ(cb2,3);
    EXPECT_EQ(res,7);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}

// test that the max time and period works correctly with the callback federate
TEST_F(callbackFed, timeStepsPeriodMax)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{ 0 };
    int cb2{0};
    helics::Time mTime=3.0;
    vFed1->setProperty(HELICS_PROPERTY_TIME_MAXTIME,3.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    vFed1->setTimeRequestReturnCallback([&cb1](auto,bool){++cb1;});
    vFed1->setTimeRequestEntryCallback([&cb2](auto,auto,bool){++cb2;});

    
    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setCosimulationTerminatedCallback([v = std::move(v)](){v->set_value(8); });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb1,4);
    EXPECT_EQ(cb2,3);
    EXPECT_EQ(res,8);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
}


// test that an error return in the init call routes properly
TEST_F(callbackFed, errorInit)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    vFed1->setInitializeCallback([](){return helics::IterationRequest::ERROR_CONDITION;});

    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();
    vFed1->setErrorHandlerCallback([v = std::move(v)](int code, std::string_view /*message*/){
        v->set_value(code);
    });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(res,HELICS_USER_EXCEPTION);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(),helics::Federate::Modes::ERROR_STATE);
}


// test that an error is thrown in the init call
TEST_F(callbackFed, initException)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    vFed1->setInitializeCallback([]()->helics::IterationRequest{throw std::exception("ETEST");});

    auto  v=std::make_shared<std::promise<std::string>>();
    auto vfut=v->get_future();
    vFed1->setErrorHandlerCallback([v = std::move(v)](int /*code*/, std::string_view message){
        v->set_value(std::string(message));
    });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(res,"ETEST");
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(),helics::Federate::Modes::ERROR_STATE);
}


// test that the pre/post time request work properly
TEST_F(callbackFed, timeStepError)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{ 0 };
    int cb2{0};
    helics::Time mTime=3.0;
    vFed1->setTimeRequestReturnCallback([&cb1](auto,bool){++cb1;});
    vFed1->setTimeRequestEntryCallback([&cb2](auto,auto,bool){++cb2;});

    vFed1->setNextTimeIterativeCallback([](auto t) {
        if (t.grantedTime >= 3.0)
        {
            return std::make_pair(helics::Time::maxVal(), helics::IterationRequest::ERROR_CONDITION);
        }
        return std::make_pair(t.grantedTime+1.0, helics::IterationRequest::NO_ITERATIONS); }
    );
    auto  v=std::make_shared<std::promise<int>>();
    auto vfut=v->get_future();

    vFed1->setErrorHandlerCallback([v = std::move(v)](int code, std::string_view /*message*/){
        v->set_value(code);
    });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb1,4);
    EXPECT_EQ(cb2,3);
    EXPECT_EQ(res,HELICS_USER_EXCEPTION);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(),helics::Federate::Modes::ERROR_STATE);
}


// test that the callback exception traps work
TEST_F(callbackFed, timeStepException)
{
    SetupTest<helics::CallbackFederate>("test", 1);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    int cb1{ 0 };
    int cb2{0};
    helics::Time mTime=3.0;
    vFed1->setTimeRequestReturnCallback([&cb1](auto,bool){++cb1;});
    vFed1->setTimeRequestEntryCallback([&cb2](auto,auto,bool){++cb2;});

    vFed1->setNextTimeIterativeCallback([](auto t) {
        if (t.grantedTime >= 3.0)
        {
            throw std::exception("ETEST");
        }

        return std::make_pair(t.grantedTime+1.0, helics::IterationRequest::NO_ITERATIONS); }
    );
    auto  v=std::make_shared<std::promise<std::string>>();
    auto vfut=v->get_future();
    vFed1->setErrorHandlerCallback([v = std::move(v)](int /*code*/, std::string_view message){
        v->set_value(std::string(message));
    });

    vFed1->enterInitializingMode();

    auto res=vfut.get();
    EXPECT_EQ(cb1,4);
    EXPECT_EQ(cb2,3);
    EXPECT_EQ(res,"ETEST");
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_EQ(vFed1->getCurrentMode(),helics::Federate::Modes::ERROR_STATE);
}


// test that the max time and period works correctly with the callback federate
TEST_F(callbackFed, timeSteps2Fed)
{
    SetupTest<helics::CallbackFederate>("test", 2);
    auto vFed1 = GetFederateAs<helics::CallbackFederate>(0);
    auto vFed2 = GetFederateAs<helics::CallbackFederate>(1);
    vFed1->setProperty(HELICS_PROPERTY_TIME_MAXTIME,3.0);
    vFed1->setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_MAXTIME,3.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_PERIOD,1.0);
    auto &p1=vFed1->registerGlobalPublication<double>("pub1");
    auto &p2=vFed1->registerGlobalPublication<double>("pub2");
    auto &s1=vFed1->registerSubscription("pub1");
    auto &s2=vFed1->registerSubscription("pub2");
    s1.setDefault(0.3);
    s2.setDefault(0.4);
    std::vector<double> v1,v2;

    vFed1->setTimeRequestReturnCallback([&](auto time,bool){p1.publish(18.7+static_cast<double>(time));
        v1.push_back(s1.getValue<double>());
        });
    vFed2->setTimeRequestReturnCallback([&](auto time,bool){p2.publish(18.7+static_cast<double>(time));
    v2.push_back(s1.getValue<double>());
        });


    auto term1=std::make_shared<std::promise<int>>();
    auto vfut1=term1->get_future();
    auto term2=std::make_shared<std::promise<int>>();
    auto vfut2=term2->get_future();

    vFed1->setCosimulationTerminatedCallback([term1 = std::move(term1)](){term1->set_value(9); });
    vFed2->setCosimulationTerminatedCallback([term2 = std::move(term2)](){term2->set_value(10); });
    vFed1->enterInitializingMode();
    vFed2->enterInitializingMode();
    auto res1=vfut1.get();
    auto res2=vfut2.get();

    EXPECT_EQ(res1,9);
    EXPECT_EQ(res2,10);
    EXPECT_TRUE(vFed1->isAsyncOperationCompleted());
    EXPECT_TRUE(vFed2->isAsyncOperationCompleted());
}
