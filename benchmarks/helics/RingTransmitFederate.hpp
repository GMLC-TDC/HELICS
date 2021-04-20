/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "BenchmarkFederate.hpp"
#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"

#include <string>

/** class implementing a token ring using a value being passed as the token*/
class RingTransmit: public BenchmarkFederate {
  public:
    int loopCount = 0;

  private:
    helics::Publication* pub = nullptr;
    helics::Input* sub = nullptr;

  public:
    RingTransmit(): BenchmarkFederate("RingTransmit") {}

    std::string getName() override { return "ringlink_" + std::to_string(index); }

    void setupArgumentParsing() override
    {
        deltaTime = helics::Time(10, time_units::ns);
        finalTime = helics::Time(5000, time_units::ns);

        opt_index->required();
        opt_max_index->required();
    }

    void doParamInit(helics::FederateInfo& fi) override
    {
        fi.setFlagOption(helics_flag_restrictive_time_policy);
        if (index == 0) {
            // fi.setProperty(helics_property_int_log_level, helics_log_level_timing);
        }
    }

    void doFedInit() override
    {
        pub = &fed->registerIndexedPublication<std::string>("pub", index);
        sub = &fed->registerIndexedSubscription("pub", (index == 0) ? maxIndex - 1 : index - 1);
    }

    void doMainLoop() override
    {
        if (index == 0) {
            std::string txstring(100, '1');
            pub->publish(txstring);
            ++loopCount;
        }
        auto nextTime = deltaTime;

        while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);
            if (fed->isUpdated(*sub)) {
                auto& nstring = fed->getString(*sub);
                fed->publish(*pub, nstring);
                ++loopCount;
            }
        }
    }
};
