/*
Copyright (c) 2017-2021,
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

#include <future>
#include <gtest/gtest.h>

/** these test cases test out the value federates with some additional tests
 */

class valuefed_add_single_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class valuefed_add_all_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class valuefed_add_type_tests_ci_skip:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

class valuefed_add_tests_ci_skip: public ::testing::Test, public FederateTestFixture {
};

/** test simple creation and destruction*/
TEST_P(valuefed_add_single_type_tests_ci_skip, initialize)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

#ifdef ENABLE_ZMQ_CORE

class valuefed_add_ztype_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(valuefed_add_ztype_tests, publication_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    auto pubid = vFed1->registerPublication<std::string>("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto pubid3 = vFed1->registerPublication("pub3", "double", "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    auto sv = vFed1->getInterfaceName(pubid);
    auto sv2 = vFed1->getInterfaceName(pubid2);
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    auto pub3name = vFed1->getInterfaceName(pubid3);
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(vFed1->getExtractionType(pubid3), "double");
    EXPECT_EQ(vFed1->getInterfaceUnits(pubid3), "V");

    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pubid.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pubid2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pubid.getHandle());
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

INSTANTIATE_TEST_SUITE_P(vfed_add_tests, valuefed_add_ztype_tests, ::testing::ValuesIn(ztypes));

#endif

TEST_P(valuefed_add_single_type_tests_ci_skip, publisher_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    helics::Publication pubid(vFed1.get(), "pub1", helics::helicsType<std::string>());
    helics::PublicationT<int> pubid2(helics::GLOBAL, vFed1.get(), "pub2");

    vFed1->setSeparator('-');
    helics::Publication pubid3(vFed1.get(), "pub3", helics::helicsType<double>(), "V");
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    const auto& sv = pubid.getKey();
    const auto& sv2 = pubid2.getKey();
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    const auto& pub3name = pubid3.getKey();
    EXPECT_EQ(pub3name, "fed0-pub3");

    EXPECT_EQ(pubid3.getType(), "double");
    EXPECT_EQ(pubid3.getUnits(), "V");
    vFed1->setSeparator('/');
    EXPECT_TRUE(vFed1->getPublication("pub1").getHandle() == pubid.getHandle());
    EXPECT_TRUE(vFed1->getPublication("pub2").getHandle() == pubid2.getHandle());
    EXPECT_TRUE(vFed1->getPublication("fed0/pub1").getHandle() == pubid.getHandle());
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
}

TEST_P(valuefed_add_single_type_tests_ci_skip, subscription_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    vFed1->setFlagOption(helics_handle_option_connection_optional);
    auto& subid = vFed1->registerSubscription("sub1", "V");
    auto& subid2 = vFed1->registerSubscription("sub2");

    auto& subid3 = vFed1->registerSubscription("sub3", "V");
    vFed1->enterExecutingMode();

    // EXPECT_TRUE (vFed->getCurrentMode () == helics::Federate::modes::executing);

    auto& sv = vFed1->getTarget(subid);
    auto& sv2 = vFed1->getTarget(subid2);
    EXPECT_EQ(sv, "sub1");
    EXPECT_EQ(sv2, "sub2");
    auto& sub3name = vFed1->getTarget(subid3);

    vFed1->addAlias(subid, "Shortcut");
    EXPECT_EQ(sub3name, "sub3");

    EXPECT_EQ(vFed1->getInterfaceUnits(subid3), "V");

    EXPECT_TRUE(vFed1->getSubscription("sub1").getHandle() == subid.getHandle());
    EXPECT_TRUE(vFed1->getSubscription("sub2").getHandle() == subid2.getHandle());

    EXPECT_TRUE(vFed1->getSubscription("Shortcut").getHandle() == subid.getHandle());

    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, subscription_and_publication_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(helics_handle_option_connection_optional);
    // register the publications
    auto pubid = vFed1->registerPublication<std::string>("pub1");
    auto pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto pubid3 = vFed1->registerPublication("pub3", "double", "V");

    // optional
    auto subid = vFed1->registerSubscription("sub1", "V");
    auto subid2 = vFed1->registerSubscription("sub2");

    auto subid3 = vFed1->registerSubscription("sub3", "V");
    // enter execution
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);
    // check subscriptions
    auto sv = vFed1->getTarget(subid);
    auto sv2 = vFed1->getTarget(subid2);
    EXPECT_EQ(sv, "sub1");
    EXPECT_EQ(sv2, "sub2");
    auto sub3name = vFed1->getTarget(subid3);
    EXPECT_EQ(sub3name, "sub3");

    EXPECT_EQ(vFed1->getInterfaceUnits(subid3), "V");

    // check publications

    sv = vFed1->getInterfaceName(pubid);
    sv2 = vFed1->getInterfaceName(pubid2);
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    auto pub3name = vFed1->getInterfaceName(pubid3);
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(vFed1->getExtractionType(pubid3), "double");
    EXPECT_EQ(vFed1->getInterfaceUnits(pubid3), "V");
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, input_and_publication_registration)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(helics_handle_option_connection_optional);
    // register the publications
    auto& pubid = vFed1->registerPublication<std::string>("pub1");
    auto& pubid2 = vFed1->registerGlobalPublication<int>("pub2");

    auto& pubid3 = vFed1->registerPublication("pub3", "double", "V");

    // optional
    auto& subid = vFed1->registerInput("sub1", "vector", "V");
    subid.addTarget("pub2");
    auto& subid2 = vFed1->registerGlobalInput("sub2", "double", "volts");

    // enter execution
    vFed1->enterExecutingMode();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);
    // check subscriptions
    EXPECT_EQ(subid.getTarget(), "pub2");
    EXPECT_EQ(vFed1->getInterfaceName(subid2), "sub2");

    EXPECT_EQ(subid.getName(), "fed0/sub1");
    EXPECT_EQ(vFed1->getInterfaceName(subid), "fed0/sub1");
    EXPECT_EQ(vFed1->getTarget(subid), "pub2");

    EXPECT_EQ(vFed1->getExtractionType(subid), "vector");
    EXPECT_EQ(vFed1->getExtractionType(subid2), "double");

    EXPECT_EQ(vFed1->getInjectionType(subid), "int32");

    // check publications

    auto& sv = vFed1->getInterfaceName(pubid);
    auto& sv2 = vFed1->getInterfaceName(pubid2);
    EXPECT_EQ(sv, "fed0/pub1");
    EXPECT_EQ(sv2, "pub2");
    auto& pub3name = vFed1->getInterfaceName(pubid3);
    EXPECT_EQ(pub3name, "fed0/pub3");

    EXPECT_EQ(vFed1->getExtractionType(pubid3), "double");
    EXPECT_EQ(vFed1->getInterfaceUnits(pubid3), "V");
    vFed1->finalize();

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::finalize);
    helics::cleanupHelicsLibrary();
}

