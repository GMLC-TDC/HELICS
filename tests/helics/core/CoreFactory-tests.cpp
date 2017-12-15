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

#if HELICS_HAVE_ZEROMQ
BOOST_AUTO_TEST_CASE (ZmqCore_test)
{
    BOOST_CHECK_EQUAL (helics::isAvailable (helics::core_type::ZMQ), true);

    auto core = helics::CoreFactory::create (helics::core_type::ZMQ, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;

}
#else // HELICS_HAVE_ZEROMQ
BOOST_AUTO_TEST_CASE(ZmqCore_test)
{
    BOOST_CHECK_EQUAL(helics::isAvailable(helics::core_type::ZMQ), false);
}
#endif  // HELICS_HAVE_ZEROMQ

#if HELICS_HAVE_MPI

BOOST_AUTO_TEST_CASE (MpiCore_test)
{
    BOOST_CHECK_EQUAL(helics::isAvailable(helics::core_type::MPI), true);
    auto core = helics::CoreFactory::create (helics::core_type::MPI, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
}
#else
BOOST_AUTO_TEST_CASE(MpiCore_test)
{
    BOOST_CHECK_EQUAL(helics::isAvailable(helics::core_type::MPI), false);
}
#endif  // HELICS_HAVE_MPI

BOOST_AUTO_TEST_CASE (TestCore_test)
{
    BOOST_CHECK_EQUAL (helics::isAvailable (helics::core_type::TEST), true);

    auto core = helics::CoreFactory::create (helics::core_type::TEST, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;
}

BOOST_AUTO_TEST_CASE (InterprocessCore_test)
{
    BOOST_CHECK_EQUAL (helics::isAvailable (helics::core_type::INTERPROCESS), true);
    BOOST_CHECK_EQUAL (helics::isAvailable (helics::core_type::IPC), true);

    auto core = helics::CoreFactory::create (helics::core_type::INTERPROCESS, "");
    BOOST_REQUIRE (core != nullptr);
    helics::CoreFactory::unregisterCore (core->getIdentifier ());
    core = nullptr;

    auto core2 = helics::CoreFactory::create (helics::core_type::IPC, "");
    BOOST_REQUIRE (core2 != nullptr);
    helics::CoreFactory::unregisterCore (core2->getIdentifier ());
    core2 = nullptr;
}

#ifndef DISABLE_TCP_CORE
BOOST_AUTO_TEST_CASE (tcpCore_test)
{
    BOOST_CHECK_EQUAL(helics::isAvailable(helics::core_type::TCP), true);

    auto core = helics::CoreFactory::create(helics::core_type::TCP, "");
    BOOST_REQUIRE(core != nullptr);
    helics::CoreFactory::unregisterCore(core->getIdentifier());
    core = nullptr;

    auto core2 = helics::CoreFactory::create(helics::core_type::TCP, "");
    BOOST_REQUIRE(core2 != nullptr);
    helics::CoreFactory::unregisterCore(core2->getIdentifier());
    core2 = nullptr;
}
#else
BOOST_AUTO_TEST_CASE(tcpCore_test)
{
    BOOST_CHECK_EQUAL(helics::isAvailable(helics::core_type::TCP), false);
}
#endif

BOOST_AUTO_TEST_CASE (udpCore_test)
{
    BOOST_CHECK_EQUAL (helics::isAvailable (helics::core_type::UDP), true);

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
