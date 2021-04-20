/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/chelics.h"

#include "gtest/gtest.h"
#include <iostream>

struct globalTestConfig: public ::testing::Environment {
    virtual void TearDown() override
    {
        // std::cout << "cleaning up" << std::endl;
        helicsCloseLibrary();
        // std::cout << "finished cleaning up" << std::endl;
    }
};

// register the global setup and teardown structure
::testing::Environment* const foo_env = ::testing::AddGlobalTestEnvironment(new globalTestConfig);