TEST_P(valuefed_add_single_type_tests_ci_skip, single_transfer)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string>("pub1");

    auto subid = vFed1->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed1->enterExecutingMode();
    // publish string1 at time=0.0;
    vFed1->publish(pubid, "string1");
    auto gtime = vFed1->requestTime(1.0);

    EXPECT_EQ(gtime, 1.0);
    std::string s = vFed1->getString(subid);
    // get the value
    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    vFed1->publish(pubid, "string2");
    // make sure the value is still what we expect
    s = vFed1->getString(subid);

    EXPECT_EQ(s, "string1");
    // advance time
    gtime = vFed1->requestTimeAdvance(1.0);
    // make sure the value was updated
    EXPECT_EQ(gtime, 2.0);
    s = vFed1->getString(subid);

    EXPECT_EQ(s, "string2");
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
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>>(GetParam(), def, v1, v2);
}

TEST_F(valuefed_add_tests_ci_skip, dual_transfer_complex_long)
{
    std::complex<double> def = {54.23233, 0.7};
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTest<std::complex<double>>("test_7", def, v1, v2);
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
    std::complex<double> v1 = std::polar(10.0, 0.43);
    std::complex<double> v2 = {-3e45, 1e-23};
    runDualFederateTestObj<std::complex<double>>(GetParam(), def, v1, v2);
}

