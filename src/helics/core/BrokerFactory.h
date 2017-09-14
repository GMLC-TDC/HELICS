/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_BROKER_FACTORY_
#define _HELICS_BROKER_FACTORY_
#pragma once

#include "helics/core/core-types.h"
#include <memory>
#include <string>

namespace helics {

class CoreBroker;

/**
* Factory for building Core API instances.
*/
class BrokerFactory {
public:
	/**
	* Creates a Broker object of the specified type.
	*
	* Invokes initialize() on the instantiated Core object.
	*/
	static std::shared_ptr<CoreBroker> create(helics_core_type type, const std::string &initializationString);

	static std::shared_ptr<CoreBroker> create(helics_core_type type, int argc, char *argv[]);

	static std::shared_ptr<CoreBroker> create(helics_core_type type, const std::string &broker_name, std::string &initializationString);
	static std::shared_ptr<CoreBroker> create(helics_core_type type, const std::string &broker_name, int argc, char *argv[]);

	/**
	* Returns true if type specified is available in current compilation.
	*/
	static bool available(helics_core_type type);
};

/** locate a coreBroker by name
@param name the name of the broker
@return a shared_ptr to the Broker*/
std::shared_ptr<CoreBroker> findBroker(const std::string &brokerName);

/** register a coreBroker so it can be found by others
@details also cleans up any leftover brokers that were previously unregistered this can be controlled by calling cleanUpBrokers
earlier if desired
@param tbroker a pointer to a CoreBroker object that should be able to be found globally
@return true if the registration was successful false otherwise*/
bool registerBroker(std::shared_ptr<CoreBroker> tbroker);
/** remove a broker from the registry
@param name the name of the broker to unregister
*/
void unregisterBroker(const std::string &name);
/** clean up unused brokers
@details when brokers are unregisterd they get put in a holding area that gets cleaned up when a new broker is registered
or when the clean up function is called this prevents some odd threading issues
*/
void cleanUpBrokers();

/** make a copy of the broker pointer to allow access to the new name
*/
void copyBrokerIdentifier(const std::string &copyFromName, const std::string &copyToName);

} // namespace helics

#endif  //_HELICS_BROKER_FACTORY_

