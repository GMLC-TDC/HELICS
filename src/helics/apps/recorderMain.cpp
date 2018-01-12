/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/


#include "recorder.h"
#include "../core/core-exceptions.h"
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        helics::recorder recorder(argc, argv);
        if (recorder.isActive())
        {
            recorder.run();
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