TEST_P(valuefed_add_type_tests_ci_skip, dual_transfer_types_obj10)
{
    helics::NamedPoint def{"trigger", 0.7};
    helics::NamedPoint v1{"response", -1e-12};
    helics::NamedPoint v2{"variance", 45.23};
    runDualFederateTestObjv2<helics::NamedPoint>(GetParam(), def, v1, v2);
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

    helics::data_block db(547, ';');
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
    vFed1->publishRaw(pubid3, db);
    vFed1->requestTime(1.0);
    // callbacks here
    EXPECT_EQ(ccnt, 0);

    vFed1->publish(pubid1, "this is a test");
    vFed1->requestTime(3.0);
    EXPECT_EQ(ccnt, 1);

    ccnt = 0;  // reset the counter
    vFed1->publishRaw(pubid3, db);
    vFed1->publish(pubid2, 4);
    vFed1->publish(pubid1, "test string2");
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

    vFed1->publish(pubid1, 10.0);
    vFed1->publish(pubid2, 20.0);
    vFed1->publish(pubid3, 30.0);
    vFed1->requestTime(2.0);
    auto v1 = vFed1->getDouble(sub1);
    auto v2 = vFed1->getDouble(sub2);
    auto v3 = vFed1->getDouble(sub3);

    EXPECT_NEAR(10.0, v1, 0.00000001);
    EXPECT_NEAR(20.0, v2, 0.00000001);
    EXPECT_NEAR(30.0, v3, 0.00000001);
}

/** test the publish/subscribe to a vectorized array*/

TEST_P(valuefed_add_type_tests_ci_skip, async_calls)
{
    SetupTest<helics::ValueFederate>(GetParam(), 2);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    auto vFed2 = GetFederateAs<helics::ValueFederate>(1);

    // register the publications
    auto pubid = vFed1->registerGlobalPublication<std::string>("pub1");

    auto subid = vFed2->registerSubscription("pub1");
    vFed1->setProperty(helics_property_time_delta, 1.0);
    vFed2->setProperty(helics_property_time_delta, 1.0);

    vFed1->enterExecutingModeAsync();
    EXPECT_TRUE(!vFed1->isAsyncOperationCompleted());
    vFed2->enterExecutingModeAsync();
    vFed1->enterExecutingModeComplete();
    vFed2->enterExecutingModeComplete();
    // publish string1 at time=0.0;
    vFed1->publish(pubid, "string1");
    vFed1->requestTimeAsync(1.0);
    vFed2->requestTimeAsync(1.0);

    auto f1time = vFed1->requestTimeComplete();
    auto gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(gtime, 1.0);
    EXPECT_EQ(f1time, 1.0);
    // get the value
    std::string s = vFed2->getString(subid);

    // make sure the string is what we expect
    EXPECT_EQ(s, "string1");
    // publish a second string
    vFed1->publish(pubid, "string2");
    // make sure the value is still what we expect
    subid.getValue(s);
    EXPECT_EQ(s, "string1");
    // advance time
    vFed1->requestTimeAsync(2.0);
    vFed2->requestTimeAsync(2.0);
    f1time = vFed1->requestTimeComplete();
    gtime = vFed2->requestTimeComplete();

    EXPECT_EQ(gtime, 2.0);
    EXPECT_EQ(f1time, 2.0);

    // make sure the value was updated

    s = subid.getValue<std::string>();
    EXPECT_EQ(s, "string2");
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

    EXPECT_TRUE(vFed1->getCurrentMode() == helics::Federate::modes::executing);

    auto info1 = vFed1->getInfo(pubid1.getHandle());
    auto info2 = vFed1->getInfo(pubid2.getHandle());
    EXPECT_EQ(info1, "test1");
    EXPECT_EQ(info2, "test2");

    vFed1->finalize();
}

