/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "BenchmarkFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/core/ActionMessage.hpp"

#include <string>

/* class implementing a leaf for the echo message benchmark*/
class EchoMessageLeaf: public BenchmarkFederate {
  private:
    helics::Endpoint ept;

  public:
    EchoMessageLeaf(): BenchmarkFederate("EchoMessageLeaf") {}

    void setupArgumentParsing() override { opt_index->required(); }

    std::string getName() override { return "echoleaf_" + std::to_string(index); }

    void doFedInit() override
    {
        // this is a local endpoint
        ept = fed->registerEndpoint("leaf");
    }

    void doMainLoop() override
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has
        // sufficient length to get beyond SSO
        const std::string txstring = std::to_string(100000 + index) + std::string(100, '1');
        const int iter = 5000;
        while (cnt <= iter + 1) {
            fed->requestNextStep();
            ++cnt;
            if (cnt <= iter) {
                ept.send("echo", txstring);
            }
            while (ept.hasMessage()) {
                auto m = ept.getMessage();
                auto& nstring = m->data.to_string();
                if (nstring != txstring) {
                    throw("incorrect string");
                }
            }
        }
    }
};
