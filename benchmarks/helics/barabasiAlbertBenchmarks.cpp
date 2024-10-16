/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BarabasiAlbertFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using helics::CoreType;

static constexpr int64_t maxscale{1 << (4 + HELICS_BENCHMARK_SHIFT_FACTOR)};

/*
 * In this benchmark # nodes (n) would be the # of federates
 * The Barabasi Albert algorithm starts with some m number
 * of connected nodes. For the purpose of this benchmark
 * we start with nodes 0 & 1 connected to each other.
 * All subsequent nodes will have a probabiltiy of
 * connecting to the pre-existing nodes based on the following
 * probability:
 *       p = ki/sum(kj)
 * where p is the probability of a link forming
 * ki is the degree of pre-existing node i
 * sum(kj) is the sum of the degrees of all pre-existing nodes
 *
 */

/* class implementing a node representation*/
class Node {
  public:
    void setName(std::string nm) { name = std::move(nm); }  // set node name
    void setNumLinks(int x) { num_links = x; }  // set number of links connected to node
    std::string getName() { return name; }  // get node name
    [[nodiscard]] int getNumLinks() const
    {
        return num_links;
    }  // get number of links connected to node
    std::vector<std::string> getTargets()
    {
        return targets;
    }  // get a list of target destinations node will link to
    void pushTargets(const std::string& nm) { targets.push_back(nm); }  // add to targets list

  private:
    std::string name = "";
    int num_links = 0;
    std::vector<std::string> targets;

  public:
    Node() = default;
};

static std::vector<Node> createTopology(int n, int m)
{
    if (m < 2 || m > n) {
        std::cerr << "ERROR: M can't be less than 2 or more than the federate count"

                  << std::endl;
        exit(1);
    }
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, 99);

    int total_links = 0;
    std::vector<Node> nodes;
    nodes.reserve(n);
    std::vector<std::vector<std::string>> targets_vector;
    for (int i = 0; i < n; i++) {
        std::string name = "BA_" + std::to_string(i);
        std::vector<std::string> t;
        targets_vector.push_back(t);
        nodes.emplace_back(Node());
        nodes.at(i).setName(name);
        nodes.at(i).setNumLinks(0);
    }

    for (int m1 = m - 1; m1 > 0; m1--) {
        // link m initial nodes together
        for (int m2 = m1 - 1; m2 >= 0; m2--) {
            // ADD TARGETS HERE
            nodes[m1].pushTargets(nodes[m2].getName() + "/ept,");
            nodes[m2].pushTargets(nodes[m1].getName() + "/ept,");
            nodes[m1].setNumLinks(nodes[m1].getNumLinks() + 1);
            nodes[m2].setNumLinks(nodes[m2].getNumLinks() + 1);

            total_links++;
        }
    }
    // the rest of the topology building here
    for (int i = m; i < n; i++) {
        for (int j = i - 1; j >= 0; j--) {
            int rnd = distribution(generator);

            int p = (float(nodes[j].getNumLinks()) / total_links) * 100;
            if (rnd < p) {
                nodes[i].pushTargets(nodes[j].getName() + "/ept,");
                nodes[j].pushTargets(nodes[i].getName() + "/ept,");
                nodes[i].setNumLinks(nodes[i].getNumLinks() + 1);
                nodes[j].setNumLinks(nodes[j].getNumLinks() + 1);
                total_links++;
            }
        }
    }

    return nodes;
}

static void BM_BarabasiAlbert_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = 2;
        int m = 2;
        std::vector<Node> nodes = createTopology(feds, m);

        gmlc::concurrency::Barrier brr(feds);

        auto wcore = helics::CoreFactory::create(
            CoreType::INPROC,  //||ZMQ core, TCP, UDP, MPI?
            std::string(
                "--autobroker --federates=2 --restrictive_time_policy --broker_init_string=\"--restrictive_time_policy\""));

        std::vector<BarabasiAlbertFederate> links(feds);
        for (int i = 0; i < feds; i++) {
            std::string s;
            if (nodes[i].getTargets().empty()) {
                s = "None";
            }
            for (auto const& t : nodes[i].getTargets()) {
                s += t;
            }
            std::string bmInit = "--index=" + std::to_string(i) +
                " --max_index=" + std::to_string(feds) + " --targets=" + s;
            links[i].initialize(wcore->getIdentifier(), bmInit);
        }

        std::thread nodesThread(
            [&](BarabasiAlbertFederate& link) { link.run([&brr]() { brr.wait(); }); },
            std::ref(links[1]));

        links[0].makeReady();

        brr.wait();

        state.ResumeTiming();

        links[0].run();
        state.PauseTiming();
        nodesThread.join();

        wcore.reset();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}

static void BM_BarabasiAlbert_multicore(benchmark::State& state, CoreType cType)
{
    for (auto _ : state) {
        state.PauseTiming();
        int m = 2;

        int feds = static_cast<int>(state.range(0));
        std::vector<Node> nodes = createTopology(feds, m);

        gmlc::concurrency::Barrier brr(feds);
        auto broker =
            helics::BrokerFactory::create(cType,
                                          std::string("--restrictive_time_policy --federates=") +
                                              std::to_string(feds));
        broker->setLoggingLevel(0);

        std::vector<BarabasiAlbertFederate> links(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);

        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(
                cType,
                std::string("--restrictive_time_policy --federates=1 --broker=" +
                            broker->getIdentifier()));
            cores[ii]->connect();
            std::string s;
            if (nodes[ii].getTargets().empty()) {
                s = "None";
            }
            for (auto const& t : nodes[ii].getTargets()) {
                s += t;
            }
            std::string bmInit = "--index=" + std::to_string(ii) +
                " --max_index=" + std::to_string(feds) + " --targets=" + s;
            links[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }

        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] = std::thread(
                [&](BarabasiAlbertFederate& link) { link.run([&brr]() { brr.wait(); }); },
                std::ref(links[ii + 1]));
        }

        links[0].makeReady();
        brr.wait();

        state.ResumeTiming();
        links[0].run();
        state.PauseTiming();
        for (auto& thrd : threadlist) {
            thrd.join();
        }

        broker->disconnect();
        broker.reset();
        cores.clear();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}

static void BarabasiAlbertArguments(benchmark::internal::Benchmark* b)
{
    for (int f = 2; f <= maxscale; f *= 2) {
        b->Args({f});
    }
}

// Single-core BM
BENCHMARK(BM_BarabasiAlbert_singleCore)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime()
    ->Iterations(3);

BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, inprocCore, CoreType::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();

#ifdef HELICS_ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, zmqCore, CoreType::ZMQ)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, zmqssCore, CoreType::ZMQ_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();
#endif

#ifdef HELICS_ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, ipcCore, CoreType::IPC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();
#endif

#ifdef HELICS_ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, tcpCore, CoreType::TCP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, tcpssCore, CoreType::TCP_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();
#endif

// Register the UDP benchmarks
#ifdef HELICS_ENABLE_UDP_CORE
BENCHMARK_CAPTURE(BM_BarabasiAlbert_multicore, udpCore, CoreType::UDP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(BarabasiAlbertArguments)
    ->UseRealTime();
#endif

HELICS_BENCHMARK_MAIN(BarabasiAlbertBenchmark);
