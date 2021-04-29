/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/ActionMessage.hpp"
#include "helics_benchmark_main.h"

using namespace helics;  // NOLINT

static void BMtoString(benchmark::State& state)
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
BENCHMARK(BMtoString);

static void BMfromString(benchmark::State& state)
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
BENCHMARK(BMfromString);

static void BMtoStringTime(benchmark::State& state)
{
    ActionMessage obj(CMD_TIME_REQUEST);
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        obj.to_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BMtoStringTime);

static void BMfromStringTime(benchmark::State& state)
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
BENCHMARK(BMfromStringTime);

static void BMpacketize(benchmark::State& state)
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
BENCHMARK(BMpacketize);

static void BMdepacketize(benchmark::State& state)
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
BENCHMARK(BMdepacketize);

static void BMpacketizeStrings(benchmark::State& state)
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
BENCHMARK(BMpacketizeStrings);

static void BMdepacketizeStrings(benchmark::State& state)
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
BENCHMARK(BMdepacketizeStrings);

HELICS_BENCHMARK_MAIN(actionMessageBenchmark);
