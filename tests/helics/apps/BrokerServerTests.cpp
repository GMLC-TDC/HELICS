/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#ifdef _MSC_VER
#pragma warning(push, 0)
#include "helics/external/filesystem.hpp"
#pragma warning(pop)
#else
#include "helics/external/filesystem.hpp"
#endif

#include "exeTestHelper.h"
#include "helics/apps/BrokerServer.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"

#include <cstdio>
#include <future>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (broker_server_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (startup_tests)
{
    helics::apps::BrokerServer brks (std::vector<std::string>{"--zmq"});
    bool active = brks.hasActiveBrokers ();
    BOOST_CHECK (!active);
    brks.startServers ();

    auto cr = helics::CoreFactory::create (helics::core_type::ZMQ, "--brokername=fred");
    BOOST_CHECK (cr->isConfigured ());
    cr->connect ();
    BOOST_CHECK (cr->isConnected ());
    brks.forceTerminate ();

    cr->waitForDisconnect (std::chrono::milliseconds (1000));
}

BOOST_AUTO_TEST_SUITE_END ()
