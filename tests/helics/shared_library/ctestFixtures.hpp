/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../coreTypeLists.hpp"
#include "helics/chelics.h"

#include <string>
#include <vector>

#define CE(command)                                                                                \
    helicsErrorClear(&err);                                                                        \
    command;                                                                                       \
    EXPECT_TRUE(err.error_code == helics_ok) << err.message

#define HELICS_SIZE_MAX 512

typedef helics_federate (*FedCreator)(const char*, helics_federate_info, helics_error*);

struct FederateTestFixture {
    FederateTestFixture();
    ~FederateTestFixture();

    helics_broker AddBroker(const std::string& core_type_name, int count);
    helics_broker AddBroker(const std::string& core_type_name,
                            const std::string& initialization_string);

    void SetupTest(FedCreator ctor,
                   const std::string& core_type_name,
                   int count,
                   helics_time time_delta = helics_time_zero,
                   const std::string& name_prefix = "fed");

    void AddFederates(FedCreator ctor,
                      std::string core_type_name,
                      int count,
                      helics_broker broker,
                      helics_time time_delta = helics_time_zero,
                      const std::string& name_prefix = "fed");

    helics_federate GetFederateAt(int index);

    std::vector<helics_broker> brokers;
    std::vector<helics_federate> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;
    helics_error err;

    std::string ctype;

  private:
    bool hasIndexCode(const std::string& type_name);
    int getIndexCode(const std::string& type_name);
};
