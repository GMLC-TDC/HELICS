/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueFederateTestTemplates.hpp"
#include "helics/application_api/CoreApp.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "testFixtures.hpp"

#include <algorithm>
#include <fstream>
#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/** these test cases test out the value federates with some additional tests
 */

class valuefed_add_single_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class valuefed_add_all_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class valuefed_add_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

class valuefed_add_tests_ci_skip: public ::testing::Test, public FederateTestFixture {};

/** test simple creation and destruction*/
TEST_P(valuefed_add_single_type_tests_ci_skip, initialize)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

#ifdef HELICS_ENABLE_ZMQ_CORE

class valuefed_add_ztype_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(valuefed_add_ztype_tests, publication_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pub2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pub3 = vFed1->registerPublication("pub3", "double", "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    const auto& pubname1 = pub1.getName();
    const auto& pubname2 = pub2.getName();
    EXPECT_EQ(pubname1, "fed0/pub1");
    EXPECT_EQ(pubname2, "pub2");
    const auto& pubname3 = pub3.getName();
    EXPECT_EQ(pubname3, "fed0/pub3");

    EXPECT_EQ(pub3.getExtractionType(), "double");
    EXPECT_EQ(pub3.getExtractionUnits(), "V");

    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pub1.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pub2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pub1.getHandle());
    vFed1->disconnect();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

INSTANTIATE_TEST_SUITE_P(vfed_add_tests, valuefed_add_ztype_tests, ::testing::ValuesIn(ztypes));

#endif

TEST_P(valuefed_add_single_type_tests_ci_skip, publisher_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    helics::Publication pub1(vFed1.get(), "pub1", helics::helicsType<std::string>());
    helics::Publication pub2(helics::InterfaceVisibility::GLOBAL,
                             vFed1.get(),
                             "pub2",
                             helics::DataType::HELICS_INT);

    vFed1->setSeparator('-');
    helics::Publication pubid3(vFed1.get(), "pub3", helics::helicsType<double>(), "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    const auto& pubname1 = pub1.getName();
    const auto& pubname2 = pub2.getName();
    EXPECT_EQ(pubname1, "fed0/pub1");
    EXPECT_EQ(pubname2, "pub2");
    const auto& pub3name = pubid3.getName();
    EXPECT_EQ(pub3name, "fed0-pub3");

    EXPECT_EQ(pubid3.getType(), "double");
    EXPECT_EQ(pubid3.getUnits(), "V");
    vFed1->setSeparator('/');
    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pub1.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pub2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pub1.getHandle());
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
}

TEST_P(valuefed_add_single_type_tests_ci_skip, subscription_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);
    auto& subid = vFed1->registerSubscription("sub1", "V");
    auto& subid2 = vFed1->registerSubscription("sub2");

    auto& subid3 = vFed1->registerSubscription("sub3", "V");
    vFed1->enterExecutingMode();

    // EXPECT_TRUE (vFed->getCurrentMode () == helics::Federate::Modes::EXECUTING);

    auto& target1 = vFed1->getTarget(subid);
    auto& target2 = vFed1->getTarget(subid2);
    EXPECT_EQ(target1, "sub1");
    EXPECT_EQ(target2, "sub2");
    auto& sub3name = vFed1->getTarget(subid3);

    vFed1->addAlias(subid, "Shortcut");
    EXPECT_EQ(sub3name, "sub3");

    EXPECT_EQ(subid3.getExtractionUnits(), "V");

    EXPECT_TRUE(vFed1->getInputByTarget("sub1").getHandle() == subid.getHandle());
    EXPECT_TRUE(vFed1->getInputByTarget("sub2").getHandle() == subid2.getHandle());

    EXPECT_TRUE(vFed1->getInputByTarget("Shortcut").getHandle() == subid.getHandle());

    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, subscription_and_publication_registration)
{
    // testing copy constructor as well in this test
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);
    // register the publications
    auto pub1 = vFed1->registerPublication<std::string>("pub1");
    auto pub2 = vFed1->registerGlobalPublication<int>("pub2");

    auto pub3 = vFed1->registerPublication("pub3", "double", "V");

    // optional

    auto sub1 = vFed1->registerSubscription("sub1", "V");
    auto sub2 = vFed1->registerSubscription("sub2");

    auto sub3 = vFed1->registerSubscription("sub3", "V");
    // enter execution
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    // check subscriptions
    auto target1 = vFed1->getTarget(sub1);
    auto target2 = vFed1->getTarget(sub2);
    EXPECT_EQ(target1, "sub1");
    EXPECT_EQ(target2, "sub2");
    auto sub3name = vFed1->getTarget(sub3);
    EXPECT_EQ(sub3name, "sub3");

    EXPECT_EQ(sub3.getUnits(), "V");

    // check publications

    target1 = pub1.getName();
    target2 = pub2.getName();
    EXPECT_EQ(target1, "fed0/pub1");
    EXPECT_EQ(target2, "pub2");
    const auto& pub3name = pub3.getName();
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(pub3.getExtractionType(), "double");
    EXPECT_EQ(pub3.getUnits(), "V");
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, input_and_publication_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);
    // register the publications
    auto& pub1 = vFed1->registerPublication<std::string>("pub1");
    auto& pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pubid3 = vFed1->registerPublication("pub3", "double", "V");

    // optional
    auto& subid = vFed1->registerInput("sub1", "vector", "V");
    subid.addTarget("pub2");
    auto& subid2 = vFed1->registerGlobalInput("sub2", "double", "volts");

    // enter execution
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);
    // check subscriptions
    EXPECT_EQ(subid.getTarget(), "pub2");
    EXPECT_EQ(subid2.getName(), "sub2");

    EXPECT_EQ(subid.getName(), "fed0/sub1");
    EXPECT_EQ(subid.getName(), "fed0/sub1");
    EXPECT_EQ(vFed1->getTarget(subid), "pub2");

    EXPECT_EQ(subid.getExtractionType(), "vector");
    EXPECT_EQ(subid2.getExtractionType(), "double");

    EXPECT_EQ(subid.getInjectionType(), "int32");

    // check publications

    auto& pubname1 = pub1.getName();
    auto& pubname2 = pubid2.getName();
    EXPECT_EQ(pubname1, "fed0/pub1");
    EXPECT_EQ(pubname2, "pub2");
    auto& pub3name = pubid3.getName();
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(pubid3.getExtractionType(), "double");
    EXPECT_EQ(pubid3.getExtractionUnits(), "V");
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::FINALIZE);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, single_transfer)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed1->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string_view string1 = sub1.getString();
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    string1 = sub1.getString();

    EXPECT_EQ(string1, "string1");
    // advance time
    gtime = vFed1->requestTimeAdvance(1.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    string1 = sub1.getString();

    EXPECT_EQ(string1, "string2");
}

