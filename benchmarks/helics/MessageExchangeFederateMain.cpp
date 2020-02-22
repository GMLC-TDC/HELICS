/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MessageExchangeFederate.hpp"

#include <chrono>
#include <iostream>

int main(int argc, char* argv[])
{
    helics::FederateInfo fi;
    MessageExchangeFederate fed;

    int rc = fed.initialize(fi, argc, argv);
    if (rc != 0) {
        exit(rc);
    }

    // setup benchmark timing
    std::chrono::time_point<std::chrono::steady_clock> start_time, end_time;
    fed.setBeforeFinalizeCallback([&end_time]() { end_time = std::chrono::steady_clock::now(); });

    // run the benchmark
    fed.run([&start_time]() { start_time = std::chrono::steady_clock::now(); });

    // print duration
    auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
    fed.addResult<decltype(elapsed)>("ELAPSED TIME (ns)", "real_time", elapsed);
    fed.printResults();
}
