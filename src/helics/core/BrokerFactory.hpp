/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Broker.hpp"
#include "core-types.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

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
    std::shared_ptr<Broker> create(core_type type, const std::string& initializationString);
    /** Create a broker from command line arguments*/
    std::shared_ptr<Broker> create(core_type type, int argc, char* argv[]);
    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker> create(core_type type, std::vector<std::string> args);

    std::shared_ptr<Broker> create(
        core_type type,
        const std::string& broker_name,
        const std::string& initializationString);

    std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, int argc, char* argv[]);

    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, std::vector<std::string> args);

    /** locate a coreBroker by name
@param brokerName the name of the broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> findBroker(const std::string& brokerName);

    /** try to find a joinable broker of a specific type*/
    std::shared_ptr<Broker> findJoinableBrokerOfType(core_type type);

    /** get all available brokers*/
    std::vector<std::shared_ptr<Broker>> getAllBrokers();

    /** check if there are any active Brokers*/
    bool brokersActive();

    /** register a coreBroker so it can be found by others
@details also cleans up any leftover brokers that were previously unregistered this can be controlled by calling
cleanUpBrokers earlier if desired
@param broker a pointer to a Broker object that should be able to be found globally
@return true if the registration was successful false otherwise*/
    bool registerBroker(const std::shared_ptr<Broker>& broker);
    /** remove a broker from the registry
@param name the name of the broker to unregister
*/
    void unregisterBroker(const std::string& name);
    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a new broker is
registered or when the clean up function is called this prevents some odd threading issues
@return the number of brokers still operating
*/
    size_t cleanUpBrokers();

    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a new broker is
registered or when the clean up function is called this prevents some odd threading issues
@param delay the number of milliseconds to wait to ensure stuff is cleaned up
@return the number of brokers still operating
*/
    size_t cleanUpBrokers(std::chrono::milliseconds delay);

    /** make a copy of the broker pointer to allow access to the new name
@return true if successful
 */
    bool copyBrokerIdentifier(const std::string& copyFromName, const std::string& copyToName);

    /** display the help listing for a particular core_type*/
    void displayHelp(core_type type = core_type::UNRECOGNIZED);
} // namespace BrokerFactory
} // namespace helics
