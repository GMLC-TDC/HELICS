/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <atomic>
#include <condition_variable>
#include <mutex>

class countdown
{
  public:
    countdown (int start) : counter_{start} {}

    void decrement ()
    {
        std::unique_lock<std::mutex> lck (mtx);
        --counter_;
        if (counter_ == 0)
        {
            cv.notify_all ();
        }
    }
    void wait ()
    {
        if (counter_ > 0)
        {
            std::unique_lock<std::mutex> lck (mtx);
            while (counter_.load () > 0)
            {
                cv.wait (lck);
            }
        }
    }

  private:
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> counter_;
};

using helics::operator"" _t ;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;
/** class implementing the hub for an echo test*/
class RingTransmit
{
  public:
    helics::Time deltaTime = helics::Time (10, time_units::ns);  // sampling rate
    helics::Time finalTime = helics::Time (10000, time_units::ns);  // final time
    int loopCount = 0;

  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication *pub;
    helics::Input *sub;

    int index_ = 0;
    int maxIndex_ = 0;
    bool initialized = false;

  public:
    RingTransmit () = default;

    void run (countdown &cdt)
    {
        if (!initialized)
        {
            throw ("must initialize first");
        }
        vFed->enterInitializingModeAsync ();
        cdt.decrement ();
        vFed->enterInitializingModeComplete ();
        vFed->enterExecutingMode ();
        mainLoop ();
    };
    void initialize (const std::string &coreName, int index, int maxIndex)
    {
        std::string name = "ringlink_" + std::to_string (index);
        index_ = index;
        maxIndex_ = maxIndex;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        if (index == 0)
        {
            // fi.setProperty (helics_property_int_log_level, helics_log_level_timing);
        }
        vFed = std::make_unique<helics::ValueFederate> (name, fi);
        pub = &vFed->registerPublicationIndexed<std::string> ("pub", index_);
        sub = &vFed->registerSubscriptionIndexed ("pub", (index_ == 0) ? maxIndex_ - 1 : index_ - 1);

        initialized = true;
    }

    void mainLoop ()
    {
        if (index_ == 0)
        {
            std::string txstring (100, '1');
            pub->publish (txstring);
            ++loopCount;
        }
        auto nextTime = deltaTime;

        while (nextTime < finalTime)
        {
            nextTime = vFed->requestTime (finalTime);
            if (vFed->isUpdated (*sub))
            {
                auto &nstring = vFed->getString (*sub);
                vFed->publish (*pub, nstring);
                ++loopCount;
            }
        }
        vFed->finalize ();
    }
};

static void BM_ring2_singleCore (benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming ();
        int feds = 2;
        countdown cdt (2);
        auto wcore = helics::CoreFactory::create (core_type::TEST, std::string ("--autobroker --federates=2"));
        // this is to delay until the threads are ready
        wcore->setFlagOption (helics::local_core_id, helics_flag_delay_init_entry, true);
        std::vector<RingTransmit> links (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            links[ii].initialize (wcore->getIdentifier (), ii, feds);
        }

        std::vector<std::thread> threadlist (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] = std::thread ([&] (RingTransmit &link) { link.run (cdt); }, std::ref (links[ii]));
        }

        std::this_thread::yield ();
        cdt.wait ();
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
        std::this_thread::yield ();
        state.ResumeTiming ();
        wcore->setFlagOption (helics::local_core_id, helics_flag_enable_init_entry, true);
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }
        state.PauseTiming ();
        if (links[0].loopCount != 10000)
        {
            std::cout << "incorrect loop count received (" << links[0].loopCount << ") instead of 100000"
                      << std::endl;
        }
        state.ResumeTiming ();
    }
}
// Register the function as a benchmark
BENCHMARK (BM_ring2_singleCore)->Unit (benchmark::TimeUnit::kMillisecond)->UseRealTime ()->Iterations (3);

static void BM_ring2_multiCore (benchmark::State &state, core_type cType)
{
    for (auto _ : state)
    {
        state.PauseTiming ();
        int feds = 2;
        countdown cdt (2);
        auto broker = helics::BrokerFactory::create (cType, std::string ("--federates=2"));
        broker->setLoggingLevel (0);

        auto wcore =
          helics::CoreFactory::create (cType, std::string (" --federates=1 --broker=" + broker->getIdentifier ()));
        wcore->connect ();
        // this is to delay until the threads are ready
        wcore->setFlagOption (helics::local_core_id, helics_flag_delay_init_entry, true);
        std::vector<RingTransmit> links (feds);
        auto ocore =
          helics::CoreFactory::create (cType, std::string (" --federates=1 --broker=" + broker->getIdentifier ()));

        links[0].initialize (wcore->getIdentifier (), 0, feds);
        ocore->connect ();
        links[1].initialize (ocore->getIdentifier (), 1, feds);

        std::vector<std::thread> threadlist (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] = std::thread ([&] (RingTransmit &link) { link.run (cdt); }, std::ref (links[ii]));
        }

        std::this_thread::yield ();
        cdt.wait ();
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
        std::this_thread::yield ();
        state.ResumeTiming ();
        wcore->setFlagOption (helics::local_core_id, helics_flag_enable_init_entry, true);
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }
        state.PauseTiming ();
        if (links[0].loopCount != 10000)
        {
            std::cout << "incorrect loop count received (" << links[0].loopCount << ") instead of 100000"
                      << std::endl;
        }
        broker->disconnect ();
        state.ResumeTiming ();
    }
}

// Register the test core benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, testCore, core_type::TEST)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, zmqCore, core_type::ZMQ)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, zmqssCore, core_type::ZMQ_SS)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, ipcCore, core_type::IPC)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, tcpCore, core_type::TCP)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, tcpssCore, core_type::TCP_SS)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();

// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_ring2_multiCore, udpCore, core_type::UDP)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->UseRealTime ();