/** test the pub/sub info field*/
TEST_P(valuefed_add_single_type_tests_ci_skip, info_pubs_subs)
{
    SetupTest<helics::ValueFederate>(GetParam(), 1);
    auto vFed1 = GetFederateAs<helics::ValueFederate>(0);
    vFed1->setFlagOption(helics_handle_option_connection_optional);
    auto pubid1 = vFed1->registerIndexedPublication<double>("pub1", 0);
    pubid1.setInfo(std::string("pub_test1"));

    auto sub1 = vFed1->registerIndexedSubscription("pub1", 0);
    auto sub2 = vFed1->registerIndexedSubscription("pub1", 1);
    auto sub3 = vFed1->registerIndexedSubscription("pub1", 2);

    sub1.setInfo(std::string("sub_test1"));
    sub2.setInfo(std::string("sub_test2"));
    sub3.setInfo(std::string("sub_test3"));

    vFed1->enterExecutingMode();

    // Check all values can be accessed and returned through the federate.
    auto info1 = vFed1->getInfo(pubid1.getHandle());
    auto info2 = vFed1->getInfo(sub1.getHandle());
    auto info3 = vFed1->getInfo(sub2.getHandle());
    auto info4 = vFed1->getInfo(sub3.getHandle());

    EXPECT_EQ(info1, "pub_test1");
    EXPECT_EQ(info2, "sub_test1");
    EXPECT_EQ(info3, "sub_test2");
    EXPECT_EQ(info4, "sub_test3");

    // Check all values can be accessed and returned directly from their subscriptions.
    auto sub_info2 = sub1.getInfo();
    auto sub_info3 = sub2.getInfo();
    auto sub_info4 = sub3.getInfo();

    EXPECT_EQ(sub_info2, "sub_test1");
    EXPECT_EQ(sub_info3, "sub_test2");
    EXPECT_EQ(sub_info4, "sub_test3");

    vFed1->finalize();
}

/** test the default constructor and move constructor and move assignment*/
TEST_F(valuefed_add_tests_ci_skip, test_move_calls)
{
    helics::ValueFederate vFed;

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreInitString = "-f 3 --autobroker";
    vFed = helics::ValueFederate("test1", fi);
    EXPECT_EQ(vFed.getName(), "test1");

    helics::ValueFederate vFedMoved(std::move(vFed));
    EXPECT_EQ(vFedMoved.getName(), "test1");
    // verify that this was moved so this does produce a warning on some systems about use after
    // move
    EXPECT_NE(vFed.getName(), "test1");  // NOLINT
}

static constexpr const char* config_files[] = {"example_value_fed.json", "example_value_fed.toml"};

class valuefed_add_configfile_tests:
    public ::testing::TestWithParam<const char*>,
    public FederateTestFixture {
};

TEST_P(valuefed_add_configfile_tests, file_load)
{
    helics::ValueFederate vFed(std::string(TEST_DIR) + GetParam());

    EXPECT_EQ(vFed.getName(), "valueFed");

    EXPECT_EQ(vFed.getInputCount(), 3);
    EXPECT_EQ(vFed.getPublicationCount(), 2);
    auto& id = vFed.getInput("pubshortcut");

    auto key = vFed.getTarget(id);
    EXPECT_EQ(key, "fedName/pub2");

    EXPECT_EQ(id.getInfo(), "this is an information string for use by the application");
    auto pub2name = vFed.getInterfaceName(vFed.getPublication(1));
    EXPECT_EQ(pub2name, "valueFed/pub2");
    // test the info from a file
    EXPECT_EQ(vFed.getPublication(0).getInfo(),
              "this is an information string for use by the application");

    EXPECT_EQ(vFed.getInput(2).getName(), "valueFed/ipt2");

    EXPECT_EQ(vFed.query("global", "global1"), "this is a global1 value");
    EXPECT_EQ(vFed.query("global", "global2"), "this is another global value");

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
    auto& id = vFed.getPublication("primary");

    EXPECT_EQ(id.getName(), "valueFed2/pub2");

    auto& id2 = vFed.getPublication("pub1");
    EXPECT_EQ(id2.getUnits(), "m");

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
    auto& id = vFed.getPublication("primary");

    EXPECT_EQ(id.getName(), "valueFed_toml/pub2");

    auto& id2 = vFed.getPublication("pub1");
    EXPECT_EQ(id2.getUnits(), "m");
    vFed.enterExecutingMode();
    vFed.disconnect();
}

