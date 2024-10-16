/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreFactory.hpp"

#include "gtest/gtest.h"
#include <string>

TEST(CoreConfig, test1)
{
    std::string configString = "--config=";
    configString.append(TEST_DIR);
    configString.append("core_config/core_config1.json");
    auto cr = helics::CoreFactory::create(helics::CoreType::TEST, configString);
    EXPECT_EQ(cr->getIdentifier(), "core_name1");
    cr->disconnect();
}

TEST(CoreConfig, test2)
{
    std::string configString = "--config_section=core --config=";
    configString.append(TEST_DIR);
    configString.append("core_config/core_config2.json");
    auto cr = helics::CoreFactory::create(helics::CoreType::TEST, configString);
    EXPECT_EQ(cr->getIdentifier(), "core_name2");
    cr->disconnect();
}

TEST(CoreConfig, test3)
{
    std::string configString = "--config_section=core --config_index 1 --config=";
    configString.append(TEST_DIR);
    configString.append("core_config/core_config3.json");
    auto cr = helics::CoreFactory::create(helics::CoreType::TEST, configString);
    EXPECT_EQ(cr->getIdentifier(), "core_name10");
    cr->disconnect();
}

TEST(CoreConfig, getFlagTests)
{
    auto cr =
        helics::CoreFactory::create(helics::CoreType::TEST, "--force_logging_flush --dumplog");
    cr->setFlagOption(helics::gLocalCoreId, HELICS_FLAG_DELAY_INIT_ENTRY, true);

    EXPECT_TRUE(cr->getFlagOption(helics::gLocalCoreId, HELICS_FLAG_DELAY_INIT_ENTRY));
    EXPECT_FALSE(cr->getFlagOption(helics::gLocalCoreId, HELICS_FLAG_ENABLE_INIT_ENTRY));

    EXPECT_TRUE(cr->getFlagOption(helics::gLocalCoreId, HELICS_FLAG_FORCE_LOGGING_FLUSH));
    EXPECT_TRUE(cr->getFlagOption(helics::gLocalCoreId, HELICS_FLAG_DUMPLOG));
    cr->disconnect();
}
