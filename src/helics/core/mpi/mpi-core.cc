/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"

#if HELICS_HAVE_MPI

#include "helics/core/mpi/mpi-core.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#define OMPI_SKIP_MPICXX
#include "mpi.h"

#include <cassert>
#include <fstream>

namespace helics
{
MpiCore::MpiCore ()=default


static void argumentParser(int argc, char *argv[], boost::program_options::variables_map &vm_map)
{
	namespace po = boost::program_options;
	po::options_description cmd_only("command line only");
	po::options_description config("configuration");
	po::options_description hidden("hidden");

	// clang-format off
	// input boost controls
	cmd_only.add_options()
		("help,h", "produce help message")
		("config-file", po::value<std::string>(), "specify a configuration file to use");


	config.add_options()
		("broker,b", po::value<std::string>(), "identifier for the broker")
		("brokerinit", po::value<int>(), "the initialization string for the broker")
		("register", "register the core for global locating");


	// clang-format on

	po::options_description cmd_line("command line options");
	po::options_description config_file("configuration file options");
	po::options_description visible("allowed options");

	cmd_line.add(cmd_only).add(config);
	config_file.add(config);
	visible.add(cmd_only).add(config);

	//po::positional_options_description p;
	//p.add("input", -1);

	po::variables_map cmd_vm;
	try
	{
		po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), cmd_vm);
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

	po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), vm_map);

	if (cmd_vm.count("config-file") > 0)
	{
		std::string config_file_name = cmd_vm["config-file"].as<std::string>();
		if (!boost::filesystem::exists(config_file_name))
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


void mpiCore::initializeFromArgs(int argc, char *argv[])
{
	namespace po = boost::program_options;
	if (!_initialized)
	{
		po::variables_map vm;
		argumentParser(argc, argv, vm);

		if (vm.count("broker") > 0)
		{
			auto brstring = vm["broker"].as<std::string>();
			//tbroker = findTestBroker(brstring);
		}

		if (vm.count("brokerinit") > 0)
		{
			//tbroker->Initialize(vm["brokerinit"].as<std::string>());
		}
		CommonCore::initializeFromArgs(argc, argv);
	}
}

void MpiCore::terminate()
{
}

void MpiCore::transmit(int route_id, const ActionMessage & cmd)
{
}

void MpiCore::addRoute(int route_id, const std::string & routeInfo)
{
}

/*
bool MpiCore::isInitialized () { return false; }

void MpiCore::error (Core::federate_id_t federateId, int errorCode){}

void MpiCore::finalize (Core::federate_id_t federateId){}

void MpiCore::enterInitializingState(Core::federate_id_t federateID) { }

bool MpiCore::enterExecutingState (Core::federate_id_t federateID, bool iterationCompleted){ return true; }

Core::federate_id_t MpiCore::registerFederate (const char *name, const CoreFederateInfo &info)
{
    return static_cast<Core::federate_id_t> (0);
}

const char *MpiCore::getFederateName (Core::federate_id_t federateId) { return ""; }

Core::federate_id_t MpiCore::getFederateId (const char *name) { return static_cast<Core::federate_id_t> (0); }

void MpiCore::setFederationSize (unsigned int size){}

unsigned int MpiCore::getFederationSize () { return 0; }

Time MpiCore::timeRequest (Core::federate_id_t federateId, Time next) { return 0; }

std::pair<Time, bool>
MpiCore::requestTimeIterative (Core::federate_id_t federateId, Time next, bool localConverged)
{
    return std::make_pair (0, true);
}

uint64_t MpiCore::getCurrentReiteration (Core::federate_id_t federateId) { return 0; }

void MpiCore::setMaximumIterations (federate_id_t federateId, uint64_t iterations){}

void MpiCore::setTimeDelta (Core::federate_id_t federateId, Time time){}

void MpiCore::setLookAhead (Core::federate_id_t federateId, Time time){}

void MpiCore::setImpactWindow(federate_id_t federateID, Time ImpactTime) {}

Core::Handle MpiCore::registerSubscription (Core::federate_id_t federateId,
                                            const char *key,
                                            const char *type,
                                            const char *units,
                                            bool required)
{
    return Core::Handle ();
}

Core::Handle MpiCore::getSubscription (Core::federate_id_t federateId, const char *key)
{
    return Core::Handle ();
}

Core::Handle
MpiCore::registerPublication (Core::federate_id_t federateId, const char *key, const char *type, const char *units)
{
    return Core::Handle ();
}

Core::Handle MpiCore::getPublication (Core::federate_id_t federateId, const char *key) { return Core::Handle (); }

const char *MpiCore::getUnits (Core::Handle handle) { return ""; }

const char *MpiCore::getType (Core::Handle handle) { return ""; }

void MpiCore::setValue (Core::Handle handle, const char *data, uint64_t len){}

data_t *MpiCore::getValue (Core::Handle handle) { return nullptr; }

void MpiCore::dereference (data_t *data){}
void MpiCore::dereference (message_t *msg){}

const Core::Handle *MpiCore::getValueUpdates (Core::federate_id_t federateId, uint64_t *size) { return nullptr; }

Core::Handle MpiCore::registerEndpoint (Core::federate_id_t federateId, const char *name, const char *type)
{
    return Core::Handle ();
}

Core::Handle MpiCore::registerSourceFilter (Core::federate_id_t federateId,
                                            const char *filterName,
                                            const char *source,
                                            const char *type_in)
{
    return Core::Handle ();
}
Core::Handle MpiCore::registerDestinationFilter (Core::federate_id_t federateId,
                                                 const char *filterName,
                                                 const char *dest,
                                                 const char *type_in)
{
    return Core::Handle ();
}
void MpiCore::addDependency(federate_id_t federateId, const char *federateName)
{

}
void MpiCore::registerFrequentCommunicationsPair (const char *source, const char *dest){}

void MpiCore::send (Core::Handle sourceHandle, const char *destination, const char *data, uint64_t len){}

void MpiCore::sendEvent (Time time, Handle source, const char *destination, const char *data, uint64_t len){}

void MpiCore::sendMessage (message_t *message){}

uint64_t MpiCore::receiveCount (Handle destination) { return 0; }

message_t *MpiCore::receive (Core::Handle destination) { return nullptr; }

std::pair<const Core::Handle, message_t*> MpiCore::receiveAny(Core::federate_id_t federateId) { return{ 0xFFFFFFFF,nullptr }; }

uint64_t MpiCore::receiveCountAny (Core::federate_id_t federateId) { return 0; }

void MpiCore::logMessage (Core::federate_id_t federateId, int logCode, const char *logMessage) {}

uint64_t MpiCore::receiveFilterCount(federate_id_t federateID) { return 0; }

std::pair<const Core::Handle, message_t*> MpiCore::receiveAnyFilter(federate_id_t federateID) { return{ 0xFFFFFFFF, nullptr }; }

void MpiCore::setFilterOperator(Handle filter, FilterOperator* callback) {}
*/
}  // namespace helics

#endif
