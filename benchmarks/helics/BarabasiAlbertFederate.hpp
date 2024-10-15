/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "BenchmarkFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/queryFunctions.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/Core.hpp"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <string>
#include <utility>
#include <vector>

/* class implementing nodes within a Barabasi-Albert network*/
class BarabasiAlbertFederate: public BenchmarkFederate {
  public:
    int initialMessageCount = 10;  // number of messages the federate should send when it starts

  private:
    helics::Endpoint* ept = nullptr;
    std::string targets;
    std::string sb;
    std::vector<std::string> targets_vector;

  public:
    BarabasiAlbertFederate(): BenchmarkFederate("BarabasiAlbertFederate") {}
    std::string getName() override { return getNameForIndex(index); }
    static std::string getNameForIndex(int index) { return "BA_" + std::to_string(index); }

    void setupArgumentParsing() override
    {
        deltaTime = helics::Time(10, time_units::ns);
        finalTime = helics::Time(5000, time_units::ns);
        opt_index->required();
        opt_max_index->required();
        app->add_option("--targets", targets, "targets this node federate will form links with");
    }

    void doFedInit() override
    {
        ept = &fed->registerTargetedEndpoint("ept");

        if (targets == "None") {
            return;
        }

        auto core = fed->getCorePointer();

        std::stringstream targetStream(targets);

        while (std::getline(targetStream, sb, ',')) {
            targets_vector.push_back(sb);
        }
        for (auto& target : targets_vector) {
            core->linkEndpoints(ept->getName(), target);
        }
    }
    void doMakeReady() override
    {
        // send initial messages
        std::string txstring(100, '1');
        for (int ii = 0; ii < initialMessageCount; ii++) {
            ept->send(txstring);
        }
    }
    void doMainLoop() override
    {
        auto nextTime = helics::timeZero;

        while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);
            while (ept->hasMessage()) {
                auto message = fed->getMessage(*ept);
                message->source = ept->getName();
                message->time = fed->getCurrentTime() + deltaTime;
                ept->send(std::move(message));
            }
        }
    }
};
