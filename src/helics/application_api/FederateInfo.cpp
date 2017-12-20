/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Federate.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;


void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map);

namespace helics
{
    FederateInfo::FederateInfo(int argc, const char * const *argv)
    {
        loadInfoFromArgs(argc, argv);
    }

    void FederateInfo::loadInfoFromArgs(int argc, const char * const *argv)
    {
        po::variables_map vm;
        argumentParser(argc, argv, vm);
        std::string name;
        if (vm.count("name") > 0)
        {
            name = vm["name"].as<std::string>();
        }
        std::string corename;
        if (vm.count("core") > 0)
        {
            coreName = vm["core"].as<std::string>();
        }

        coreType = helics::coreTypeFromString(corename);

        coreInitString = "1";
        if (vm.count("coreinit") > 0)
        {
            coreInitString.push_back(' ');
            coreInitString = vm["coreinit"].as<std::string>();
        }
        if (vm.count("broker") > 0)
        {
            coreInitString += " --broker=";
            coreInitString += vm["broker"].as<std::string>();
        }

        if (vm.count("timedelta") > 0)
        {
            timeDelta = vm["timedelta"].as<double>();
        }

        if (vm.count("period") > 0)
        {
            period = vm["period"].as<double>();
        }

        if (vm.count("offset") > 0)
        {
            offset = vm["offset"].as<double>();
        }
    }

} //namespace helics

    void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map)
    {
        po::options_description cmd_only("command line only");
        po::options_description config("configuration");
        po::options_description hidden("hidden");

        // clang-format off
        // input boost controls
        cmd_only.add_options()
            ("help,h", "produce help message")
            ("version,v", "helics version number")
            ("config-file", po::value<std::string>(), "specify a configuration file to use");


        config.add_options()
            ("broker,b", po::value<std::string>(), "address of the broker to connect")
            ("name,n", po::value<std::string>(), "name of the player federate")
            ("core,c", po::value<std::string>(), "type of the core to connect to")
            ("stop", po::value<double>(), "the time to stop recording")
            ("offset", po::value<double>(), "the offset of the time steps")
            ("period", po::value<double>(), "the period of the federate")
            ("timedelta", po::value<double>(), "the time delta of the federate")
            ("coreinit,i", po::value<std::string>(), "the core initialization string");


        hidden.add_options() ("input", po::value<std::string>(), "input file");
        // clang-format on

        po::options_description cmd_line("command line options");
        po::options_description config_file("configuration file options");
        po::options_description visible("allowed options");

        cmd_line.add(cmd_only).add(config).add(hidden);
        config_file.add(config).add(hidden);
        visible.add(cmd_only).add(config);

        po::variables_map cmd_vm;
        try
        {
            po::store(po::command_line_parser(argc, argv).options(cmd_line).run(), cmd_vm);
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            throw (e);
        }

        po::notify(cmd_vm);

        // objects/pointers/variables/constants

        // program options control
        if (cmd_vm.count("help") > 0)
        {
            std::cout << visible << '\n';
            return;
        }

        if (cmd_vm.count("version") > 0)
        {
            std::cout << helics::getHelicsVersionString() << '\n';
            return;
        }

        po::store(po::command_line_parser(argc, argv).options(cmd_line).run(), vm_map);

        if (cmd_vm.count("config-file") > 0)
        {
            std::string config_file_name = cmd_vm["config-file"].as<std::string>();
            if (!filesystem::exists(config_file_name))
            {
                std::cerr << "config file " << config_file_name << " does not exist\n";
                throw (std::invalid_argument("unknown config file"));
            }
            else
            {
                std::ifstream fstr(config_file_name.c_str());
                po::store(po::parse_config_file(fstr, config_file), vm_map);
                fstr.close();
            }
        }

        po::notify(vm_map);

    }


