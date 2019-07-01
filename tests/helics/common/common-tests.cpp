/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/helics-config.h"

#include "gtest/gtest.h"

#ifdef ENABLE_ZMQ_CORE
#include "cppzmq/zmq.hpp"
#include "helics/common/zmqContextManager.h"
#endif

#include <helics/core/BrokerFactory.hpp>
#include <helics/core/CoreFactory.hpp>

struct globalTestConfig : public ::testing::Environment
{
    virtual void TearDown () override
    {
#ifdef ENABLE_ZMQ_CORE
#ifdef __APPLE__
        if (ZmqContextManager::setContextToLeakOnDelete ())
        {
            ZmqContextManager::getContext ().close ();
        }
#endif
#endif
        helics::CoreFactory::cleanUpCores ();
        helics::BrokerFactory::cleanUpBrokers ();
    }
};

// register the global setup and teardown structure
::testing::Environment *const foo_env = ::testing::AddGlobalTestEnvironment (new globalTestConfig);
//____________________________________________________________________________//
