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
