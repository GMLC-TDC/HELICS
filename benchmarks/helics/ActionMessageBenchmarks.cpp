/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/ActionMessage.hpp"
#include "helics_benchmark_main.h"

using namespace helics;

static void BM_AM_toString(benchmark::State& state)
{
    ActionMessage obj(CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData("this is a new string to add to the string data");
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        obj.to_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_toString);

static void BM_AM_FromString(benchmark::State& state)
{
    ActionMessage obj(CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData("this is a new string to add to the string data");
    std::string load;
    load.reserve(500);
    obj.to_string(load);
    ActionMessage conv;

    for (auto _ : state) {
        conv.from_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_FromString);

static void BM_AM_toString_time(benchmark::State& state)
{
    ActionMessage obj(CMD_TIME_REQUEST);
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        obj.to_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_toString_time);

static void BM_AM_FromString_time(benchmark::State& state)
{
    ActionMessage obj(CMD_TIME_REQUEST);
    std::string load;
    load.reserve(500);
    obj.to_string(load);
    ActionMessage conv;

    for (auto _ : state) {
        conv.from_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_FromString_time);

static void BM_AM_packetize(benchmark::State& state)
{
    ActionMessage obj(CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData("this is a new string to add to the string data");
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        obj.packetize(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_packetize);

static void BM_AM_depacketize(benchmark::State& state)
{
    ActionMessage obj(CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData("this is a new string to add to the string data");
    std::string load;
    load.reserve(500);
    obj.packetize(load);
    ActionMessage conv;

    for (auto _ : state) {
        conv.depacketize(load.data(), static_cast<int>(load.size()));
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_depacketize);

static void BM_AM_packetize_strings(benchmark::State& state)
{
    ActionMessage obj(CMD_MULTI_MESSAGE);
    obj.name = "sstring";
    for (int ii = 0; ii < 100; ++ii) {
        obj.setString(ii, ActionMessage(CMD_PING_REPLY).to_string());
    }
    std::string load;
    load.reserve(50000);
    for (auto _ : state) {
        obj.packetize(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_packetize_strings);

static void BM_AM_depacketize_strings(benchmark::State& state)
{
    ActionMessage obj(CMD_MULTI_MESSAGE);
    obj.name = "sstring";
    for (int ii = 0; ii < 100; ++ii) {
        obj.setString(ii, ActionMessage(CMD_PING_REPLY).to_string());
    }
    std::string load;
    load.reserve(50000);
    obj.packetize(load);
    ActionMessage conv;

    for (auto _ : state) {
        conv.depacketize(load.data(), static_cast<int>(load.size()));
    }
}
// Register the function as a benchmark
BENCHMARK(BM_AM_depacketize_strings);

HELICS_BENCHMARK_MAIN(actionMessageBenchmark);