TEST(valuefederate, toml_file_bad)
{
    EXPECT_THROW(helics::ValueFederate vFed(std::string(TEST_DIR) + "example_value_fed_bad.toml"),
                 helics::InvalidParameter);
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
}

INSTANTIATE_TEST_SUITE_P(valuefed_tests,
                         valuefed_add_configfile_tests,
                         ::testing::ValuesIn(config_files));

TEST(valuefed_json_tests, json_publish)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.separator = '/';
    fi.coreName = "json_test2";
    fi.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fi);
    vFed.registerGlobalPublication<double>("pub1");
    vFed.registerPublication<std::string>("pub2");
    vFed.registerPublication<double>("group1/pubA");
    vFed.registerPublication<std::string>("group1/pubB");

    auto& s1 = vFed.registerSubscription("pub1");
    auto& s2 = vFed.registerSubscription("test2/pub2");
    auto& s3 = vFed.registerSubscription("test2/group1/pubA");
    auto& s4 = vFed.registerSubscription("test2/group1/pubB");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed.requestTime(1.0);
    EXPECT_EQ(s1.getValue<double>(), 99.9);
    EXPECT_EQ(s2.getValue<std::string>(), "things");
    EXPECT_EQ(s3.getValue<double>(), 45.7);
    EXPECT_EQ(s4.getValue<std::string>(), "count");

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed.requestTime(2.0);
    EXPECT_EQ(s1.getValue<double>(), 88.2);
    EXPECT_EQ(s2.getValue<std::string>(), "items");
    EXPECT_EQ(s3.getValue<double>(), 15.0);
    EXPECT_EQ(s4.getValue<std::string>(), "count2");

    vFed.publishJSON("{\"pub1\": 77.2}");

    vFed.requestTime(3.0);
    EXPECT_EQ(s1.getValue<double>(), 77.2);

    vFed.disconnect();
}

TEST(valuefed_json_tests, test_json_register_publish)
{
    helics::BrokerFactory::terminateAllBrokers();
    helics::CoreFactory::terminateAllCores();
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.separator = '/';
    fi.coreName = "core_pub_json";
    fi.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fi);

    vFed.registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto& s1 = vFed.registerSubscription("test2/pub1");
    auto& s2 = vFed.registerSubscription("test2/pub2");
    auto& s3 = vFed.registerSubscription("test2/group1/pubA");
    auto& s4 = vFed.registerSubscription("test2/group1/pubB");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    vFed.requestTime(1.0);
    EXPECT_EQ(s1.getValue<double>(), 99.9);
    EXPECT_EQ(s2.getValue<std::string>(), "things");
    EXPECT_EQ(s3.getValue<double>(), 45.7);
    EXPECT_EQ(s4.getValue<std::string>(), "count");

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input2.json");
    vFed.requestTime(2.0);
    EXPECT_EQ(s1.getValue<double>(), 88.2);
    EXPECT_EQ(s2.getValue<std::string>(), "items");
    EXPECT_EQ(s3.getValue<double>(), 15.0);
    EXPECT_EQ(s4.getValue<std::string>(), "count2");

    vFed.disconnect();
}

TEST(valuefed_json_tests, test_json_register_publish_error)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.separator = '/';
    fi.coreInitString = "--autobroker";
    helics::ValueFederate vFed("test2", fi);

    vFed.registerPublication<double>("pub1");
    // this tests an already registered publication
    vFed.registerFromPublicationJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    auto& s1 = vFed.registerSubscription("test2/pub1");
    auto& s2 = vFed.registerSubscription("test2/pub2");
    auto& s3 = vFed.registerSubscription("test2/group1/pubA");
    auto& s4 = vFed.registerSubscription("test2/group1/pubB");

    EXPECT_NO_THROW(vFed.registerFromPublicationJSON("{\"pub3\":45}"));
    auto& s5 = vFed.registerSubscription("test2/pub3");
    vFed.enterExecutingMode();

    vFed.publishJSON(std::string(TEST_DIR) + "example_pub_input1.json");
    EXPECT_NO_THROW(vFed.publishJSON("{\"pub3\":45}"));
    vFed.requestTime(1.0);
    EXPECT_EQ(s1.getValue<double>(), 99.9);
    EXPECT_EQ(s2.getValue<std::string>(), "things");
    EXPECT_EQ(s3.getValue<double>(), 45.7);
    EXPECT_EQ(s4.getValue<std::string>(), "count");
    EXPECT_EQ(s5.getValue<double>(), 45.0);
    vFed.disconnect();
}

