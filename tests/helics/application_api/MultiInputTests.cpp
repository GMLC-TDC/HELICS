/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/helics_definitions.hpp"

#include <future>
#include <gtest/gtest.h>
#ifndef HELICS_SHARED_LIBRARY
#    include "testFixtures.hpp"
#else
#    include "testFixtures_shared.hpp"
#endif

/** these test cases test out the value federates
 */

class multiInput: public ::testing::Test, public FederateTestFixture {
};

TEST_F(multiInput, order)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1,1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    vFed1->enterExecutingMode();

    EXPECT_EQ(in1.getPublicationType(), "double");

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val=in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 5.0);
    vFed1->finalize();
}

/** swap the order from previous test to make sure it works fine in this context*/
TEST_F(multiInput, order2)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    vFed1->enterExecutingMode();
    pub2.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub1.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub1.publish(4.0);
    pub2.publish(5.0);
    EXPECT_EQ(in1.getOption(defs::connections), 2);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}


TEST_F(multiInput, priority)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.setOption(defs::options::input_priority_location, 1);
    vFed1->enterExecutingMode();


    EXPECT_EQ(in1.getPublicationType(), "double");

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    auto fval=in1.getOption(defs::options::clear_priority_list);
    EXPECT_EQ(fval, 0);
    in1.setOption(defs::options::clear_priority_list);

    pub2.publish(6.0);
    pub1.publish(7.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 7.0);
    fval = in1.getOption(defs::options::clear_priority_list);
    EXPECT_NE(fval, 0);

    vFed1->finalize();
}


TEST_F(multiInput, max)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_mode::max_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.7);
    pub2.publish(3.0);
   
    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    pub3.publish(6.0);
    pub2.publish(4.0);
    pub1.publish(5.0);
    
    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 6.0);
    vFed1->finalize();
}

TEST_F(multiInput, min)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_mode::min_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.7);
    pub2.publish(3.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.7);
    pub3.publish(6.0);
    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}


TEST_F(multiInput, and)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<bool>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<bool>("pub2");
    auto& pub3 = vFed1->registerGlobalPublication<bool>("pub3");

    auto& in1 = vFed1->registerInput<bool>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_mode::and_operation);
    vFed1->enterExecutingMode();

    pub1.publish(true);
    vFed1->requestNextStep();
    auto val = in1.getValue<bool>();
    EXPECT_TRUE(val);
    pub3.publish(false);
    pub2.publish(true);

    vFed1->requestNextStep();
    val = in1.getValue<bool>();
    EXPECT_FALSE(val);
    pub3.publish(true);
    pub2.publish(true);
    pub1.publish(true);

    vFed1->requestNextStep();
    val = in1.getValue<bool>();
    EXPECT_TRUE(val);
    vFed1->finalize();
}



TEST_F(multiInput, or)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<bool>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<bool>("pub2");
    auto& pub3 = vFed1->registerGlobalPublication<bool>("pub3");

    auto& in1 = vFed1->registerInput<bool>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_mode::or_operation);
    vFed1->enterExecutingMode();

    pub1.publish(true);
    vFed1->requestNextStep();
    auto val = in1.getValue<bool>();
    EXPECT_TRUE(val);
    pub3.publish(false);
    pub2.publish(true);

    vFed1->requestNextStep();
    val = in1.getValue<bool>();
    EXPECT_TRUE(val);
    pub3.publish(true);
    pub2.publish(true);
    pub1.publish(true);

    vFed1->requestNextStep();
    val = in1.getValue<bool>();
    EXPECT_TRUE(val);

    pub3.publish(false);
    pub2.publish(false);
    pub1.publish(false);

    vFed1->requestNextStep();
    val = in1.getValue<bool>();
    EXPECT_FALSE(val);
    vFed1->finalize();
}



TEST_F(multiInput, sum)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication("pub2","vector");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_mode::sum_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0,4.0,5.0,2.0});

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 15.0);
    pub3.publish(6.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 18.0);
    vFed1->finalize();
}