TEST_P(valuefed_add_all_type_tests_ci_skip, dual_transfer_string)
{
    // this one is going to test really ugly strings
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data
    // test case
    decltype(auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString(cstr, sizeof(cstr));
    runDualFederateTest<std::string>(GetParam(),
                                     std::string(86263, '\0'),
                                     specialString,
                                     std::string());
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_vector)
{
    std::vector<double> defVec = {34.3, 24.2};
    std::vector<double> v1Vec = {12.4, 14.7, 16.34, 18.17};
    std::vector<double> v2Vec = {9.9999, 8.8888, 7.7777};
    runDualFederateTestv2<std::vector<double>>(GetParam(), defVec, v1Vec, v2Vec);
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_complex)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> val1 = std::polar(10.0, 0.43);
    std::complex<double> val2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>>(GetParam(), def, val1, val2);
}

TEST_F(valuefed_add_tests_ci_skip, dual_transfer_complex_long)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> val1 = std::polar(10.0, 0.43);
    std::complex<double> val2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>>("test_7", def, val1, val2);
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_types_obj8)
{
    // this is a bizarre string since it contains a \0 and icc 17 can't be used inside a boost data
    // test case
    decltype(auto) cstr = "inside\n\0 of the \0\n functional\r \brelationship of helics\n";
    std::string specialString(cstr, sizeof(cstr));

    runDualFederateTestObj<std::string>(GetParam(),
                                        std::string(86263, '\0'),
                                        specialString,
                                        std::string());
}

TEST_P(valuefed_add_all_type_tests_ci_skip, dual_transfer_types_obj9)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> val1 = std::polar(10.0, 0.43);
    std::complex<double> val2 = {-3e45, 1e-23};
    runDualFederateTestObj<std::complex<double>>(GetParam(), def, val1, val2);
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_types_obj10)
{
    helics::NamedPoint def{"trigger", 0.7};
    helics::NamedPoint val1{"response", -1e-12};
    helics::NamedPoint val2{"variance", 45.23};
    runDualFederateTestObj<helics::NamedPoint>(GetParam(), def, val1, val2);
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_types_obj11)
{
    runDualFederateTestObj<bool>(GetParam(), true, false, true);
}

/** test the callback specification with a vector list*/

TEST_P(valuefed_add_single_type_tests_ci_skip, vector_callback_lists)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto pubid1 = vFed1->registerPublication<std::string>("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto pubid3 = vFed1->registerPublication("pub3", "");

    auto sub1 = vFed1->registerSubscription("fed0/pub1", "");
    auto sub2 = vFed1->registerSubscription("pub2", "");
    auto sub3 = vFed1->registerSubscription("fed0/pub3", "");

    helics::SmallBuffer buffer(547, ';');
    int ccnt = 0;
    // set subscriptions 1 and 2 to have callbacks
    vFed1->setInputNotificationCallback(sub1,
                                        [&](helics::Input& /*unused*/, helics::Time /*unused*/) {
                                            ++ccnt;
                                        });
    vFed1->setInputNotificationCallback(sub2,
                                        [&](helics::Input& /*unused*/, helics::Time /*unused*/) {
                                            ++ccnt;
                                        });
    vFed1->enterExecutingMode();
    vFed1->publishBytes(pubid3, buffer);
    vFed1->requestTime(1.0);
    // callbacks here
    EXPECT_EQ(ccnt, 0);

    pubid1.publish("this is a test");
    vFed1->requestTime(3.0);
    EXPECT_EQ(ccnt, 1);

    ccnt = 0;  // reset the counter
    vFed1->publishBytes(pubid3, buffer);
    pubid2.publish(4);
    pubid1.publish("test string2");
    vFed1->requestTime(5.0);
    EXPECT_EQ(ccnt, 2);

    EXPECT_NEAR(static_cast<double>(vFed1->getLastUpdateTime(sub3)), 3.0, 0.000001);
    vFed1->finalize();
}

