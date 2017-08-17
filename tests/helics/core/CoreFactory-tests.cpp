/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include "helics/config.h"
#include "helics/core/CoreFactory.h"

BOOST_AUTO_TEST_SUITE(CoreFactory_tests)

BOOST_AUTO_TEST_CASE(ZmqCore_test)
{
#if HELICS_HAVE_ZEROMQ
    const bool haveZmq = true;
#else
    const bool haveZmq = false;
#endif // HELICS_HAVE_ZEROMQ

    BOOST_CHECK_EQUAL(helics::CoreFactory::isAvailable(HELICS_ZMQ), haveZmq);

#if HELICS_HAVE_ZEROMQ
    auto core = helics::CoreFactory::create(HELICS_ZMQ, "");
    BOOST_CHECK(core != nullptr);
#endif // HELICS_HAVE_ZEROMQ
}

BOOST_AUTO_TEST_CASE(MpiCore_test)
{
#if HELICS_HAVE_MPI
    const bool haveMpi = true;
#else
    const bool haveMpi = false;
#endif // HELICS_HAVE_MPI

    BOOST_CHECK_EQUAL(helics::CoreFactory::isAvailable(HELICS_MPI), haveMpi);

#if HELICS_HAVE_MPI
    auto core = helics::CoreFactory::create(HELICS_MPI, "");
    BOOST_CHECK(core != nullptr);
#endif // HELICS_HAVE_MPI
}

BOOST_AUTO_TEST_CASE(TestCore_test)
{
    BOOST_CHECK_EQUAL(helics::CoreFactory::isAvailable(HELICS_TEST), true);

    auto core = helics::CoreFactory::create(HELICS_TEST, "");
    BOOST_CHECK(core != nullptr);
}

BOOST_AUTO_TEST_CASE(InterprocessCore_test)
{
    BOOST_CHECK_EQUAL(helics::CoreFactory::isAvailable(HELICS_INTERPROCESS), false);
}

BOOST_AUTO_TEST_SUITE_END()
