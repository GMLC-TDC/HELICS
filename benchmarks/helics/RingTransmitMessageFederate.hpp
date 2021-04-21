/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "BenchmarkFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"

#include <string>
#include <utility>

/** class implementing a token ring using messages as the transmission mechanism*/
class RingTransmitMessage: public BenchmarkFederate {
  public:
    int loopCount{0};

  private:
    helics::Endpoint* ept{nullptr};

  public:
    RingTransmitMessage(): BenchmarkFederate("RingTransmitMessage") {}

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
    }

    void doFedInit() override
    {
        ept = &fed->registerIndexedEndpoint("ept", index);
        ept->setDefaultDestination("ept_" +
                                   std::to_string((index == maxIndex - 1) ? 0 : (index + 1)));
    }

    void doMainLoop() override
    {
        if (index == 0) {
            std::string txstring(100, '1');
            ept->send(txstring);
            ++loopCount;
        }
        auto nextTime = deltaTime;

        while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);
            if (ept->hasMessage()) {
                auto m = fed->getMessage(*ept);
                m->dest = ept->getDefaultDestination();
                m->source = ept->getName();
                ept->send(std::move(m));
                ++loopCount;
            }
        }
    }
};