/** test the publish/subscribe to a vectorized array*/

TEST_P(valuefed_add_single_type_tests_ci_skip, indexed_pubs_subs)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto pubid1 = vFed1->registerIndexedPublication<double>("pub1", 0);
    auto pubid2 = vFed1->registerIndexedPublication<double>("pub1", 1);

    auto pubid3 = vFed1->registerIndexedPublication<double>("pub1", 2);

    auto sub1 = vFed1->registerIndexedSubscription("pub1", 0);
    auto sub2 = vFed1->registerIndexedSubscription("pub1", 1);
    auto sub3 = vFed1->registerIndexedSubscription("pub1", 2);
    vFed1->enterExecutingMode();

    pubid1.publish(10.0);
    pubid2.publish(20.0);
    pubid3.publish(30.0);
    vFed1->requestTime(2.0);
    auto val1 = sub1.getDouble();
    auto val2 = sub2.getDouble();
    auto val3 = sub3.getDouble();

    EXPECT_NEAR(10.0, val1, 0.00000001);
    EXPECT_NEAR(20.0, val2, 0.00000001);
    EXPECT_NEAR(30.0, val3, 0.00000001);
}

/** test the publish/subscribe to a vectorized array*/

TEST_P(valuefed_add_type_tests_ci_skip, async_calls)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto& pub1 = vFed1->registerGlobalPublication<std::string>("pub1");

    auto& sub1 = vFed2->registerSubscription("pub1");
    vFed1->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);
    vFed2->setProperty(HELICS_PROPERTY_TIME_DELTA, 1.0);

    vFed1->enterExecutingModeAsync();
    EXPECT_TRUE(!vFed1->isAsyncOperationCompleted());
    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    // publish string1 at time=0.0;
    pub1.publish("string1");
    vFed1->requestTimeAsync(1.0);
    vFed2->requestTimeAsync(1.0);

    auto f1time = vFed1->requestTimeComplete();
    auto gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time, 1.0);
    // get the value
    std::string string1 = sub1.getString();

    // make sure the string is what we expect
    EXPECT_EQ(string1, "string1");
    // publish a second string
    pub1.publish("string2");
    // make sure the value is still what we expect
    sub1.getValue(string1);
    EXPECT_EQ(string1, "string1");
    // advance time
    vFed1->requestTimeAsync(2.0);
    vFed2->requestTimeAsync(2.0);
    f1time = vFed1->requestTimeComplete();
    gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time, 2.0);

    // make sure the value was updated

    string1 = sub1.getValue<std::string>();
    EXPECT_EQ(string1, "string2");
    vFed1->finalizeAsync();
    vFed2->finalize();
    vFed1->finalizeComplete();
}

/** test info field for multiple publications */
TEST_P(valuefed_add_single_type_tests_ci_skip, info_field)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto pubid1 = vFed1->registerPublication<std::string>("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");
    pubid1.setInfo(std::string("test1"));
    pubid2.setInfo(std::string("test2"));
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::Modes::EXECUTING);

    auto info1 = pubid1.getInfo();
    auto info2 = pubid2.getInfo();
    EXPECT_EQ(info1, "test1");
    EXPECT_EQ(info2, "test2");

    vFed1->finalize();
}

/** test the pub/sub info field*/
TEST_P(valuefed_add_single_type_tests_ci_skip, info_pubs_subs)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL);
    auto pubid1 = vFed1->registerIndexedPublication<double>("pub1", 0);
    pubid1.setInfo(std::string("pub_test1"));

    auto sub1 = vFed1->registerIndexedSubscription("pub1", 0);
    auto sub2 = vFed1->registerIndexedSubscription("pub1", 1);
    auto sub3 = vFed1->registerIndexedSubscription("pub1", 2);

    sub1.setInfo(std::string("sub_test1"));
    sub2.setInfo(std::string("sub_test2"));
    sub3.setInfo(std::string("sub_test3"));

    vFed1->enterExecutingMode();

    // Check all values can be accessed and returned directly from their subscriptions.
    const auto& info1 = pubid1.getInfo();
    const auto& sub_info2 = sub1.getInfo();
    const auto& sub_info3 = sub2.getInfo();
    const auto& sub_info4 = sub3.getInfo();

    EXPECT_EQ(info1, "pub_test1");
    EXPECT_EQ(sub_info2, "sub_test1");
    EXPECT_EQ(sub_info3, "sub_test2");
    EXPECT_EQ(sub_info4, "sub_test3");

    vFed1->finalize();
}

/** test the default constructor and move constructor and move assignment*/
TEST_F(valuefed_add_tests_ci_skip, test_move_calls)
{
    helics::ValueFederate vFed;

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreInitString = "-f 3 --autobroker";
    vFed = helics::ValueFederate("test1", fedInfo);
    EXPECT_EQ(vFed.getName(), "test1");

    helics::ValueFederate vFedMoved(std::move(vFed));
    EXPECT_EQ(vFedMoved.getName(), "test1");
    // verify that this was moved so this does produce a warning on some systems about use after
    // move
    EXPECT_NE(vFed.getName(), "test1");  // NOLINT
}

