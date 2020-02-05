/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PholdFederate.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics_benchmark_util.h"

#include <chrono>
#include <iostream>
#include <random>

int main(int argc, char* argv[])
{
    int index = 0;
    int max_index = 0;
    unsigned int init_ev_count = 16; // starting number of events
    double local_prob = .9; // probability of local events
    double rand_time_mean =
        .9; // * deltaTime, mean for exponential distribution used when picking event times
    bool gen_rand_seed;
    unsigned int rand_seed;

    helics::helicsCLI11App app("phold benchmark federate");

    auto opt_index = app.add_option("--index", index, "the index of this phold federate");
    auto opt_max_index =
        app.add_option("--max_index", max_index, "the maximum index given to a phold federate");
    opt_index->required();
    opt_max_index->required();
    opt_max_index->ignore_underscore();

    auto opt_init_ev_count =
        app.add_option("--init_ev_count", init_ev_count, "the starting number of events");
    auto opt_local_prob =
        app.add_option("--local_probability", local_prob, "the probability of local events");
    auto opt_rand_time_mean = app.add_option(
        "--rand_time_mean",
        rand_time_mean,
        "mean for the exponential distribution used when picking event times");
    auto opt_gen_rand_seed =
        app.add_flag("--gen_rand_seed", gen_rand_seed, "enable generating a random seed");
    auto opt_set_rand_seed = app.add_option("--set_rand_seed", rand_seed, "set the random seed");

    app.allow_extras();

    auto res = app.helics_parse(argc, argv);
    helics::FederateInfo fi;

    if (res != helics::helicsCLI11App::parse_output::ok) {
        switch (res) {
            case helics::helicsCLI11App::parse_output::help_call:
            case helics::helicsCLI11App::parse_output::help_all_call:
                fi.loadInfoFromArgs("--help");
                // FALLTHRU
            case helics::helicsCLI11App::parse_output::version_call:
                return 0;
            default:
                return static_cast<int>(res);
        }
    }

    fi.loadInfoFromArgs(app.remainArgs());

    PholdFederate fed;

    if (*opt_init_ev_count) {
        fed.setInitialEventCount(init_ev_count);
    }

    if (*opt_local_prob) {
        fed.setLocalProbability(local_prob);
    }

    if (*opt_rand_time_mean) {
        fed.setRandomTimeMean(rand_time_mean);
    }

    if (*opt_gen_rand_seed) {
        fed.setGenerateRandomSeed(gen_rand_seed);
    } else {
        fed.setGenerateRandomSeed(false);
    }

    if (*opt_set_rand_seed) {
        fed.setRandomSeed(rand_seed);
    } else {
        std::mt19937 rand_gen(0x600d5eed);
        std::uniform_int_distribution<unsigned int> rand_seed_uniform;
        for (int i = 0; i < index; i++) {
            rand_seed_uniform(rand_gen);
        }
        fed.setRandomSeed(rand_seed_uniform(rand_gen));
    }

    printHELICSsystemInfo();

    fed.initialize(fi, index, max_index);

    // setup benchmark timing
    std::chrono::time_point<std::chrono::steady_clock> start_time, end_time;
    fed.setBeforeFinalizeCallback([&end_time]() { end_time = std::chrono::steady_clock::now(); });

    // run the benchmark
    fed.run([&start_time]() { start_time = std::chrono::steady_clock::now(); });

    // print duration
    auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    std::cout << "ELAPSED TIME (ns): " << elapsed << std::endl;
    std::cout << "EVENT COUNT: " << fed.evCount << std::endl;
}
