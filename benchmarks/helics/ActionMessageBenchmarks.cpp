/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/core/ActionMessage.hpp"
#include <benchmark/benchmark.h>

using namespace helics;

static void BM_AM_toString (benchmark::State &state)
{
    ActionMessage obj (CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData ("this is a new string to add to the string data");
    std::string load;
    load.reserve (500);
    for (auto _ : state)
    {
        obj.to_string (load);
    }
}
// Register the function as a benchmark
BENCHMARK (BM_AM_toString);

static void BM_AM_FromString (benchmark::State &state)
{
    ActionMessage obj (CMD_REG_FED);
    obj.name = "the name of the federate is really long";
    obj.setStringData ("this is a new string to add to the string data");
    std::string load;
    load.reserve (500);
    obj.to_string (load);
    ActionMessage conv;

    for (auto _ : state)
    {
        conv.from_string (load);
    }
}
// Register the function as a benchmark
BENCHMARK (BM_AM_FromString);
