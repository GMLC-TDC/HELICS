/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/core/CommonCore.h"
#include "helics/core/CoreFactory.h"
#include "helics/helics-config.h"

BOOST_AUTO_TEST_SUITE (CoreFactory_tests)

BOOST_AUTO_TEST_CASE (ZmqCore_test)
{
#if HELICS_HAVE_ZEROMQ
    const bool haveZmq = true;
#else
    const bool haveZmq = false;
#endif  // HELICS_HAVE_ZEROMQ

    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::ZMQ), haveZmq);

#if HELICS_HAVE_ZEROMQ
    auto core = helics::CoreFactory::create (helics::core_type::ZMQ, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
#endif  // HELICS_HAVE_ZEROMQ
}

BOOST_AUTO_TEST_CASE (MpiCore_test)
{
#if HELICS_HAVE_MPI
    const bool haveMpi = true;
#else
    const bool haveMpi = false;
#endif  // HELICS_HAVE_MPI

    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::MPI), haveMpi);

#if HELICS_HAVE_MPI
    auto core = helics::CoreFactory::create (HELICS_MPI, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
#endif  // HELICS_HAVE_MPI
}

BOOST_AUTO_TEST_CASE (TestCore_test)
{
    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::TEST), true);

    auto core = helics::CoreFactory::create (helics::core_type::TEST, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
}

BOOST_AUTO_TEST_CASE (InterprocessCore_test)
{
    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::INTERPROCESS), true);
    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::IPC), true);

    auto core = helics::CoreFactory::create (helics::core_type::INTERPROCESS, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;

    auto core2 = helics::CoreFactory::create (helics::core_type::IPC, "");
    BOOST_REQUIRE (core2 != nullptr);
    helics::CoreFactory::unregisterCore (core2->getIdentifier ());
    core2 = nullptr;
}

BOOST_AUTO_TEST_CASE (tcpCore_test)
{
    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::TCP), false);
}

BOOST_AUTO_TEST_CASE (udpCore_test)
{
    BOOST_CHECK_EQUAL (helics::CoreFactory::isAvailable (helics::core_type::UDP), true);

    auto core = helics::CoreFactory::create (helics::core_type::UDP, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;

    auto core2 = helics::CoreFactory::create (helics::core_type::UDP, "");
    BOOST_REQUIRE (core2 != nullptr);
    helics::CoreFactory::unregisterCore (core2->getIdentifier ());
    core2 = nullptr;
}

BOOST_AUTO_TEST_SUITE_END ()
