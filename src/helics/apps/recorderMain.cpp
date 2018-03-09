/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/


#include "Recorder.hpp"
#include "../core/core-exceptions.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        helics::apps::Recorder Recorder(argc, argv);
        if (Recorder.isActive())
        {
            Recorder.run();
        }

    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << ia.what() << std::endl;
        helics::cleanupHelicsLibrary();
        return (-2);
    }
    catch (const helics::HelicsException &he)
    {
        std::cerr << he.what() << std::endl;
        helics::cleanupHelicsLibrary();
        return (-4);
    }

    helics::cleanupHelicsLibrary();
    return 0;

}

