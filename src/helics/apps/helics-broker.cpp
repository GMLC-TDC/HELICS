/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "BrokerApp.hpp"
#include "../core/core-exceptions.hpp"
#include "../application_api/Federate.hpp"
#include <iostream>

int main (int argc, char *argv[])
{
    int ret = 0;
    try
    {
        helics::apps::BrokerApp broker(argc, argv);
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << ia.what() << std::endl;
        ret = -2;
    }
    catch (const helics::HelicsException &he)
    {
        std::cerr << he.what() << std::endl;
        ret = -4;
    }

    helics::cleanupHelicsLibrary();
    return ret;
}