static constexpr const char* config_files[] = {"example_value_fed.json",
                                               "example_value_fed.toml",
                                               "example_value_fed_helics1.json",
                                               "example_value_fed_helics2.json",
                                               "example_value_fed_helics_broker.json"};

class valuefed_add_configfile_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {};

TEST_P(valuefed_add_configfile_tests, file_load)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(vFed.getName(), "valueFed");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& inp1 = vFed.getInput("pubshortcut");

    auto& key = vFed.getTarget(inp1);
    EXPECT_EQ(key, "fedName/pub2");

    EXPECT_EQ(inp1.getInfo(), "this is an information string for use by the application");
    auto pub2name = vFed.getPublication(1).getName();
    EXPECT_EQ(pub2name, "valueFed/pub2");
    // test the info from a file
    EXPECT_EQ(vFed.getPublication(0).getInfo(),
              "this is an information string for use by the application");

    EXPECT_EQ(vFed.getInput(2).getName(), "valueFed/ipt2");

    EXPECT_EQ(vFed.query("global_value", "global1"), "this is a global1 value");
    EXPECT_EQ(vFed.query("global_value", "global2"), "this is another global value");

    auto& pub = vFed.getPublication("pub1");
    EXPECT_EQ(pub.getUnits(), "m");
    vFed.disconnect();
}

TEST_P(valuefed_add_configfile_tests, file_load_as_string)
{
    std::ifstream file(std::string(TEST_DIR) + GetParam());
    std::stringstream buffer;
    buffer << file.rdbuf();

    helics::ValueFederate vFed(buffer.str());

    EXPECT_EQ(vFed.getName(), "valueFed");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& inp1 = vFed.getInput("pubshortcut");

    auto key = vFed.getTarget(inp1);
    EXPECT_EQ(key, "fedName/pub2");

    EXPECT_EQ(inp1.getInfo(), "this is an information string for use by the application");
    auto pub2name = vFed.getPublication(1).getName();
    EXPECT_EQ(pub2name, "valueFed/pub2");
    // test the info from a file
    EXPECT_EQ(vFed.getPublication(0).getInfo(),
              "this is an information string for use by the application");

    EXPECT_EQ(vFed.getInput(2).getName(), "valueFed/ipt2");

    EXPECT_EQ(vFed.query("global_value", "global1"), "this is a global1 value");
    EXPECT_EQ(vFed.query("global_value", "global2"), "this is another global value");

    auto& pub = vFed.getPublication("pub1");
    EXPECT_EQ(pub.getUnits(), "m");
    vFed.disconnect();
}

TEST(valuefed_json_tests, file_loadb)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + "example_value_fed_testb.json");

    EXPECT_EQ(vFed.getName(), "valueFed2");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& pub1 = vFed.getPublication("primary");

    EXPECT_EQ(pub1.getName(), "valueFed2/pub2");

    auto& id2 = vFed.getPublication("pub1");
    EXPECT_EQ(id2.getUnits(), "m");

    EXPECT_EQ(id2.getTag("description"), "a test publication");
    EXPECT_EQ(std::stod(id2.getTag("period")), 0.5);

    auto& inp2 = vFed.getInput("ipt2");
    EXPECT_EQ(inp2.getTag("description"), "a test input");
    EXPECT_EQ(std::stod(inp2.getTag("period")), 0.7);

    EXPECT_EQ(vFed.getTag("description"), "fedb description");
    EXPECT_EQ(vFed.getTag("version"), "27");

    auto& sub1 = vFed.getInput(0);
    vFed.enterInitializingMode();

    auto val1 = sub1.getDouble();
    EXPECT_DOUBLE_EQ(val1, 9.33);
    val1 = inp2.getDouble();
    EXPECT_DOUBLE_EQ(val1, 3.67);
    vFed.disconnect();
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(valuefed_json_tests, file_loadb_with_space)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) +
                               "folder with space/example_value_fed_testb.json");

    EXPECT_EQ(vFed.getName(), "valueFed2");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& pub1 = vFed.getPublication("primary");

    EXPECT_EQ(pub1.getName(), "valueFed2/pub2");

    auto& pub2 = vFed.getPublication("pub1");
    EXPECT_EQ(pub2.getUnits(), "m");

    EXPECT_EQ(pub2.getTag("description"), "a test publication");
    EXPECT_EQ(std::stod(pub2.getTag("period")), 0.5);

    auto& inp2 = vFed.getInput("ipt2");
    EXPECT_EQ(inp2.getTag("description"), "a test input");
    EXPECT_EQ(std::stod(inp2.getTag("period")), 0.7);

    EXPECT_EQ(vFed.getTag("description"), "fedb description");
    EXPECT_EQ(vFed.getTag("version"), "27");

    auto& sub1 = vFed.getInput(0);
    vFed.enterInitializingMode();

    auto val1 = sub1.getDouble();
    EXPECT_DOUBLE_EQ(val1, 9.33);
    val1 = inp2.getDouble();
    EXPECT_DOUBLE_EQ(val1, 3.67);
    vFed.disconnect();
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(valuefederate, toml_file_loadb)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + "example_value_fed_testb.toml");

    EXPECT_EQ(vFed.getName(), "valueFed_toml");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& pub2 = vFed.getPublication("primary");

    EXPECT_EQ(pub2.getName(), "valueFed_toml/pub2");

    auto& pub1 = vFed.getPublication("pub1");
    EXPECT_EQ(pub1.getUnits(), "m");
    EXPECT_EQ(pub1.getTag("description"), "a test publication");
    EXPECT_EQ(std::stod(pub1.getTag("period")), 0.5);

    auto& inp2 = vFed.getInput("ipt2");
    EXPECT_EQ(inp2.getTag("description"), "a test input");
    std::string dval = inp2.getTag("period");
    EXPECT_FALSE(dval.empty());
    if (!dval.empty()) {
        EXPECT_EQ(std::stod(dval), 0.7);
    }

    auto& sub1 = vFed.getInput(0);
    vFed.enterExecutingMode();

    auto val1 = sub1.getDouble();
    EXPECT_DOUBLE_EQ(val1, 9.33);
    val1 = inp2.getDouble();
    EXPECT_DOUBLE_EQ(val1, 3.67);
    EXPECT_EQ(vFed.getTag("description"), "fedb description");
    EXPECT_EQ(vFed.getTag("version"), "27");

    vFed.disconnect();
}

