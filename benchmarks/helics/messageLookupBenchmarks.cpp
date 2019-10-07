/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>

#include <gmlc/concurrency/Barrier.hpp>

#include "helics/helics-config.h"

using helics::operator"" _t ;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;
/** class implementing the hub for an echo test*/
class messageGenerator
{
  public:
    helics::Time finalTime = helics::Time (100, time_units::ms);  // final time
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    std::vector<helics::Endpoint> epts;
    int cnt_local_ = 10;
    int cnt_total_ = 10;
    int perloop_message_ = 10;
    int startIndex_ = 0;
    bool initialized = false;
    bool readyToRun = false;

  public:
    messageGenerator () = default;

    void run (std::function<void()> callOnReady = {})
    {
        if (!readyToRun)
        {
            makeReady ();
        }
        if (callOnReady)
        {
            callOnReady ();
        }
        mainLoop ();
    };

    void initialize (const std::string &coreName, int cntlocal, int cnttotal, int perloop, int index)
    {
        cnt_local_ = cntlocal;
        cnt_total_ = cnttotal;
        perloop_message_ = perloop;
        startIndex_ = index * cntlocal;
        std::string name = "mgen_" + std::to_string (index);
        helics::FederateInfo fi;
        fi.coreName = coreName;
        fi.setProperty (helics_property_time_period, 1.0);
        mFed = std::make_unique<helics::MessageFederate> (name, fi);
        epts.reserve (cnt_local_);
        for (int ii = 0; ii < cnt_local_; ++ii)
        {
            epts.push_back (mFed->registerGlobalEndpoint ("ept_" + std::to_string (index * cntlocal + ii)));
        }
        initialized = true;
    }

    void makeReady ()
    {
        if (!initialized)
        {
            throw ("must initialize first");
        }
        mFed->enterExecutingMode ();
        readyToRun = true;
    }

    void mainLoop ()
    {
        std::random_device rd;  // obtain a random number from hardware
        std::mt19937 eng (rd ());  // seed the generator
        std::uniform_int_distribution<> messageDest (0, cnt_total_ - 1);  // define possible destinations
        std::uniform_int_distribution<> messageSource (0, cnt_local_ - 1);  // define possible sources
        const std::string message = "hello";
        const std::string destName = "ept_";
        auto cTime = 0.0_t;
        for (int jj = 0; jj < 100; ++jj)
        {
            for (int ii = 0; ii < perloop_message_; ++ii)
            {
                auto src = messageSource (eng);
                auto dest = messageDest (eng);
                epts[src].send (destName + std::to_string (dest), message);
            }
            mFed->requestNextStep ();
            auto m = mFed->getMessage ();
            while (m)
            {
                m = mFed->getMessage ();
            }
        }

        mFed->finalize ();
    }
};

static void BM_mgen_singleFed (benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        auto wcore = helics::CoreFactory::create (core_type::TEST, std::string ("--autobroker "));
        messageGenerator mgen;
        mgen.initialize (wcore->getIdentifier (), static_cast<int> (state.range (0)),
                         static_cast<int> (state.range (0)), 100, 0);

        mgen.makeReady ();
        state.ResumeTiming ();
        mgen.run ();
        state.PauseTiming ();
        wcore.reset ();
        cleanupHelicsLibrary ();
        state.ResumeTiming ();
    }
}
// Register the function as a benchmark
BENCHMARK (BM_mgen_singleFed)
  ->RangeMultiplier (4)
  ->Range (1, 1 << 18)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (2)
  ->UseRealTime ();
