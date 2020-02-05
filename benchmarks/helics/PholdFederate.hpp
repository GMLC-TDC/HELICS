/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/helics-config.h"

#include <random>

using namespace helics;
/** class implementing a federate for the PHOLD benchmark*/
class PholdFederate {
  public:
    helics::Time deltaTime = helics::Time(10, time_units::ns); // sampling rate
    helics::Time finalTime = helics::Time(10000, time_units::ns); // final time
    int evCount = 0; // number of events handled by this federate

  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint* ept;

    int index_ = 0;
    int maxIndex_ = 0;

    // values from paper "Warp Speed: Executing Time Warp on 1,966,080 Cores"
    // 16 circulating events per LP (also ran with 8 events per LP)
    // 10% remote communications (also ran with 25%, 50%, and 100% remote communications)
    // mean exponential distribution of 0.90
    // optional lookahead of .1 (to prevent arbitrarily small increases in time with conservative simulators)
    unsigned int initEvCount_ = 16; // starting number of events
    double localProbability_ = .9; // probability of local events
    double randTimeMean_ =
        deltaTime * .9; // mean for the exponential distribution used when picking event times
    double lookahead_ = deltaTime * .1;

    // classes related to the exponential and uniform distribution random number generator
    bool generateRandomSeed = true;
    unsigned int seed =
        0xABad5eed; // some suggestions for seed choice were that not having a majority of the bits as 0 is better
    std::mt19937 rand_gen;
    std::exponential_distribution<double> rand_exp;
    std::uniform_real_distribution<double> rand_uniform_double;
    std::uniform_int_distribution<unsigned int> rand_uniform_int;

    // callbacks for instrumenting the code
    std::function<void()> callBeforeFinalize = nullptr;
    std::function<void()> callAfterFinalize = nullptr;

    bool initialized{false};
    bool readyToRun{false};

  public:
    PholdFederate() = default;

    // functions for setting parameters
    void setGenerateRandomSeed(bool b) { generateRandomSeed = b; };
    void setRandomSeed(unsigned int s) { seed = s; };
    void setRandomTimeMean(double mean) { randTimeMean_ = mean; };
    void setInitialEventCount(unsigned int count) { initEvCount_ = count; };
    void setLocalProbability(double p) { localProbability_ = p; };

    // functions for setting callbacks
    void setBeforeFinalizeCallback(std::function<void()> cb = nullptr) { callBeforeFinalize = cb; };
    void setAfterFinalizeCallback(std::function<void()> cb = nullptr) { callAfterFinalize = cb; };

    void run(std::function<void()> callOnReady = nullptr)
    {
        makeReady();
        if (callOnReady) {
            callOnReady();
        }
        mainLoop();
    };

    void initialize(const std::string& coreName, int index, int maxIndex)
    {
        helics::FederateInfo fi;
        fi.coreName = coreName;
        initialize(fi, index, maxIndex);
    }

    void initialize(const helics::FederateInfo fi, int index, int maxIndex)
    {
        std::string name = "phold_" + std::to_string(index);
        index_ = index;
        maxIndex_ = maxIndex;

        mFed = std::make_unique<helics::MessageFederate>(name, fi);
        ept = &mFed->registerEndpoint("ept");

        // en.cppreference.com/w/cpp/numeric/random/exponential_distribution
        if (generateRandomSeed) {
            std::random_device rd;
            rand_gen.seed(rd());
        } else {
            rand_gen.seed(seed);
        }
        rand_exp = std::exponential_distribution<double>(1.0 / randTimeMean_);
        rand_uniform_double = std::uniform_real_distribution<double>(0.0, 1.0);
        // create random number distribution for picking a destination if there is more than 1 federate
        if (maxIndex_ > 1) {
            rand_uniform_int = std::uniform_int_distribution<unsigned int>(0, maxIndex_ - 2);
        }
        initialized = true;
    }

    void finalize()
    {
        if (callBeforeFinalize) {
            callBeforeFinalize();
        }
        mFed->finalize();
        if (callAfterFinalize) {
            callAfterFinalize();
        }
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        mFed->enterExecutingMode();

        // create initial events (not included in benchmark time)
        for (unsigned int i = 0; i < initEvCount_; i++) {
            createNewEvent();
        }

        readyToRun = true;
    }

    void createNewEvent()
    {
        // decide if the event is local or remote
        auto destIndex = index_;
        if (maxIndex_ > 1 && rand_uniform_double(rand_gen) > localProbability_) {
            destIndex = rand_uniform_int(rand_gen);
            if (destIndex == index_) {
                destIndex = maxIndex_ - 1;
            }
        }

        // set the event time to current time + lookahead + rand exponential (mean >= lookahead or ~2x lookahead)
        helics::Time evTime =
            mFed->getCurrentTime() + helics::Time(lookahead_) + helics::Time(rand_exp(rand_gen));
        std::string data = "ev";
        std::string dest = "phold_" + std::to_string(destIndex) + "/ept";
        ept->send(dest, data, evTime);
    }

    void mainLoop()
    {
        auto nextTime = deltaTime;

        while (nextTime < finalTime) {
            nextTime = mFed->requestTime(finalTime);
            // for each event message received, create a new event
            while (ept->hasMessage()) {
                auto m = ept->getMessage();
                evCount++;
                createNewEvent();
            }
        }
        finalize();
    }
};