TEST(valuefederate, toml_file_bad)
{
    EXPECT_THROW(helics::ValueFederate vFed(std::string(TEST_DIR) + "example_value_fed_bad.toml"),
                 helics::InvalidParameter);
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

INSTANTIATE_TEST_SUITE_P(valuefed,
                         valuefed_add_configfile_tests,
                         ::testing::ValuesIn(config_files));

TEST(valuefed_json_tests, json_publish)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.separator = '/';
    fedInfo.coreName = "json_test2";
    fedInfo.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fedInfo);
    vFed.registerGlobalPublication<double>("pub1");
    vFed.registerPublication<std::string>("pub2");
    vFed.registerPublication<double>("group1/pubA");
    vFed.registerPublication<std::string>("group1/pubB");

    auto& sub1 = vFed.registerSubscription("pub1");
    auto& sub2 = vFed.registerSubscription("test2/pub2");
    auto& sub3 = vFed.registerSubscription("test2/group1/pubA");
    auto& sub4 = vFed.registerSubscription("test2/group1/pubB");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed.requestTime(1.0);
    EXPECT_EQ(sub1.getValue<double>(), 99.9);
    EXPECT_EQ(sub2.getValue<std::string>(), "things");
    EXPECT_EQ(sub3.getValue<double>(), 45.7);
    EXPECT_EQ(sub4.getValue<std::string>(), "count");

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed.requestTime(2.0);
    EXPECT_EQ(sub1.getValue<double>(), 88.2);
    EXPECT_EQ(sub2.getValue<std::string>(), "items");
    EXPECT_EQ(sub3.getValue<double>(), 15.0);
    EXPECT_EQ(sub4.getValue<std::string>(), "count2");

    vFed.publishJSON("{\"pub1\": 77.2}");

    vFed.requestTime(3.0);
    EXPECT_EQ(sub1.getValue<double>(), 77.2);

    vFed.disconnect();
}

TEST(valuefed_json_tests, test_json_register_publish)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.separator = '/';
    fedInfo.coreName = "core_pub_json";
    fedInfo.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fedInfo);

    vFed.registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto& sub1 = vFed.registerSubscription("test2/pub1");
    auto& sub2 = vFed.registerSubscription("test2/pub2");
    auto& sub3 = vFed.registerSubscription("test2/group1/pubA");
    auto& sub4 = vFed.registerSubscription("test2/group1/pubB");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed.requestTime(1.0);
    EXPECT_EQ(sub1.getValue<double>(), 99.9);
    EXPECT_EQ(sub2.getValue<std::string>(), "things");
    EXPECT_EQ(sub3.getValue<double>(), 45.7);
    EXPECT_EQ(sub4.getValue<std::string>(), "count");

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed.requestTime(2.0);
    EXPECT_EQ(sub1.getValue<double>(), 88.2);
    EXPECT_EQ(sub2.getValue<std::string>(), "items");
    EXPECT_EQ(sub3.getValue<double>(), 15.0);
    EXPECT_EQ(sub4.getValue<std::string>(), "count2");

    vFed.disconnect();
}

TEST(valuefed_json_tests, test_json_register_publish_error)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.separator = '/';
    fedInfo.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fedInfo);

    vFed.registerPublication<double>("pub1");
    // this tests an already registered publication
    vFed.registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto& sub1 = vFed.registerSubscription("test2/pub1");
    auto& sub2 = vFed.registerSubscription("test2/pub2");
    auto& sub3 = vFed.registerSubscription("test2/group1/pubA");
    auto& sub4 = vFed.registerSubscription("test2/group1/pubB");

    EXPECT_NO_THROW(vFed.registerFromPublicationJSON("{\"pub3\":45}"));
    auto& sub5 = vFed.registerSubscription("test2/pub3");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    EXPECT_NO_THROW(vFed.publishJSON("{\"pub3\":45}"));
    vFed.requestTime(1.0);
    EXPECT_EQ(sub1.getValue<double>(), 99.9);
    EXPECT_EQ(sub2.getValue<std::string>(), "things");
    EXPECT_EQ(sub3.getValue<double>(), 45.7);
    EXPECT_EQ(sub4.getValue<std::string>(), "count");
    EXPECT_EQ(sub5.getValue<double>(), 45.0);
    vFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(valuefed,
                         valuefed_add_single_type_tests_ci_skip,
                         ::testing::ValuesIn(CoreTypes_single));
