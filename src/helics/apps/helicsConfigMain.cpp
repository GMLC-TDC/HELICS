/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helicsConfigMain.h"
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

static void show_usage (std::string const &name)
{
    std::cout << "Usage: " << name << " --prefix|--includes|--libs|--flags|--bin|--version|--help\n";
    std::cout << "--prefix returns the base install location\n";
    std::cout << "--includes, -I returns the helics include location\n";
    std::cout << "--libs, -L returns the helics library location\n";
    std::cout << "--flags returns the C++ flags used for compilation\n";
    std::cout << "--bin return the location of the binaries\n";
    std::cout << "--version returns the helics version\n";
    std::cout << "--std returns the C++ standard flag used\n";
    std::cout << "--help, -h, -? returns this help display\n";
        
}

path base_path( const char *filename)
{
    path cfile(filename);
    path bin_dir = cfile.parent_path();
    path base_dir = bin_dir.parent_path();
    if (exists(base_dir / HELICS_INCLUDE_SUFFIX))
    {
        return base_dir;
    }
    if (base_dir.has_parent_path())
    {
        if (exists(base_dir.parent_path() / HELICS_INCLUDE_SUFFIX))
        {
            return base_dir.parent_path();
        }
    }
    return path(HELICS_INSTALL_PREFIX);
}
int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        show_usage (argv[0]);
        return 1;
    }

    for (int ii = 1; ii < argc; ++ii)
    {
        std::string arg(argv[ii]);
        if ((arg == "-h") || (arg == "--help") || (arg == "-?"))
        {
            show_usage (argv[0]);
        }
        else if (arg == "--version")
        {
            std::cout << HELICS_VERSION << '\n';
        }
        else if (arg == "--prefix")
        {
            path bpath = base_path(argv[0]);
#if BOOST_VERSION_LEVEL>0
            bpath.lexically_normal();
#endif
            std::cout << bpath.make_preferred() << '\n';
        }
        else if ((arg == "--includes") || (arg == "-I")||(arg=="--include"))
        {
            path bpath = base_path(argv[0]);
            bpath /= HELICS_INCLUDE_SUFFIX;

#if BOOST_VERSION_LEVEL>0
            bpath.lexically_normal();
#endif
            std::cout << bpath.make_preferred() << '\n';
        }
        else if ((arg == "--libs")||(arg=="-L")||(arg=="--lib"))
        {
            path bpath = base_path(argv[0]);
            bpath /= HELICS_LIB_SUFFIX;
#if BOOST_VERSION_LEVEL>0
            bpath.lexically_normal();
#endif
            std::cout << bpath.make_preferred() << '\n';
        }
        else if ((arg == "--bin")||(arg=="--binaries"))
        {
            path bpath = base_path(argv[0]);
            bpath /= HELICS_BIN_SUFFIX;
#if BOOST_VERSION_LEVEL>0
            bpath.lexically_normal();
#endif
            std::cout << bpath.make_preferred() << '\n';
        }
        else if (arg == "--flags")
        {
            std::cout << HELICS_CXX_VERSION << " " << HELICS_CXX_FLAGS << '\n';
        }
        else if (arg == "--std")
        {
            std::cout << HELICS_CXX_VERSION <<'\n';
        }
        else
        {
            std::cerr << "Received unknown argument: " << arg << std::endl;
            show_usage (argv[0]);
        }
    }

    return 0;
}
