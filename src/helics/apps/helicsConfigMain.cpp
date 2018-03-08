/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <iostream>
#include "helicsConfigMain.h"

const char kPathSeparator =
#ifdef _WIN32
                            '\\';
#else
                            '/';
#endif

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " --prefix|--includes|--libs|--help" << std::endl;
}

int main (int argc, char *argv[])
{

    if (argc != 2) {
        show_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ( (arg == "-h") || (arg == "--help") ) {
            show_usage(argv[0]);
        } else if ( arg == "--prefix" ) {
            std::cout << HELICS_INSTALL_PREFIX << std::endl;
        } else if ( arg == "--includes" ) {
            std::cout << HELICS_INSTALL_PREFIX << kPathSeparator << "include" << std::endl;
        } else if ( arg == "--libs" ) {
            std::cout << HELICS_INSTALL_PREFIX << kPathSeparator << "lib" << std::endl;
        } else {
            std::cerr << "Received unknown argument: " << arg << std::endl;
            show_usage(argv[0]);
        }
    }

    return 0;

}
