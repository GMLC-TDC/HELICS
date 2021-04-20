/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../src/helics/cpp98/Broker.hpp"
#include "../src/helics/cpp98/Core.hpp"
#include "../src/helics/cpp98/ValueFederate.hpp"
#include "cpptestFixtures.hpp"

#include <gtest/gtest.h>

/** these test cases test out the value federates
 */

class multiInput: public ::testing::Test, public FederateTestFixture_cpp {
};
using namespace helicscpp;
TEST_F(multiInput, order)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    vFed1->enterExecutingMode();

    EXPECT_STREQ(in1.getPublicationType(), "double");

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 5.0);
    vFed1->finalize();
}

/** swap the order from previous test to make sure it works fine in this context*/
TEST_F(multiInput, order2)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    vFed1->enterExecutingMode();
    pub2.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub1.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub1.publish(4.0);
    pub2.publish(5.0);
    EXPECT_EQ(in1.getOption(helics_handle_option_connections), 2);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}

TEST_F(multiInput, priority)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.setOption(helics_handle_option_input_priority_location, 1);
    vFed1->enterExecutingMode();

    EXPECT_STREQ(in1.getPublicationType(), "double");

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(3.0);
    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 3.0);

    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 4.0);
    auto fval = in1.getOption(helics_handle_option_clear_priority_list);
    EXPECT_EQ(fval, 0);
    in1.setOption(helics_handle_option_clear_priority_list);

    pub2.publish(6.0);
    pub1.publish(7.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 7.0);
    fval = in1.getOption(helics_handle_option_clear_priority_list);
    EXPECT_NE(fval, 0);

    vFed1->finalize();
}

TEST_F(multiInput, max)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_max_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.7);
    pub2.publish(3.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 3.0);
    pub3.publish(6.0);
    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 6.0);
    vFed1->finalize();
}

TEST_F(multiInput, min)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_min_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.7);
    pub2.publish(3.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 1.7);
    pub3.publish(6.0);
    pub2.publish(4.0);
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}

TEST_F(multiInput, and)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "boolean");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "boolean");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "boolean");

    auto in1 = vFed1->registerInput("", "boolean");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_and_operation);
    vFed1->enterExecutingMode();

    pub1.publish(true);
    vFed1->requestNextStep();
    auto val = in1.getBoolean();
    EXPECT_TRUE(val);
    pub3.publish(false);
    pub2.publish(true);

    vFed1->requestNextStep();
    val = in1.getBoolean();
    EXPECT_FALSE(val);
    pub3.publish(true);
    pub2.publish(true);
    pub1.publish(true);

    vFed1->requestNextStep();
    val = in1.getBoolean();
    EXPECT_TRUE(val);
    vFed1->finalize();
}

TEST_F(multiInput, or)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "boolean");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "boolean");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "boolean");

    auto in1 = vFed1->registerInput("", "boolean");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_or_operation);
    vFed1->enterExecutingMode();

    pub1.publish(true);
    vFed1->requestNextStep();
    auto val = in1.getBoolean();
    EXPECT_TRUE(val);
    pub3.publish(false);
    pub2.publish(true);

    vFed1->requestNextStep();
    val = in1.getBoolean();
    EXPECT_TRUE(val);
    pub3.publish(true);
    pub2.publish(true);
    pub1.publish(true);

    vFed1->requestNextStep();
    val = in1.getBoolean();
    EXPECT_TRUE(val);

    pub3.publish(false);
    pub2.publish(false);
    pub1.publish(false);

    vFed1->requestNextStep();
    val = in1.getBoolean();
    EXPECT_FALSE(val);
    vFed1->finalize();
}

TEST_F(multiInput, sum)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_sum_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 4.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 17.0);
    pub3.publish(6.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 18.0);
    vFed1->finalize();
}

TEST_F(multiInput, average)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_average_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 3.0);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}

TEST_F(multiInput, diff)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_diff_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(1.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub1.publish(4.0);
    pub2.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, -1.0);
    vFed1->finalize();
}

TEST_F(multiInput, vectorize)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double");

    auto in1 = vFed1->registerInput("", "double");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    std::vector<double> val;
    in1.getVector(val);
    EXPECT_EQ(val.size(), 1U);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    in1.getVector(val);
    EXPECT_EQ(val.size(), 6U);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    in1.getVector(val);
    EXPECT_EQ(val.size(), 4U);
    vFed1->finalize();
}

TEST_F(multiInput, vectorize_string)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "string");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "string");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "string");

    auto in1 = vFed1->registerInput("", "string");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish("test1");
    vFed1->requestNextStep();
    auto val = in1.getString();
    EXPECT_EQ(val, "[ \"test1\" ]");
    pub3.publish("test3");
    pub2.publish("test2");

    vFed1->requestNextStep();
    val = in1.getString();
    auto aloc1 = val.find("test1");
    auto aloc2 = val.find("test2");
    auto aloc3 = val.find("test3");

    EXPECT_GT(aloc2, aloc1);
    EXPECT_GT(aloc3, aloc2);

    vFed1->finalize();
}

TEST_F(multiInput, vectorizeComplex)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "complex_vector");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "complex_vector");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "complex_vector");

    auto in1 = vFed1->registerInput("", "complex_vector");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    std::vector<double> val;
    in1.getVector(val);
    EXPECT_EQ(val.size(), 2U);
    EXPECT_DOUBLE_EQ(val[0], 2.0);
    EXPECT_DOUBLE_EQ(val[1], 0.0);
    pub3.publish(std::complex<double>{3.0, -1.2});
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    in1.getVector(val);
    EXPECT_EQ(val.size(), 8U);
    EXPECT_DOUBLE_EQ(val[2], 3.0);
    EXPECT_DOUBLE_EQ(val[3], 5.0);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    in1.getVector(val);
    EXPECT_EQ(val.size(), 6U);
    EXPECT_DOUBLE_EQ(val[0], 5.0);
    EXPECT_DOUBLE_EQ(val[1], 0.0);
    EXPECT_DOUBLE_EQ(val[4], 4.0);
    EXPECT_DOUBLE_EQ(val[5], 0.0);
    vFed1->finalize();
}

TEST_F(multiInput, max_units)
{
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto pub1 = vFed1->registerGlobalPublication("pub1", "double", "m");
    auto pub2 = vFed1->registerGlobalPublication("pub2", "double", "ft");
    auto pub3 = vFed1->registerGlobalPublication("pub3", "double", "in");

    auto in1 = vFed1->registerInput("", "double", "m");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics_handle_option_multi_input_handling_method,
                  helics_multi_input_max_operation);
    vFed1->enterExecutingMode();

    pub1.publish(1.0);
    vFed1->requestNextStep();
    double val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub3.publish(32.0);
    pub2.publish(3.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub3.publish(60.0);
    pub2.publish(4.0);
    pub1.publish(2.0);

    vFed1->requestNextStep();
    val = in1.getDouble();
    EXPECT_DOUBLE_EQ(val, 2.0);
    vFed1->finalize();
}
