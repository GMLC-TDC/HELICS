/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <random>

#include <gmlc/concurrency/Barrier.hpp>

#include "helics/helics-config.h"

using namespace helics;

// headers used by the MessageExchangeFederate class
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"

using helics::operator"" _t ;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
/** class implementing a federate that sends messages to another (and vice versa)*/
class MessageExchangeFederate
{
  public:
    helics::Time deltaTime = helics::Time (10, time_units::ns);  // sampling rate
    helics::Time finalTime = helics::Time (10, time_units::ns);  // final time
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint ept;
    bool initialized = false;
    bool readyToRun = false;
    
    int msgCount;
    std::string msg;
    std::string dest;

  public:
    MessageExchangeFederate () = default;

    void run (std::function<void()> callOnReady = {}, std::function<void()> callOnEnd = {})
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
        if (callOnEnd)
        {
            callOnEnd ();
        }
    };

    void initialize (const std::string &coreName, int i, int msg_size, int msg_count)
    {
        std::string name = "msgExchange_" + std::to_string (i);
        helics::FederateInfo fi;
        fi.coreName = coreName;
        mFed = std::make_unique<helics::MessageFederate> (name, fi);
        ept = mFed->registerEndpoint ("ept");

        // set the destination to the other federate in the pair
        dest = "msgExchange_" + std::to_string ((i+1)%2) + "/ept";

        // create a message string to send
        msg = std::string (msg_size, '0');
        msgCount = msg_count;

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
        auto cTime = 0.0_t;
        while (cTime < finalTime)
        {
            while (ept.hasMessage ())
            {
                ept.getMessage ();
            }

            for (int i = 0; i < msgCount; i++)
            {
                ept.send (dest, msg);
            }

            cTime = mFed->requestTimeAdvance (deltaTime);
        }
        mFed->finalize ();
    }
};

static void BM_sendMessage (benchmark::State &state, core_type cType, bool singleCore = false)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        int fed_count = 2;
        gmlc::concurrency::Barrier brr (static_cast<size_t>(fed_count+1));

        auto broker = helics::BrokerFactory::create (cType, "brokerb",
                                                     std::string ("--federates=") + std::to_string (fed_count));
        broker->setLoggingLevel (helics_log_level_no_print);
        
        int wcore_fed_count = 1;
        if (!singleCore)
        {
            wcore_fed_count = fed_count;
        }
        std::shared_ptr<helics::Core> wcore;
        std::vector<MessageExchangeFederate> feds (fed_count);
        std::vector<std::shared_ptr<helics::Core>> cores (fed_count);
        
        // create cores and federates
        int msg_size = state.range(0);
        int msg_count = state.range(1);
        if (singleCore)
        {
            wcore = helics::CoreFactory::create (cType, std::string ("--federates=") + std::to_string (wcore_fed_count));
        }
        for (int ii = 0; ii < fed_count; ++ii)
        {
            if (!singleCore)
            {
                cores[ii] = helics::CoreFactory::create (cType, "-f 1");
                cores[ii]->connect ();
                feds[ii].initialize (cores[ii]->getIdentifier (), ii, msg_size, msg_count);
            }
            else
            {
                feds[ii].initialize (wcore->getIdentifier (), ii, msg_size, msg_count);
            }
        }

        // create threads for federates and get most of them running
        std::vector<std::thread> threadlist (static_cast<size_t> (fed_count));
        for (int ii = 0; ii < fed_count; ++ii)
        {
            threadlist[ii] = std::thread ([&] (MessageExchangeFederate &f) { f.run ([&brr] () { brr.wait ();brr.wait(); }, [&brr] () { brr.wait (); }); },
                                          std::ref (feds[ii]));
        }

        // synchronize the federates and run the benchmark with timing
        brr.wait ();
        state.ResumeTiming ();
        brr.wait ();
        brr.wait();
        state.PauseTiming ();

        // clean-up federate threads
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }

        // reset state for next benchmark
        broker->disconnect ();
        broker.reset ();
        cores.clear ();
        wcore.reset ();
        cleanupHelicsLibrary ();

        state.ResumeTiming ();
    }
}

// Some math notes:
// 1 << 6 = 64
// 1 << 10 = 1024
// 1 << 15 = 32KB
// 1 << 20 = 1MB
// 1 << 30 = 1GB

// The first element in the ranges is message size, and the second is message count

// Register the single core benchmark
BENCHMARK_CAPTURE (BM_sendMessage, singleCore, core_type::INPROC, true)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<20}, {1, 1}}) // 1GB takes about 6 seconds
  ->Ranges ({{1, 1}, {1, 1<<9}})
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register multi core benchmarks

static constexpr int64_t maxscale{1 << 5};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, inprocMultiCore, core_type::INPROC)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<20}, {1, 1}}) // 1GB takes about 6 seconds
  ->Ranges ({{1, 1}, {1, 1<<9}})
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, zmqMultiCore, core_type::ZMQ)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<20}, {1, 1}}) // 1GB takes about 30 seconds
  ->Ranges ({{1, 1}, {1, 1<<9}})
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, zmqssMultiCore, core_type::ZMQ_SS)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<20}, {1, 1}})
  ->Ranges ({{1, 1}, {1, 1<<9}})
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, ipcMultiCore, core_type::IPC)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<11}, {1, 1}}) // msg size of 4096 bytes causes Boost transmit error
  ->Ranges ({{1, 1<<11}, {1, 1<<9}}) // msg count has a much bigger effect on time taken (increasing size had no noticeable effect on times)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, tcpMultiCore, core_type::TCP)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<11}, {1, 1}}) // msg size of 4096 bytes causes error/terminate
  ->Ranges ({{1, 1}, {1, 1<<9}}) // msg count has a bigger effect on time taken (increasing size had minimal effect on times)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, tcpssMultiCore, core_type::TCP_SS)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<11}, {1, 1}}) // msg size of 4096 bytes causes error/terminate
  ->Ranges ({{1, 1}, {1, 1<<9}}) // msg count has a bigger effect on time taken (increasing size had minimal effect on times)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_sendMessage, udpMultiCore, core_type::UDP)
  //->RangeMultiplier (2)
  ->Ranges ({{1, 1<<15}, {1, 1}}) // msg size of 65536 bytes causes error/terminate, though somewhere about 8K the benchmark time drops from several ms to <1ms
  ->Ranges ({{1, 1}, {1, 1<<9}}) // msg count has a bigger effect on time taken (increasing size had minimal effect on times); larger sizes/counts did seem to result in hanging, maybe an important packet was lost
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();
#endif
