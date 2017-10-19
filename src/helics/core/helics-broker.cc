#include "helics/config.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "helics/core/BrokerFactory.h"
#include "helics/core/CoreBroker.h"

#include <iostream>
#include <fstream>



namespace po = boost::program_options;
namespace filesystem = boost::filesystem;


bool argumentParser(int argc, char *argv[], po::variables_map &vm_map);

int main(int argc, char *argv[])
{

	po::variables_map vm;
	bool exit_early = argumentParser(argc, argv, vm);

	if (exit_early)
	{
		return 0;
	}

	std::string name = (vm.count("name") > 0) ? vm["name"].as<std::string>() : "";
	std::string btype = (vm.count("type") > 0) ? vm["type"].as<std::string>() : "zmq";
	
	helics::core_type type;
	try
	{
		type = helics::coreTypeFromString(btype);
	}
	catch (std::invalid_argument &ie)
	{
		std::cerr<< "Unable to generate broker: " << ie.what() << '\n';
		return (-2);
	}
	auto broker=helics::BrokerFactory::create(type, argc, argv);
	if (broker->isConnected())
	{
		do //sleep until the broker finishes
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			
		} while (broker->isConnected());
		
	}
	else
	{
		std::cerr << "Broker is unable to connect\n";
		return (-1);
	}
	while (helics::BrokerFactory::cleanUpBrokers()>0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
    return 0;
}

bool argumentParser(int argc, char *argv[], po::variables_map &vm_map)
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
		("name,n", po::value<std::string>(), "name of the broker")
		("type,t", po::value<std::string>(), "type of the broker");

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
		po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), cmd_vm);
	}
	catch (std::exception &e)
	{
		std::cerr <<"invalid command line"<< e.what() << std::endl;
		throw (e);
	}

	po::notify(cmd_vm);

	// objects/pointers/variables/constants


	// program options control
	if (cmd_vm.count("help") > 0)
	{
		std::cout << visible << '\n';
		return true;
	}

	if (cmd_vm.count("version") > 0)
	{
		std::cout << HELICS_VERSION_MAJOR << '.' << HELICS_VERSION_MINOR << '.' << HELICS_VERSION_PATCH << " (" << HELICS_DATE << ")\n";
		return true;
	}


	po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), vm_map);

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


    return false;
}