INSTANTIATE_TEST_SUITE_P(valuefed, valuefed_add_type_tests_ci_skip, ::testing::ValuesIn(CoreTypes));
INSTANTIATE_TEST_SUITE_P(valuefed,
                         valuefed_add_all_type_tests_ci_skip,
                         ::testing::ValuesIn(CoreTypes_all));

TEST(valuefederate, coreApp)
{
    helics::CoreApp capp(helics::CoreType::TEST, "corename", "-f 1 --autobroker");
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", capp, fedInfo);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());

    Fed1->finalize();
}

TEST(valuefederate, core_ptr)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_ptr";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", nullptr, fedInfo);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::ValueFederate>("vfed2", nullptr, fedInfo),
                 helics::RegistrationFailure);
    Fed1->finalize();
}

TEST(valuefederate, from_file_bad)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    std::string fstr2 = "non_existing.toml";
    EXPECT_THROW(auto fed = std::make_shared<helics::ValueFederate>(fstr2),
                 helics::InvalidParameter);
}

TEST(valuefederate, from_file_bad2)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    auto fstr2 = "non_existing.toml";
    EXPECT_THROW(auto fed = std::make_shared<helics::ValueFederate>(fstr2),
                 helics::InvalidParameter);
}

TEST(valuefederate, from_file_bad3)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_bad_toml";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfedb", fedInfo);

    auto fstr2 = "non_existing.toml";
    EXPECT_THROW(Fed1->registerInterfaces(fstr2), helics::InvalidParameter);
    Fed1->finalize();
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

TEST(valuefederate, pubAlias)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();

    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_alias";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);
    auto& pub1 = Fed1->registerPublication<double>("", "parsecs");

    Fed1->addAlias(pub1, "localPub");

    auto& pub_a = Fed1->getPublication("localPub");

    EXPECT_EQ(pub_a.getUnits(), pub1.getUnits());
    EXPECT_EQ(pub_a.getUnits(), "parsecs");

    Fed1->enterExecutingMode();
    Fed1->finalize();
}

TEST(valuefederate, regJsonFailures)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_pjson";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);

    EXPECT_THROW(Fed1->registerFromPublicationJSON("invalid.json"), helics::InvalidParameter);

    Fed1->enterExecutingMode();
    EXPECT_THROW(Fed1->publishJSON("invalid.json"), helics::InvalidParameter);
    Fed1->finalize();
}

TEST(valuefederate, getInputs)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_ipt";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);

    auto& id1 = Fed1->registerInput("inp1", "double", "V");
    Fed1->enterExecutingMode();

    auto& ip2 = Fed1->getInput("inp1");
    EXPECT_TRUE(ip2.isValid());
    EXPECT_EQ(ip2.getName(), id1.getName());

    const auto& cFed = *Fed1;

    auto& ip3 = cFed.getInput(0);
    EXPECT_TRUE(ip3.isValid());
    EXPECT_EQ(ip3.getName(), id1.getName());

    auto& ip4 = cFed.getInput("inp1");
    EXPECT_TRUE(ip4.isValid());
    EXPECT_EQ(ip4.getName(), id1.getName());

    auto& ip5 = cFed.getInput("vfed1/inp1");
    EXPECT_TRUE(ip5.isValid());
    EXPECT_EQ(ip5.getName(), id1.getName());
    Fed1->finalize();
}

TEST(valuefederate, indexed_inputs)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_indexipt";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);

    auto& id0 = Fed1->registerIndexedInput<double>("inp", 0, "V");
    auto& id1 = Fed1->registerIndexedInput<double>("inp", 1, "V");

    auto& id2 = Fed1->registerIndexedInput<double>("inp", 1, 1, "A");

    Fed1->enterExecutingMode();

    auto& ip2 = Fed1->getInput("inp", 0);
    EXPECT_TRUE(ip2.isValid());
    EXPECT_EQ(ip2.getName(), id0.getName());

    auto& ip3 = Fed1->getInput("inp", 1);
    EXPECT_TRUE(ip3.isValid());
    EXPECT_EQ(ip3.getName(), id1.getName());

    auto& ip4 = Fed1->getInput("inp", 1, 1);
    EXPECT_TRUE(ip4.isValid());
    EXPECT_EQ(ip4.getName(), id2.getName());

    Fed1->finalize();
}

