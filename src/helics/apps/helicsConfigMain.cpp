/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsConfigMain.h"
#ifdef _MSC_VER
#    pragma warning(push, 0)
#    include "helics/external/filesystem.hpp"
#    pragma warning(pop)
#else
#    include "helics/external/filesystem.hpp"
#endif

#include <iostream>
#include <string>

static void show_usage(std::string const& name)
{
    std::cout << "Usage: " << name
              << " --prefix|--includes|--libs|--flags|--bin|--version|--help\n";
    std::cout << "--prefix returns the base install location\n";
    std::cout << "--includes, -I returns the helics include location\n";
    std::cout << "--libs, -L returns the helics library location\n";
    std::cout << "--flags returns the C++ flags used for compilation\n";
    std::cout << "--install returns the install location\n";
    std::cout << "--bin return the location of the binaries\n";
    std::cout << "--version returns the helics version\n";
    std::cout << "--std returns the C++ standard flag used\n";
    std::cout << "--help, -h, -? returns this help display\n";
}

using namespace ghc::filesystem;  // NOLINT
path dir_path(const char* filename, const char* tail)
{
    path cfile = canonical(absolute(path(filename)));
    path bin_dir = (cfile.has_parent_path()) ? cfile.parent_path() : current_path();
    path base_dir = bin_dir.parent_path();
    path dpath;
    if (is_directory(base_dir / tail)) {
        dpath = base_dir / tail;
    } else if (base_dir.has_parent_path()) {
        if (is_directory(base_dir.parent_path() / tail)) {
            dpath = base_dir.parent_path() / tail;
        } else {
            dpath = path(HELICS_INSTALL_PREFIX) / tail;
        }
    } else {
        dpath = path(HELICS_INSTALL_PREFIX) / tail;
    }
    dpath = dpath.lexically_normal();
    return dpath.make_preferred();
}

path base_path(const char* filename)
{
    path cfile = canonical(absolute(path(filename)));
    path bin_dir = (cfile.has_parent_path()) ? cfile.parent_path() : current_path();
    path base_dir = bin_dir.parent_path();
    path dpath;
    if (is_directory(base_dir / HELICS_INCLUDE_SUFFIX)) {
        dpath = base_dir;
    } else if (base_dir.has_parent_path()) {
        if (is_directory(base_dir.parent_path() / HELICS_INCLUDE_SUFFIX)) {
            dpath = base_dir.parent_path();
        } else {
            dpath = path(HELICS_INSTALL_PREFIX);
        }
    } else {
        dpath = path(HELICS_INSTALL_PREFIX);
    }
    dpath = dpath.lexically_normal();
    return dpath.make_preferred();
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }

    for (int ii = 1; ii < argc; ++ii) {
        std::string arg(argv[ii]);
        if ((arg == "-h") || (arg == "--help") || (arg == "-?")) {
            show_usage(argv[0]);
        } else if (arg == "--version") {
            std::cout << HELICS_VERSION << '\n';
        } else if (arg == "--prefix") {
            path bpath = base_path(argv[0]);
            bpath = bpath.lexically_normal();
            std::cout << bpath.make_preferred() << '\n';
        } else if ((arg == "--includes") || (arg == "-I") || (arg == "--include")) {
            std::cout << dir_path(argv[0], HELICS_INCLUDE_SUFFIX) << '\n';
        } else if ((arg == "--libs") || (arg == "-L") || (arg == "--lib")) {
            std::cout << dir_path(argv[0], HELICS_LIB_SUFFIX) << '\n';
        } else if ((arg == "--bin") || (arg == "--binaries")) {
            std::cout << dir_path(argv[0], HELICS_BIN_SUFFIX) << '\n';
        } else if (arg == "--install") {
            std::cout << base_path(argv[0]) << '\n';
        } else if (arg == "--flags") {
            std::cout << HELICS_CXX_VERSION << " " << HELICS_CXX_FLAGS << '\n';
        } else if ((arg == "--std") || (arg == "--standard")) {
            std::cout << HELICS_CXX_VERSION << '\n';
        } else {
            std::cerr << "Received unknown argument: " << arg << '\n';
            show_usage(argv[0]);
        }
    }

    return 0;
}
