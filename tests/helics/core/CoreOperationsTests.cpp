/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/CoreFactory.hpp"
#include "helics/core/CoreFederateInfo.hpp"
#include "helics/core/core-exceptions.hpp"

#include "gtest/gtest.h"
#include <memory>

TEST(federate_tests, fail_max_federates)
{
    auto core = helics::CoreFactory::create(helics::core_type::TEST,
                                            "--name=core_0 --maxfederates 0 --autobroker");

    helics::CoreFederateInfo cfi;

    EXPECT_THROW(core->registerFederate("fed1_name", cfi), helics::RegistrationFailure);

    EXPECT_FALSE(core->isOpenToNewFederates());
    core->disconnect();
}
