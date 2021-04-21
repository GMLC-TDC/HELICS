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

#include <random>
#include <string>
#include <utility>

/** class implementing a federate for the PHOLD benchmark*/
class PholdFederate: public BenchmarkFederate {
  public:
    int evCount{0};  // number of events handled by this federate

  private:
    helics::Endpoint* ept{nullptr};

    // values from paper "Warp Speed: Executing Time Warp on 1,966,080 Cores"
    // 16 circulating events per LP (also ran with 8 events per LP)
    // 10% remote communications (also ran with 25%, 50%, and 100% remote communications)
    // mean exponential distribution of 0.90
    // optional lookahead of .1 (to prevent arbitrarily small increases in time with conservative
    // simulators)
    unsigned int initEvCount_{16};  // starting number of events
    double localProbability_{.9};  // probability of local events
    double randTimeMean_{
        deltaTime * .9};  // mean for the exponential distribution used when picking event times
    double lookahead_{deltaTime * .1};

    // classes related to the exponential and uniform distribution random number generator
    bool generateRandomSeed{false};
    // some suggestions for seed choice were that not having a majority of the bits as 0 is better
    unsigned int seed{0xABad5eed};
    std::mt19937 rand_gen;
    std::exponential_distribution<double> rand_exp;
    std::uniform_real_distribution<double> rand_uniform_double;
    std::uniform_int_distribution<unsigned int> rand_uniform_int;

  public:
    PholdFederate(): BenchmarkFederate("PHOLD") {}

    // functions for setting parameters
    void setGenerateRandomSeed(bool b) { generateRandomSeed = b; }
    void setRandomSeed(unsigned int s) { seed = s; }
    void setRandomTimeMean(double mean) { randTimeMean_ = mean; }
    void setInitialEventCount(unsigned int count) { initEvCount_ = count; }
    void setLocalProbability(double p) { localProbability_ = p; }
    void setLookahead(double v) { lookahead_ = v; }

    std::string getName() override { return "phold_" + std::to_string(index); }

    void setupArgumentParsing() override
    {
        app->add_option("--init_ev_count", initEvCount_, "the starting number of events");
        app->add_option("--local_probability",
                        localProbability_,
                        "the probability of local events");
        app->add_option("--rand_time_mean",
                        randTimeMean_,
                        "mean for the exponential distribution used when picking event times");
        app->add_flag("--gen_rand_seed", generateRandomSeed, "enable generating a random seed");
        app->add_option("--set_rand_seed", seed, "set the random seed");
        app->add_option("--set_phold_lookahead", lookahead_, "set the lookahead used by phold");
    }

    void doAddBenchmarkResults() override
    {
        addResult("EVENT COUNT", "EvCount", std::to_string(evCount));
    }

    void doParamInit(helics::FederateInfo& /*fi*/) override
    {
        if (app->get_option("--set_rand_seed")->count() == 0) {
            std::mt19937 random_engine(0x600d5eed);  // NOLINT
            std::uniform_int_distribution<unsigned int> rand_seed_uniform;
            for (int i = 0; i < index; i++) {
                (void)rand_seed_uniform(random_engine);
            }
            setRandomSeed(rand_seed_uniform(random_engine));
        }

        // set up based on given params
        // en.cppreference.com/w/cpp/numeric/random/exponential_distribution
        if (generateRandomSeed) {
            std::random_device rd;
            rand_gen.seed(rd());
        } else {
            rand_gen.seed(seed);
        }
        rand_exp = std::exponential_distribution<double>(1.0 / randTimeMean_);
        rand_uniform_double = std::uniform_real_distribution<double>(0.0, 1.0);
        // create random number distribution for picking a destination if there is more than 1
        // federate
        if (maxIndex > 1) {
            rand_uniform_int = std::uniform_int_distribution<unsigned int>(0, maxIndex - 2);
        }
    }

    void doFedInit() override { ept = &fed->registerEndpoint("ept"); }

    void doMakeReady() override
    {
        // create initial events (not included in benchmark time)
        for (unsigned int i = 0; i < initEvCount_; i++) {
            createNewEvent();
        }
    }

    void doMainLoop() override
    {
        auto nextTime = deltaTime;

        while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);
            // for each event message received, create a new event
            while (ept->hasMessage()) {
                auto m = ept->getMessage();
                evCount++;
                createNewEvent();
            }
        }
    }

    void createNewEvent()
    {
        // decide if the event is local or remote
        auto destIndex = index;
        if (maxIndex > 1 && rand_uniform_double(rand_gen) > localProbability_) {
            destIndex = rand_uniform_int(rand_gen);
            if (destIndex == index) {
                destIndex = maxIndex - 1;
            }
        }

        // set the event time to current time + lookahead + rand exponential (mean >= lookahead or
        // ~2x lookahead)
        helics::Time evTime =
            fed->getCurrentTime() + helics::Time(lookahead_) + helics::Time(rand_exp(rand_gen));
        std::string data = "ev";
        std::string dest = "phold_" + std::to_string(destIndex) + "/ept";
        ept->send(dest, data, evTime);
    }
};
