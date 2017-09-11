#include "helics/config.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>

#define USE_LOGGING 1
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream()
#define ENDL std::endl
#endif

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;


void argumentParser(int argc, char *argv[], po::variables_map &vm_map);

int main(int argc, char **argv)
{

	po::variables_map vm;
	argumentParser(argc, argv, vm);

    return 0;
}

void argumentParser(int argc, char *argv[], po::variables_map &vm_map)
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
		("broker,b", po::value<std::string>(), "address to connect of the higer broker to if not specfied assumed to be root")
		("name,n", po::value<std::string>(), "name of the broker")
		("core,c", po::value<std::string>(), "type of the broker")
		("stop", po::value<double>(), "the time to stop the simulation")
		("federates",po::value<int32_t>(),"the minimum number of federates to connect before initialization")
		("brokers_min",po::value<int32_t>(),"the minimum number of brokers to connect before initialization")
		("connection",po::value<std::string>(),"the port/address to list for incoming connections")
		("connection2",po::value<std::string>(),"the port for the second connection for a gateway configuration")
		("coreinit,i", po::value<std::string>(), "the broker initializion string");

	// clang-format on

	po::options_description cmd_line("command line options");
	po::options_description config_file("configuration file options");
	po::options_description visible("allowed options");

	cmd_line.add(cmd_only).add(config);
	config_file.add(config);
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
		std::cout << 0.1 << '\n';
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
	// check to make sure we have some input file
	if (vm_map.count("input") == 0)
	{
		std::cout << " no input file specified\n";
		std::cout << visible << '\n';
		return;
	}
}