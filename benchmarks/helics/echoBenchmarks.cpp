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
class EchoHub
{
  public:
    helics::Time finalTime = helics::Time (1000, time_units::ns);  // final time
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    std::vector<helics::Publication> pubs;
    std::vector<helics::Input> subs;
    int cnt_ = 10;
    bool initialized = false;

  public:
    EchoHub () = default;

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

    void initialize (const std::string &coreName, int cnt)
    {
        cnt_ = cnt;
        std::string name = "echohub";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate> (name, fi);
        pubs.reserve (cnt_);
        subs.reserve (cnt_);
        for (int ii = 0; ii < cnt_; ++ii)
        {
            pubs.push_back (vFed->registerPublicationIndexed<std::string> ("leafrx", ii));
            subs.push_back (vFed->registerSubscriptionIndexed ("leafsend", ii));
        }
        initialized = true;
    }

    void mainLoop ()
    {
        auto cTime = 0.0_t;
        while (cTime <= finalTime)
        {
            for (int ii = 0; ii < cnt_; ++ii)
            {
                if (vFed->isUpdated (subs[ii]))
                {
                    auto &val = vFed->getString (subs[ii]);
                    pubs[ii].publish (val);
                }
            }
            cTime = vFed->requestTime (finalTime + 0.05);
        }
        vFed->finalize ();
    }
};

class EchoLeaf
{
  public:
    helics::Time deltaTime = helics::Time (10, time_units::ns);  // sampling rate
    helics::Time finalTime = helics::Time (1000, time_units::ns);  // final time
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication pub;
    helics::Input sub;

    int index_ = 0;
    bool initialized = false;

  public:
    EchoLeaf () = default;

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
    void initialize (const std::string &coreName, int index)
    {
        std::string name = "echoleaf_" + std::to_string (index);
        index_ = index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate> (name, fi);
        pub = vFed->registerPublicationIndexed<std::string> ("leafsend", index_);
        sub = vFed->registerSubscriptionIndexed ("leafrx", index_);
        initialized = true;
    }

    void mainLoop ()
    {
        auto nextTime = deltaTime;
        std::string txstring (100, '1');
        while (nextTime < finalTime + deltaTime / 2)
        {
            nextTime = vFed->requestTime (nextTime + deltaTime);
            if (nextTime <= finalTime)
            {
                vFed->publish (pub, txstring);
            }
            if (vFed->isUpdated (sub))
            {
                auto &nstring = vFed->getString (sub);
                if (nstring != txstring)
                {
                    std::cout << "incorrect string\n";
                    break;
                }
            }
        }
        vFed->finalize ();
    }
};

static void BM_echo_singleCore (benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        int feds = static_cast<int> (state.range (0));
        countdown cdt (feds + 1);
        auto wcore = helics::CoreFactory::create (core_type::TEST, std::string ("--autobroker --federates=") +
                                                                     std::to_string (feds + 1));
        // this is to delay until the threads are ready
        wcore->setFlagOption (helics::local_core_id, helics_flag_delay_init_entry, true);
        EchoHub hub;
        hub.initialize (wcore->getIdentifier (), feds);
        std::vector<EchoLeaf> leafs (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            leafs[ii].initialize (wcore->getIdentifier (), ii);
        }

        std::vector<std::thread> threadlist (static_cast<size_t> (feds) + 1);
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] = std::thread ([&] (EchoLeaf &lf) { lf.run (cdt); }, std::ref (leafs[ii]));
        }
        threadlist[feds] = std::thread ([&] () { hub.run (cdt); });
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
    }
}
// Register the function as a benchmark
BENCHMARK (BM_echo_singleCore)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 6)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

static void BM_echo_multiCore (benchmark::State &state, core_type cType)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        int feds = static_cast<int> (state.range (0));
        countdown cdt (feds + 1);

        auto broker = helics::BrokerFactory::create (cType, "brokerb",
                                                     std::string ("--federates=") + std::to_string (feds + 1));
        broker->setLoggingLevel (0);
        auto wcore = helics::CoreFactory::create (cType, std::string ("--federates=1"));
        // this is to delay until the threads are ready
        wcore->setFlagOption (helics::local_core_id, helics_flag_delay_init_entry, true);
        EchoHub hub;
        hub.initialize (wcore->getIdentifier (), feds);
        std::vector<EchoLeaf> leafs (feds);
        std::vector<std::shared_ptr<helics::Core>> cores (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            cores[ii] = helics::CoreFactory::create (cType, "-f 1");
            cores[ii]->connect ();
            leafs[ii].initialize (cores[ii]->getIdentifier (), ii);
        }

        std::vector<std::thread> threadlist (static_cast<size_t> (feds) + 1);
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] = std::thread ([&] (EchoLeaf &lf) { lf.run (cdt); }, std::ref (leafs[ii]));
        }
        threadlist[feds] = std::thread ([&] () { hub.run (cdt); });
        std::this_thread::yield ();
        cdt.wait ();
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        std::this_thread::yield ();
        state.ResumeTiming ();
        wcore->setFlagOption (helics::local_core_id, helics_flag_enable_init_entry, true);
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }
        state.PauseTiming ();
        broker->disconnect ();
    }
}

// Register the test core benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, testCore, core_type::TEST)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, zmqCore, core_type::ZMQ)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, zmqssCore, core_type::ZMQ_SS)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, ipcCore, core_type::IPC)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, tcpCore, core_type::TCP)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, tcpssCore, core_type::TCP_SS)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_echo_multiCore, udpCore, core_type::UDP)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();