INSTANTIATE_TEST_SUITE_P(valuefed_tests,
                         valuefed_add_single_type_tests_ci_skip,
                         ::testing::ValuesIn(core_types_single));
INSTANTIATE_TEST_SUITE_P(valuefed_tests,
                         valuefed_add_type_tests_ci_skip,
                         ::testing::ValuesIn(core_types));
INSTANTIATE_TEST_SUITE_P(valuefed_tests,
                         valuefed_add_all_type_tests_ci_skip,
                         ::testing::ValuesIn(core_types_all));

TEST(valuefederate, coreApp)
{
    helics::CoreApp capp(helics::core_type::TEST, "corename", "-f 1 --autobroker");
    helics::FederateInfo fi(helics::core_type::TEST);
    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", capp, fi);
    EXPECT_NO_THROW(Fed1->enterExecutingMode());

    Fed1->finalize();
}

TEST(valuefederate, core_ptr)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_ptr";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", nullptr, fi);
    Fed1->enterExecutingMode();

    EXPECT_THROW(auto fed2 = std::make_shared<helics::ValueFederate>("vfed2", nullptr, fi),
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

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_bad_toml";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfedb", fi);

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

    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_alias";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);
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
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_pjson";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);

    EXPECT_THROW(Fed1->registerFromPublicationJSON("invalid.json"), helics::InvalidParameter);

    Fed1->enterExecutingMode();
    EXPECT_THROW(Fed1->publishJSON("invalid.json"), helics::InvalidParameter);
    Fed1->finalize();
}

TEST(valuefederate, getInputs)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_ipt";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);

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
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_indexipt";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);

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

TEST(valuefederate, indexed_pubs)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_indexpub";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);

    auto& pz = Fed1->registerGlobalPublication<double>("pubg", "volt*meters");

    auto& plocal = Fed1->registerPublication<double>("publocal", "W");

    auto& p0 = Fed1->registerIndexedPublication<double>("pub", 0, "V");
    auto& p1 = Fed1->registerIndexedPublication<double>("pub", 1, "V");

    auto& p2 = Fed1->registerIndexedPublication<double>("pub", 1, 1, "A");

    Fed1->registerSubscription("pubg");

    Fed1->enterExecutingMode();

    auto& ip2 = Fed1->getPublication("pub", 0);
    EXPECT_TRUE(ip2.isValid());
    EXPECT_EQ(ip2.getName(), p0.getName());

    auto& ip3 = Fed1->getPublication("pub", 1);
    EXPECT_TRUE(ip3.isValid());
    EXPECT_EQ(ip3.getName(), p1.getName());

    auto& ip4 = Fed1->getPublication("pub", 1, 1);
    EXPECT_TRUE(ip4.isValid());
    EXPECT_EQ(ip4.getName(), p2.getName());

    const auto& cFed = *Fed1;

    auto& pg3 = cFed.getPublication(0);
    EXPECT_TRUE(pg3.isValid());
    EXPECT_EQ(pg3.getName(), pz.getName());

    auto& pg4 = cFed.getPublication("pubg");
    EXPECT_TRUE(pg4.isValid());
    EXPECT_EQ(pg4.getName(), pz.getName());

    auto& gs = cFed.getSubscription("pubg");
    EXPECT_TRUE(gs.isValid());
    EXPECT_EQ(gs.getTarget(), "pubg");

    auto& pg5 = cFed.getPublication("publocal");
    EXPECT_TRUE(pg5.isValid());
    EXPECT_EQ(pg5.getName(), plocal.getName());

    Fed1->finalize();
}

