/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/ActionMessage.hpp"
#include "helics_benchmark_main.h"

#include <string>

using namespace helics;  // NOLINT

static ActionMessage generateTestMessage1()
{
    ActionMessage obj(CMD_REG_FED);
    obj.name("the name of the federate is really long");
    obj.setStringData("this is a new string to add to the string data");
    return obj;
}

static const auto testMessage1 = generateTestMessage1();

static void BMtoString(benchmark::State& state)
{
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        testMessage1.to_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BMtoString);

static void BMfromString(benchmark::State& state)
{
    std::string load;
    load.reserve(500);
    testMessage1.to_string(load);
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
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        testMessage1.packetize(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BMpacketize);

static void BMdepacketize(benchmark::State& state)
{
    std::string load;
    load.reserve(500);
    testMessage1.packetize(load);
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
    obj.name("sstring");
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
    obj.name("sstring");
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

// benchmarks with the Json serialization of actionMessage

static void BMtoStringJson(benchmark::State& state)
{
    std::string load;
    load.reserve(500);
    for (auto _ : state) {
        load.assign(testMessage1.to_json_string());
    }
}
// Register the function as a benchmark
BENCHMARK(BMtoStringJson);

static void BMfromStringJson(benchmark::State& state)
{
    std::string load = testMessage1.to_json_string();
    ActionMessage conv;

    for (auto _ : state) {
        conv.from_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BMfromStringJson);

static void BMfromStringJsonDirect(benchmark::State& state)
{
    std::string load = testMessage1.to_json_string();
    ActionMessage conv;

    for (auto _ : state) {
        conv.from_json_string(load);
    }
}

// Register the function as a benchmark
BENCHMARK(BMfromStringJsonDirect);

static void BMtoStringTimeJson(benchmark::State& state)
{
    ActionMessage obj(CMD_TIME_REQUEST);
    std::string load;
    for (auto _ : state) {
        load.assign(obj.to_json_string());
    }
}
// Register the function as a benchmark
BENCHMARK(BMtoStringTimeJson);

static void BMfromStringTimeJson(benchmark::State& state)
{
    ActionMessage obj(CMD_TIME_REQUEST);
    auto load = obj.to_json_string();
    ActionMessage conv;

    for (auto _ : state) {
        conv.from_string(load);
    }
}
// Register the function as a benchmark
BENCHMARK(BMfromStringTimeJson);

static void BMpacketizeJson(benchmark::State& state)
{
    std::string load;
    for (auto _ : state) {
        load.assign(testMessage1.packetize_json());
    }
}
// Register the function as a benchmark
BENCHMARK(BMpacketizeJson);

static void BMdepacketizeJson(benchmark::State& state)
{
    std::string load = testMessage1.packetize_json();
    ActionMessage conv;

    for (auto _ : state) {
        conv.depacketize(load.data(), static_cast<int>(load.size()));
    }
}
// Register the function as a benchmark
BENCHMARK(BMdepacketizeJson);

static void BMpacketizeStringsJson(benchmark::State& state)
{
    ActionMessage obj(CMD_MULTI_MESSAGE);
    obj.name("sstring");
    for (int ii = 0; ii < 100; ++ii) {
        obj.setString(ii, ActionMessage(CMD_PING_REPLY).to_string());
    }
    std::string load;
    for (auto _ : state) {
        load.assign(obj.packetize_json());
    }
}
// Register the function as a benchmark
BENCHMARK(BMpacketizeStringsJson);

static void BMdepacketizeStringsJson(benchmark::State& state)
{
    ActionMessage obj(CMD_MULTI_MESSAGE);
    obj.name("sstring");
    for (int ii = 0; ii < 100; ++ii) {
        obj.setString(ii, ActionMessage(CMD_PING_REPLY).to_string());
    }

    std::string load = obj.packetize_json();
    ActionMessage conv;

    for (auto _ : state) {
        conv.depacketize(load.data(), static_cast<int>(load.size()));
    }
}
// Register the function as a benchmark
BENCHMARK(BMdepacketizeStringsJson);

HELICS_BENCHMARK_MAIN(actionMessageBenchmark);
