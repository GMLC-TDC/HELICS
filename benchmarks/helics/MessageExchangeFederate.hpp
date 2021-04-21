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
#include "helics/helics-config.h"

#include <string>

/** class implementing a federate that sends messages to another (and vice versa)*/
class MessageExchangeFederate: public BenchmarkFederate {
  private:
    helics::Endpoint ept;

    int msgCount{0};
    int msgSize{0};
    std::string msg;
    std::string dest;

  public:
    MessageExchangeFederate(): BenchmarkFederate("MessageExchange") {}

    std::string getName() override { return "msgExchange_" + std::to_string(index); }

    void setupArgumentParsing() override
    {
        // set custom defaults for the time parameters
        deltaTime = helics::Time(10, time_units::ns);
        finalTime = helics::Time(10, time_units::ns);

        opt_index->required();
        app->add_option("--msg_count", msgCount, "the number of messages to send")->required();
        app->add_option("--msg_size", msgSize, "the size of the messages to send")->required();
    }

    void doParamInit(helics::FederateInfo& /*fi*/) override
    {
        // set the destination to the other federate in the pair
        dest = "msgExchange_" + std::to_string((index + 1) % 2) + "/ept";

        // create a message string to send
        msg = std::string(msgSize, '0');
    }

    void doFedInit() override { ept = fed->registerEndpoint("ept"); }

    void doMainLoop() override
    {
        auto cTime = helics::timeZero;
        while (cTime < finalTime) {
            while (ept.hasMessage()) {
                ept.getMessage();
            }

            for (int i = 0; i < msgCount; i++) {
                ept.send(dest, msg);
            }

            cTime = fed->requestTimeAdvance(deltaTime);
        }
    }
};