TEST(valuefederate, json)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.useJsonSerialization = true;
    fedInfo.coreName = "core_indexipt";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);

    auto& pub1 = Fed1->registerPublication("pub1", "double", "A");

    auto& id0 = Fed1->registerIndexedInput<double>("inp", 0, "V");
    Fed1->registerIndexedInput<double>("inp", 1, "V");

    Fed1->registerIndexedInput<double>("inp", 1, 1, "A");

    Fed1->enterExecutingMode();

    auto& ip2 = Fed1->getInput("inp", 0);
    EXPECT_TRUE(ip2.isValid());
    EXPECT_EQ(ip2.getName(), id0.getName());
    EXPECT_EQ(ip2.getType(), "json");

    auto& ip3 = Fed1->getInput("inp", 1);
    EXPECT_TRUE(ip3.isValid());
    EXPECT_EQ(ip3.getType(), "json");

    auto& ip4 = Fed1->getInput("inp", 1, 1);
    EXPECT_TRUE(ip4.isValid());
    EXPECT_EQ(ip4.getType(), "json");
    EXPECT_EQ(pub1.getType(), "json");
    Fed1->finalize();
}

TEST(valuefederate, indexed_pubs)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_indexpub";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);

    auto& pubz = Fed1->registerGlobalPublication<double>("pubg", "volt*meters");

    auto& plocal = Fed1->registerPublication<double>("publocal", "W");

    auto& pub0 = Fed1->registerIndexedPublication<double>("pub", 0, "V");
    auto& pub1 = Fed1->registerIndexedPublication<double>("pub", 1, "V");

    auto& pub2 = Fed1->registerIndexedPublication<double>("pub", 1, 1, "A");

    Fed1->registerSubscription("pubg");

    Fed1->enterExecutingMode();

    auto& ip2 = Fed1->getPublication("pub", 0);
    EXPECT_TRUE(ip2.isValid());
    EXPECT_EQ(ip2.getName(), pub0.getName());

    auto& ip3 = Fed1->getPublication("pub", 1);
    EXPECT_TRUE(ip3.isValid());
    EXPECT_EQ(ip3.getName(), pub1.getName());

    auto& ip4 = Fed1->getPublication("pub", 1, 1);
    EXPECT_TRUE(ip4.isValid());
    EXPECT_EQ(ip4.getName(), pub2.getName());

    const auto& cFed = *Fed1;

    auto& pg3 = cFed.getPublication(0);
    EXPECT_TRUE(pg3.isValid());
    EXPECT_EQ(pg3.getName(), pubz.getName());

    auto& pg4 = cFed.getPublication("pubg");
    EXPECT_TRUE(pg4.isValid());
    EXPECT_EQ(pg4.getName(), pubz.getName());

    auto& inp1 = cFed.getInputByTarget("pubg");
    EXPECT_TRUE(inp1.isValid());
    EXPECT_EQ(inp1.getTarget(), "pubg");

    auto& pg5 = cFed.getPublication("publocal");
    EXPECT_TRUE(pg5.isValid());
    EXPECT_EQ(pg5.getName(), plocal.getName());

    Fed1->finalize();
}

TEST(valuefederate, update_query)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_upd_query";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);
    auto& pub1 = Fed1->registerIndexedPublication<int64_t>("pub", 1);
    auto& pub2 = Fed1->registerIndexedPublication<int64_t>("pub", 2);
    auto& pub3 = Fed1->registerIndexedPublication<int64_t>("pubb", 1, 1);

    auto& sub1 = Fed1->registerIndexedSubscription("pub", 1);
    auto& sub2 = Fed1->registerIndexedSubscription("pub", 2);
    auto& sub3 = Fed1->registerIndexedSubscription("pubb", 1, 1);

    Fed1->enterExecutingMode();
    pub1.publish(5);
    Fed1->requestNextStep();
    auto upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 1U);
    EXPECT_EQ(upd[0], 0);

    pub2.publish(3);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 1U);
    EXPECT_EQ(upd[0], 1);

    pub1.publish(6);
    pub2.publish(7);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 2U);
    EXPECT_EQ(upd[0], 0);

    pub1.publish(8);
    pub2.publish(9);
    pub3.publish(10);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    EXPECT_TRUE(sub1.isUpdated());
    EXPECT_TRUE(sub2.isUpdated());
    EXPECT_TRUE(sub3.isUpdated());

    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 3U);
    EXPECT_EQ(upd[0], 0);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    EXPECT_EQ(upd.size(), 0U);

    Fed1->finalize();
}

TEST(valuefederate, indexed_targets)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_ind_target";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);
    auto& pub1 = Fed1->registerIndexedPublication<int64_t>("pub", 1);
    auto& pub2 = Fed1->registerIndexedPublication<int64_t>("pub", 2);
    auto& pub3 = Fed1->registerIndexedPublication<int64_t>("pubb", 1, 1);

    auto& inp1 = Fed1->registerInput<int64_t>("");
    auto& inp2 = Fed1->registerInput<int64_t>("");
    auto& inp3 = Fed1->registerInput<int64_t>("");

    Fed1->addIndexedTarget(inp1, "pub", 1);
    Fed1->addIndexedTarget(inp2, "pub", 2);
    Fed1->addIndexedTarget(inp3, "pubb", 1, 1);
    Fed1->enterExecutingMode();

    pub1.publish(8);
    pub2.publish(9);
    pub3.publish(10);
    Fed1->requestNextStep();

    EXPECT_EQ(inp1.getValue<int64_t>(), 8);
    EXPECT_EQ(inp2.getValue<int64_t>(), 9);
    EXPECT_EQ(inp3.getValue<int64_t>(), 10);

    Fed1->finalize();
}

