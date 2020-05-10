/*
Copyright (c) 2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/cpp98/helics.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    volatile helicscpp::FederateInfo fi;
    std::cout << helicsGetVersion() << std::endl;
    return 0;
}
