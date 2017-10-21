/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/ValueFederate.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

bool argumentParser (int argc, const char * const *argv, po::variables_map &vm_map);


int main (int argc, const char * const *argv)
{
    po::variables_map vm;
    if (argumentParser (argc, argv, vm)) {
        return 0;
    }

    std::string name = "fed";
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }
    std::string corename = "zmq";
    if (vm.count ("core") > 0)
    {
        corename = vm["core"].as<std::string> ();
    }
    helics::FederateInfo fi (name);
    try
    {
        fi.coreType = helics::coreTypeFromString (corename);
    }
    catch (std::invalid_argument &ia)
    {
        std::cerr << "Unrecognized core type\n";
        return (-1);
    }
    fi.coreInitString = "";
	fi.logLevel = 5;
    auto vFed = std::make_unique<helics::ValueFederate> (fi);

    auto id = vFed->registerGlobalPublication ("name", "type");

    std::cout << "entering init State\n";
    vFed->enterInitializationState ();
    std::cout << "entered init State\n";
    vFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    for (int i=1; i<10; ++i) {
        auto newTime = vFed->requestTime (i);
        vFed->publish (id, i);
        std::cout << "processed time " << static_cast<double> (newTime) << "\n";
    }
    vFed->finalize ();

    return 0;
}

bool argumentParser (int argc, const char * const *argv, po::variables_map &vm_map)
{
    po::options_description opt ("options");

    // clang-format off
    // input boost controls
    opt.add_options () 
		("help,h", "produce help message")
		("name,n", po::value<std::string> (),"name of the federate")
		("core,c",po::value<std::string> (),"name of the core to connect to");

    // clang-format on

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (opt).run (), cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
        throw (e);
    }

    po::notify (cmd_vm);

    // objects/pointers/variables/constants

    // program options control
    if (cmd_vm.count ("help") > 0)
    {
        std::cout << opt << '\n';
        return true;
    }

    if (cmd_vm.count ("version") > 0)
    {
		std::cout << helics::getHelicsVersionString () << '\n';
        return true;
    }

    po::store (po::command_line_parser (argc, argv).options (opt).run (), vm_map);

    po::notify (vm_map);

    return false;
}
