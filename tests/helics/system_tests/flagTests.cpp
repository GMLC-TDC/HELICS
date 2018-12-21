/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../application_api/testFixtures.hpp"
#include "helics/ValueFederates.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace bdata = boost::unit_test::data;
namespace utf = boost::unit_test;

/** tests for the different flag options and considerations*/

BOOST_FIXTURE_TEST_SUITE (flag_tests, FederateTestFixture)

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (test_optional_pub, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerInput<double> ("ipt1");

    auto &ipt2 = vFed1->registerInput<double> ("ipt2");
    auto &ipt3 = vFed1->registerInput<double> ("ipt3");
    ipt1.setOption (helics::defs::options::connection_optional);
    ipt1.addTarget ("bob");
    ipt2.addTarget ("tom");
    ipt3.addTarget ("harry");
    std::atomic<int> warnings{0};
    vFed1->setLoggingCallback ([&warnings](int level, const std::string &, const std::string &) {
        if (level == 1)
        {
            ++warnings;
        }
    });

    vFed1->enterExecutingMode ();
    BOOST_CHECK_EQUAL (warnings.load (), 2);
    vFed1->finalize ();
}

BOOST_DATA_TEST_CASE (test_optional_sub, bdata::make (core_types_single), core_type)
{
    SetupTest<helics::ValueFederate> (core_type, 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &pub1 = vFed1->registerPublication<double> ("pub1");

    pub1.addTarget ("bob");
    pub1.setOption (helics::defs::options::connection_optional);
    pub1.addTarget ("tom");
    pub1.addTarget ("harry");
    std::atomic<int> warnings{0};

    vFed1->setLoggingCallback ([&warnings](int level, const std::string &, const std::string &) {
        if (level == 1)
        {
            ++warnings;
        }
    });

    vFed1->enterExecutingMode ();
    BOOST_CHECK_EQUAL (warnings.load (), 1);
    BOOST_CHECK (pub1.getOption (helics::defs::options::connection_optional));

    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (single_connection_test)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput<double> ("ipt1");

    auto &pub1 = vFed1->registerPublication<double> ("pub1");
    auto &pub2 = vFed1->registerPublication<double> ("pub2");
    ipt1.setOption (helics::defs::options::single_connection_only);
    pub1.addTarget ("ipt1");
    pub2.addTarget ("ipt1");

    BOOST_CHECK_THROW (vFed1->enterExecutingMode (), helics::ConnectionFailure);

    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (single_connection_test_pub)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput<double> ("ipt1");

    auto &ipt2 = vFed1->registerGlobalInput<double> ("ipt2");
    auto &pub1 = vFed1->registerGlobalPublication<double> ("pub1");
    pub1.setOption (helics::defs::options::single_connection_only);
    ipt1.addTarget ("pub1");
    ipt2.addTarget ("pub1");

    BOOST_CHECK_THROW (vFed1->enterExecutingMode (), helics::ConnectionFailure);

    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (type_match_check)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput<double> ("ipt1");

    vFed1->registerGlobalPublication<double> ("pub1");
    ipt1.setOption (helics::defs::options::strict_type_checking);
    ipt1.addTarget ("pub1");
    vFed1->enterExecutingMode ();
    BOOST_CHECK (ipt1.getOption (helics::defs::options::strict_type_checking));
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (type_mismatch_error)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput<double> ("ipt1");

    vFed1->registerGlobalPublication<std::vector<double>> ("pub1");
    ipt1.setOption (helics::defs::options::strict_type_checking);
    ipt1.addTarget ("pub1");
    BOOST_CHECK_THROW (vFed1->enterExecutingMode (), helics::ConnectionFailure);
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (type_mismatch_error_fed_level)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    vFed1->setFlagOption (helics::defs::flags::strict_input_type_checking);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput<double> ("ipt1");

    vFed1->registerGlobalPublication<std::vector<double>> ("pub1");
    ipt1.addTarget ("pub1");
    BOOST_CHECK_THROW (vFed1->enterExecutingMode (), helics::ConnectionFailure);
    vFed1->finalize ();
}

BOOST_AUTO_TEST_CASE (type_mismatch_error2)
{
    SetupTest<helics::ValueFederate> ("test", 1, 1.0);
    auto vFed1 = GetFederateAs<helics::ValueFederate> (0);
    // register the publications
    auto &ipt1 = vFed1->registerGlobalInput ("ipt1", "custom", "V");

    vFed1->registerGlobalPublication ("pub1", "other");
    ipt1.setOption (helics::defs::options::strict_type_checking);
    ipt1.addTarget ("pub1");
    BOOST_CHECK_THROW (vFed1->enterExecutingMode (), helics::ConnectionFailure);
    vFed1->finalize ();
}

BOOST_AUTO_TEST_SUITE_END ()
