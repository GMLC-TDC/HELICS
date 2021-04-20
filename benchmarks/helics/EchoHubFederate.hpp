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

#include <string>
#include <vector>

/** class implementing the hub for an echo test*/
class EchoHub: public BenchmarkFederate {
  private:
    std::vector<helics::Publication> pubs;
    std::vector<helics::Input> subs;
    int num_leafs{10};

  public:
    EchoHub(): BenchmarkFederate("EchoHub") {}

    void setupArgumentParsing() override
    {
        // default final time for this benchmark
        finalTime = helics::Time(100, time_units::ms);

        app->add_option("--num_leafs",
                        num_leafs,
                        "the number of echoleaf federates to expect",
                        true);
    }

    std::string getName() override { return "echohub"; }

    void doFedInit() override
    {
        pubs.reserve(num_leafs);
        subs.reserve(num_leafs);
        for (int ii = 0; ii < num_leafs; ++ii) {
            pubs.push_back(fed->registerIndexedPublication<std::string>("leafrx", ii));
            subs.push_back(fed->registerIndexedSubscription("leafsend", ii));
        }
    }

    void doMainLoop() override
    {
        helics::Time cTime{0.0};
        while (cTime <= finalTime) {
            for (int ii = 0; ii < num_leafs; ++ii) {
                if (fed->isUpdated(subs[ii])) {
                    auto& val = fed->getString(subs[ii]);
                    pubs[ii].publish(val);
                }
            }
            cTime = fed->requestTime(finalTime + 0.05);
        }
    }
};
