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
#include "helics/core/ActionMessage.hpp"

#include <string>

/** class implementing the leaf for a timing test*/
class TimingLeaf: public BenchmarkFederate {
  private:
    helics::Publication pub;
    helics::Input sub;

  public:
    TimingLeaf(): BenchmarkFederate("TimingLeaf") {}

    std::string getName() override { return "timingleaf_" + std::to_string(index); }

    void setupArgumentParsing() override { opt_index->required(); }

    void doFedInit() override
    {
        pub = fed->registerPublicationIndexed<std::string>("leafsend", index);
        sub = fed->registerSubscriptionIndexed("leafrx", index);
    }

    void doMainLoop() override
    {
        int cnt = 0;
        const int iter = 5000;
        while (cnt <= iter + 1) {
            fed->requestNextStep();
            ++cnt;
        }
    }
};
