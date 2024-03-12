/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../core/core-exceptions.hpp"
#include "Connector.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    int ret = 0;
    try {
        helics::apps::Connector Connector(argc, argv);
        if (Connector.isActive()) {
            Connector.run();
        }
    }
    catch (const std::invalid_argument& ia) {
        std::cerr << ia.what() << std::endl;
        ret = -2;
    }
    catch (const helics::HelicsException& he) {
        std::cerr << he.what() << std::endl;
        ret = -4;
    }

    helics::cleanupHelicsLibrary();
    return ret;
}
