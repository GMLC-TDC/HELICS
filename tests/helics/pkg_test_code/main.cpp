/*
Copyright (c) 2019-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/helics.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    volatile helics::FederateInfo fi;
    helics::BrokerApp("--version");
    return 0;
}
