/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "argParser.h"
#include <iostream>
#include <boost/filesystem.hpp>

namespace helics
{
void argumentParser (int argc,
                     char *argv[],
                     boost::program_options::variables_map &vm_map,
                     const argDescriptors &additionalArgs)
{
    namespace po = boost::program_options;
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
	// input boost controls
	cmd_only.add_options()
		("help,h", "produce help message")
		("config-file", po::value<std::string>(), "specify a configuration file to use");


	config.add_options()
		("broker,b", po::value<std::string>(), "identifier for the broker")
		("broker_address",po::value<std::string>(),"location of the broker address")
		("register", "register the core for global locating");

	for (auto &addArg : additionalArgs)
	{
		if (addArg.type_.empty())
		{
			config.add_options()
				(addArg.flag_.c_str(), addArg.desc_.c_str());
		}
		else if (addArg.type_ == "string")
		{
			config.add_options()
				(addArg.flag_.c_str(), po::value <std::string>(), addArg.desc_.c_str());
		}
		else if (addArg.type_ == "int")
		{
			config.add_options()
				(addArg.flag_.c_str(), po::value <int>(), addArg.desc_.c_str());
		}
		else if (addArg.type_ == "double")
		{
			config.add_options()
				(addArg.flag_.c_str(), po::value <double>(), addArg.desc_.c_str());
		}
		else if (addArg.type_ == "vector_string")
		{
			config.add_options()
				(addArg.flag_.c_str(), po::value <std::vector<std::string>>(), addArg.desc_.c_str());
		}
	}
    // clang-format on

    po::options_description cmd_line ("command line options");
    po::options_description config_file ("configuration file options");
    po::options_description visible ("allowed options");

    cmd_line.add (cmd_only).add (config);
    config_file.add (config);
    visible.add (cmd_only).add (config);

    // po::positional_options_description p;
    // p.add("input", -1);

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), cmd_vm);
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
        std::cout << visible << '\n';
        return;
    }

    po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), vm_map);

    if (cmd_vm.count ("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
        if (!boost::filesystem::exists (config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument ("unknown config file"));
        }
        else
        {
            std::ifstream fstr (config_file_name.c_str ());
            po::store (po::parse_config_file (fstr, config_file), vm_map);
            fstr.close ();
        }
    }

    po::notify (vm_map);
}

}  // namespace helics