#ifdef HELICS_ENABLE_ZMQ_CORE
/** test out register interfaces after configuration make sure that doesn't cause issues*/
TEST(valuefederate, file_and_config)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, "-f 2");

    auto file1 = std::string(TEST_DIR) + "fed1_config.json";
    auto file2 = std::string(TEST_DIR) + "fed2_config.json";
    auto Fed1 = std::make_shared<helics::ValueFederate>(file1);
    auto Fed2 = std::make_shared<helics::ValueFederate>(file2);

    Fed1->registerInterfaces(file1);
    Fed2->registerInterfaces(file2);

    EXPECT_EQ(Fed1->getInputCount(), 1);
    EXPECT_EQ(Fed2->getInputCount(), 1);
    EXPECT_EQ(Fed1->getPublicationCount(), 1);
    EXPECT_EQ(Fed2->getPublicationCount(), 1);

    auto& pub1 = Fed1->getPublication(0);
    auto& inp1 = Fed2->getInput(0);
    EXPECT_TRUE(inp1.isValid());
    EXPECT_TRUE(pub1.isValid());

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingModeAsync();
    Fed1->enterExecutingModeComplete();

    // check some tags
    EXPECT_EQ(Fed1->getTag("tag1"), "1");
    EXPECT_EQ(Fed1->getTag("tag2"), "1");
    EXPECT_EQ(Fed1->getTag("tag3"), "1");

    EXPECT_TRUE(Fed2->getFlagOption(HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE));
    pub1.publish(std::complex<double>(1, 2));

    Fed1->requestTimeAsync(1.0);
    Fed2->enterExecutingModeComplete();
    auto val = inp1.getValue<std::complex<double>>();

    EXPECT_EQ(val, std::complex<double>(1, 2));

    Fed2->requestTimeAsync(1.0);
    Fed1->requestTimeComplete();

    Fed1->finalize();
    auto time2 = Fed2->requestTimeComplete();
    EXPECT_EQ(time2, 1.0);
    Fed2->finalize();
    broker->disconnect();
}

// test out some potential bugs in the name handling for cores
TEST(valuefederate, file_name_config)
{
    auto broker = helics::BrokerFactory::create(helics::CoreType::ZMQ, "-f 2");

    auto file1 = std::string(TEST_DIR) + "name_config1.json";
    auto file2 = std::string(TEST_DIR) + "name_config2.json";
    auto Fed1 = std::make_shared<helics::ValueFederate>(file1);
    auto Fed2 = std::make_shared<helics::ValueFederate>(file2);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    Fed1->finalize();
    Fed2->finalize();
    broker->disconnect();
}
#endif

TEST(valuefederate, duplicate_targets)
{
    helics::FederateInfo fedInfo(helics::CoreType::TEST);
    fedInfo.coreName = "core_dup";
    fedInfo.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fedInfo);
    auto& pub1 = Fed1->registerGlobalPublication<int64_t>("pub");

    auto& inp1 = Fed1->registerGlobalInput<int64_t>("inp");

    inp1.addTarget("pub");
    inp1.addTarget("pub");
    Fed1->enterExecutingMode();

    pub1.publish(8);
    Fed1->requestNextStep();

    EXPECT_EQ(inp1.getValue<int64_t>(), 8);

    Fed1->finalize();
}

class vfedPermutation: public ::testing::TestWithParam<int>, public FederateTestFixture {};

TEST_P(vfedPermutation, value_linking_order_permutations_nosan)
{
    SetupTest<helics::ValueFederate>("test_2", 2, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);
    helics::CoreApp core(vFed1->getCorePointer());

    std::vector<std::function<void()>> exList(5);
    std::vector<int> exOrder(5);
    std::iota(exOrder.begin(), exOrder.end(), 0);

    int permutations = GetParam();
    for (int kk = 0; kk < permutations; ++kk) {
        std::next_permutation(exOrder.begin(), exOrder.end());
    }
    exList[0] = [&vFed1]() { vFed1->registerGlobalInput("dest_input", "double"); };
    exList[1] = [&vFed2]() { vFed2->registerGlobalPublication("source_pub", "double"); };
    exList[2] = [&core]() { core->addAlias("dest_input", "dest"); };
    exList[3] = [&core]() { core->addAlias("source_pub", "source"); };
    exList[4] = [&core]() { core->dataLink("source", "dest"); };

    for (int ii = 0; ii < 5; ++ii) {
        exList[exOrder[ii]]();
    }
    auto& inp1 = vFed1->getInput("dest_input");
    auto& pub1 = vFed2->getPublication("source_pub");
    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingMode();
    vFed2->enterExecutingModeComplete();
    pub1.publish(37.6);
    vFed2->requestTimeAsync(0);
    vFed1->requestNextStep();
    vFed2->requestTimeComplete();
    EXPECT_TRUE(inp1.isUpdated());
    auto val = inp1.getDouble();
    EXPECT_EQ(val, 37.6);
    vFed1->finalize();
    vFed2->finalize();
}

INSTANTIATE_TEST_SUITE_P(OrderPermutations, vfedPermutation, testing::Range(0, 5 * 4 * 3 * 2 * 1));
