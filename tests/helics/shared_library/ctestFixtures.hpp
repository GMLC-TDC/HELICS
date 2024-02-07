/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../coreTypeLists.hpp"
#include "helics/helics.h"

#include <string>
#include <vector>

#define CE(command)                                                                                \
    helicsErrorClear(&err);                                                                        \
    command;                                                                                       \
    EXPECT_TRUE(err.error_code == HELICS_OK) << err.message

#define HELICS_SIZE_MAX 512

typedef HelicsFederate (*FedCreator)(const char*, HelicsFederateInfo, HelicsError*);

struct FederateTestFixture {
    FederateTestFixture();
    ~FederateTestFixture();

    HelicsBroker AddBroker(const std::string& CoreType_name, int count);
    HelicsBroker AddBroker(const std::string& CoreType_name,
                           const std::string& initialization_string);

    void SetupTest(FedCreator ctor,
                   const std::string& CoreType_name,
                   int count,
                   HelicsTime timeDelta = HELICS_TIME_ZERO,
                   const std::string& name_prefix = "fed");

    void AddFederates(FedCreator ctor,
                      std::string CoreType_name,
                      int count,
                      HelicsBroker broker,
                      HelicsTime time_delta = HELICS_TIME_ZERO,
                      const std::string& name_prefix = "fed");

    HelicsFederate GetFederateAt(int index);

    std::vector<HelicsBroker> brokers;
    std::vector<HelicsFederate> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;
    HelicsError err;

    std::string ctype;
    bool NoFree{false};

  private:
    static bool hasIndexCode(const std::string& type_name);
    static int getIndexCode(const std::string& type_name);
};
