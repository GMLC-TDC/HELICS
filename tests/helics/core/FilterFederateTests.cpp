/*
Copyright (c) 2017-2024,
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
    FilterFederate a(GlobalFederateId{23}, "name1", GlobalBrokerId{22342}, nullptr);
    auto res = a.createFilter(GlobalBrokerId{}, InterfaceHandle{0}, "tey1", "", "", false);
    EXPECT_NE(res, nullptr);
}

TEST(FilterFederateTests, constructor_test2)
{
    auto a = std::make_unique<FilterFederate>(GlobalFederateId{23},
                                              "name1",
                                              GlobalBrokerId{22342},
                                              nullptr);
    auto res = a->createFilter(GlobalBrokerId{}, InterfaceHandle{0}, "tey1", "", "", false);
    EXPECT_NE(res, nullptr);
}
