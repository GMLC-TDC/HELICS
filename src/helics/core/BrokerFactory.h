/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_BROKER_FACTORY_
#define _HELICS_BROKER_FACTORY_
#pragma once

#include "core-types.h"
#include "broker.h"
#include <memory>
#include <string>

namespace helics {

/**
* Factory for building Core API instances.
*/
namespace BrokerFactory {
	/**
	* Creates a Broker object of the specified type.
	*
	* Invokes initialize() on the instantiated Core object.
	*/
	std::shared_ptr<Broker> create(core_type type, const std::string &initializationString);

	std::shared_ptr<Broker> create(core_type type, int argc, const char * const *argv);

	std::shared_ptr<Broker> create(core_type type, const std::string &broker_name, const std::string &initializationString);
	std::shared_ptr<Broker> create(core_type type, const std::string &broker_name, int argc, const char * const *argv);

	/**
	* Returns true if type specified is available in current compilation.
	*/
	bool available(core_type type);


/** locate a coreBroker by name
@param name the name of the broker
@return a shared_ptr to the Broker*/
std::shared_ptr<Broker> findBroker(const std::string &brokerName);

/** register a coreBroker so it can be found by others
@details also cleans up any leftover brokers that were previously unregistered this can be controlled by calling cleanUpBrokers
earlier if desired
@param broker a pointer to a Broker object that should be able to be found globally
@return true if the registration was successful false otherwise*/
bool registerBroker(std::shared_ptr<Broker> broker);
/** remove a broker from the registry
@param name the name of the broker to unregister
*/
void unregisterBroker(const std::string &name);
/** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a new broker is registered
or when the clean up function is called this prevents some odd threading issues
@return the number of brokers still operating
*/
size_t cleanUpBrokers();

/** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a new broker is registered
or when the clean up function is called this prevents some odd threading issues
@param delay the number of milliseconds to wait to ensure stuff is cleaned up
@return the number of brokers still operating
*/
size_t cleanUpBrokers( int delay);

/** make a copy of the broker pointer to allow access to the new name
*/
void copyBrokerIdentifier(const std::string &copyFromName, const std::string &copyToName);

/** display the help listing for a particular core_type*/
void displayHelp(core_type type = core_type::UNRECOGNIZED);
} //namespace BrokerFactory
} // namespace helics

#endif  //_HELICS_BROKER_FACTORY_