TEST(valuefederate, update_query)
{
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_upd_query";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);
    auto& p1 = Fed1->registerIndexedPublication<int64_t>("pub", 1);
    auto& p2 = Fed1->registerIndexedPublication<int64_t>("pub", 2);
    auto& p3 = Fed1->registerIndexedPublication<int64_t>("pubb", 1, 1);

    auto& s1 = Fed1->registerIndexedSubscription("pub", 1);
    auto& s2 = Fed1->registerIndexedSubscription("pub", 2);
    auto& s3 = Fed1->registerIndexedSubscription("pubb", 1, 1);

    Fed1->enterExecutingMode();
    p1.publish(5);
    Fed1->requestNextStep();
    auto upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 1U);
    EXPECT_EQ(upd[0], 0);

    p2.publish(3);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 1U);
    EXPECT_EQ(upd[0], 1);

    p1.publish(6);
    p2.publish(7);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    Fed1->clearUpdates();
    ASSERT_EQ(upd.size(), 2U);
    EXPECT_EQ(upd[0], 0);

    p1.publish(8);
    p2.publish(9);
    p3.publish(10);
    Fed1->requestNextStep();

    upd = Fed1->queryUpdates();
    EXPECT_TRUE(s1.isUpdated());
    EXPECT_TRUE(s2.isUpdated());
    EXPECT_TRUE(s3.isUpdated());

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
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_ind_target";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);
    auto& p1 = Fed1->registerIndexedPublication<int64_t>("pub", 1);
    auto& p2 = Fed1->registerIndexedPublication<int64_t>("pub", 2);
    auto& p3 = Fed1->registerIndexedPublication<int64_t>("pubb", 1, 1);

    auto& s1 = Fed1->registerInput<int64_t>("");
    auto& s2 = Fed1->registerInput<int64_t>("");
    auto& s3 = Fed1->registerInput<int64_t>("");

    Fed1->addIndexedTarget(s1, "pub", 1);
    Fed1->addIndexedTarget(s2, "pub", 2);
    Fed1->addIndexedTarget(s3, "pubb", 1, 1);
    Fed1->enterExecutingMode();

    p1.publish(8);
    p2.publish(9);
    p3.publish(10);
    Fed1->requestNextStep();

    EXPECT_EQ(s1.getValue<int64_t>(), 8);
    EXPECT_EQ(s2.getValue<int64_t>(), 9);
    EXPECT_EQ(s3.getValue<int64_t>(), 10);

    Fed1->finalize();
}

#ifdef ENABLE_ZMQ_CORE
/** test out register interfaces after configuration make sure that doesn't cause issues*/
TEST(valuefederate, file_and_config)
{
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, "-f 2");

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

    auto& p1 = Fed1->getPublication(0);
    auto& i1 = Fed2->getInput(0);

    Fed1->enterExecutingModeAsync();
    Fed2->enterExecutingMode();
    Fed1->enterExecutingModeComplete();

    p1.publish(std::complex<double>(1, 2));

    Fed1->requestTimeAsync(1.0);
    auto t2 = Fed2->requestTime(1.0);
    EXPECT_EQ(t2, 0.5);  // fed2 has wait_for_current_time flag active
    Fed2->requestTimeAsync(1.0);
    Fed1->requestTimeComplete();
    auto val = i1.getValue<std::complex<double>>();

    EXPECT_EQ(val, std::complex<double>(1, 2));

    Fed1->finalize();
    t2 = Fed2->requestTimeComplete();
    EXPECT_EQ(t2, 1.0);
    Fed2->finalize();
    broker->disconnect();
}

// test out some potential bugs in the name handling for cores
TEST(valuefederate, file_name_config)
{
    auto broker = helics::BrokerFactory::create(helics::core_type::ZMQ, "-f 2");

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
    helics::FederateInfo fi(helics::core_type::TEST);
    fi.coreName = "core_dup";
    fi.coreInitString = "-f 1 --autobroker";

    auto Fed1 = std::make_shared<helics::ValueFederate>("vfed1", fi);
    auto& p1 = Fed1->registerGlobalPublication<int64_t>("pub");

    auto& s1 = Fed1->registerGlobalInput<int64_t>("inp");

    s1.addTarget("pub");
    s1.addTarget("pub");
    Fed1->enterExecutingMode();

    p1.publish(8);
    Fed1->requestNextStep();

    EXPECT_EQ(s1.getValue<int64_t>(), 8);

    Fed1->finalize();
}
