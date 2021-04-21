/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/BrokerApp.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/common/JsonProcessingFunctions.hpp"
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
    SetupTest<ValueFederate>("test", 1, 1.0);
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
    auto fval = in1.getOption(defs::options::clear_priority_list);
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
                  helics::multi_input_handling_method::max_operation);
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
                  helics::multi_input_handling_method::min_operation);
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
                  helics::multi_input_handling_method::and_operation);
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
                  helics::multi_input_handling_method::or_operation);
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
    auto& pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::sum_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 4.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 17.0);
    pub3.publish(6.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 18.0);
    vFed1->finalize();
}

TEST_F(multiInput, average)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::options::multi_input_handling_method,
                  helics::multi_input_handling_method::average_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 3.0);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 4.0);
    vFed1->finalize();
}

TEST_F(multiInput, diff)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::diff_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    pub2.publish(1.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub1.publish(4.0);
    pub2.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, -1.0);
    vFed1->finalize();
}

TEST_F(multiInput, vectorize)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication("pub2", "vector");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3");

    auto& in1 = vFed1->registerInput<double>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    auto val = in1.getValue<std::vector<double>>();
    EXPECT_EQ(val.size(), 1U);
    pub3.publish(1.0);
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getValue<std::vector<double>>();
    EXPECT_EQ(val.size(), 6U);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<std::vector<double>>();
    EXPECT_EQ(val.size(), 4U);
    vFed1->finalize();
}

TEST_F(multiInput, vectorize_string)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication("pub2", "string");
    auto& pub3 = vFed1->registerGlobalPublication<std::string>("pub3");

    auto& in1 = vFed1->registerInput<std::string>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish("test1");
    vFed1->requestNextStep();
    auto val = in1.getValue<std::string>();
    EXPECT_EQ(val, "[ \"test1\" ]");
    pub3.publish("test3");
    pub2.publish("test2");

    vFed1->requestNextStep();
    val = in1.getValue<std::string>();
    auto jv = loadJsonStr(val);

    EXPECT_EQ(jv.size(), 3U);
    EXPECT_STREQ(jv[0].asCString(), "test1");
    EXPECT_STREQ(jv[1].asCString(), "test2");
    EXPECT_STREQ(jv[2].asCString(), "test3");

    vFed1->finalize();
}

TEST_F(multiInput, vectorizeComplex)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<std::complex<double>>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication("pub2", "complex_vector");
    auto& pub3 = vFed1->registerGlobalPublication<std::complex<double>>("pub3");

    auto& in1 = vFed1->registerInput<std::vector<std::complex<double>>>("");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::vectorize_operation);
    vFed1->enterExecutingMode();

    pub1.publish(2.0);
    vFed1->requestNextStep();
    auto val = in1.getValue<std::vector<std::complex<double>>>();
    EXPECT_EQ(val.size(), 1U);
    pub3.publish(std::complex<double>{3.0, -1.2});
    pub2.publish(std::vector<double>{3.0, 5.0, 5.0, 2.0});

    vFed1->requestNextStep();
    val = in1.getValue<std::vector<std::complex<double>>>();
    EXPECT_EQ(val.size(), 4U);
    pub3.publish(4.0);
    pub2.publish(std::vector<double>{3.0, 4.0});
    pub1.publish(5.0);

    vFed1->requestNextStep();
    val = in1.getValue<std::vector<std::complex<double>>>();
    EXPECT_EQ(val.size(), 3U);
    vFed1->finalize();
}

TEST_F(multiInput, max_units)
{
    using namespace helics;
    SetupTest<ValueFederate>("test", 1, 1.0);
    auto vFed1 = GetFederateAs<ValueFederate>(0);

    auto& pub1 = vFed1->registerGlobalPublication<double>("pub1", "m");
    auto& pub2 = vFed1->registerGlobalPublication<double>("pub2", "ft");
    auto& pub3 = vFed1->registerGlobalPublication<double>("pub3", "in");

    auto& in1 = vFed1->registerInput<double>("", "m");
    in1.addTarget("pub1");
    in1.addTarget("pub2");
    in1.addTarget("pub3");
    in1.setOption(helics::defs::multi_input_handling_method,
                  helics::multi_input_handling_method::max_operation);
    vFed1->enterExecutingMode();

    pub1.publish(1.0);
    vFed1->requestNextStep();
    double val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub3.publish(32.0);
    pub2.publish(3.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 1.0);
    pub3.publish(60.0);
    pub2.publish(4.0);
    pub1.publish(2.0);

    vFed1->requestNextStep();
    val = in1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 2.0);
    vFed1->finalize();
}

TEST_F(multiInput, file_config_json)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + "multi_input_config.json");

    auto& p1 = vFed.getPublication(0);
    auto& p2 = vFed.getPublication(1);
    auto& i1 = vFed.getInput(0);
    vFed.enterExecutingMode();
    auto res = i1.getOption(helics::defs::options::connections);
    EXPECT_EQ(res, 2);
    res = i1.getOption(helics::defs::options::multi_input_handling_method);

    EXPECT_EQ(res, helics_multi_input_average_operation);

    p1.publish(11.3);
    p2.publish(14.7);

    vFed.requestNextStep();

    double val = i1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 13.0);

    vFed.finalize();
}

TEST_F(multiInput, file_config_toml)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + "multi_input_config.toml");

    auto& p1 = vFed.getPublication(0);
    auto& p2 = vFed.getPublication(1);
    auto& i1 = vFed.getInput(0);
    vFed.enterExecutingMode();
    auto res = i1.getOption(helics::defs::options::connections);
    EXPECT_EQ(res, 2);
    res = i1.getOption(helics::defs::options::multi_input_handling_method);

    EXPECT_EQ(res, helics_multi_input_average_operation);

    p1.publish(11.3);
    p2.publish(14.7);

    vFed.requestNextStep();

    double val = i1.getValue<double>();
    EXPECT_DOUBLE_EQ(val, 13.0);

    vFed.finalize();
}
