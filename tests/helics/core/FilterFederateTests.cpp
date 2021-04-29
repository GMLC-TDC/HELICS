/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/FilterFederate.hpp"

#include "gtest/gtest.h"
#include <memory>

using namespace helics;

TEST(FilterFederateTests, constructor_test)
{
    FilterFederate a(global_federate_id{23}, "name1", global_broker_id{22342}, nullptr);
    auto res = a.createFilter(global_broker_id{}, interface_handle(0), "tey1", "", "", false);
    EXPECT_NE(res, nullptr);
}

TEST(FilterFederateTests, constructor_test2)
{
    auto a = std::make_unique<FilterFederate>(global_federate_id{23},
                                              "name1",
                                              global_broker_id{22342},
                                              nullptr);
    auto res = a->createFilter(global_broker_id{}, interface_handle(0), "tey1", "", "", false);
    EXPECT_NE(res, nullptr);
}
