/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/BrokerApp.hpp"
#include "Broker.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {
/**
 * Factory for building Core API instances.
 */
namespace BrokerFactory {
#define HELICS_SHARED_DEPRECATED                                                                   \
    [[deprecated(                                                                                  \
        "Broker Factory is deprecated for use in the C++ shared library use BrokerApp instead if you really need the functionality either link to the static library and/or contact the developers with the requirements to potentially add it to BrokerApp")]]
    /**
     * Creates a Broker object of the specified type.
     *
     * Invokes initialize() on the instantiated Core object.
     */
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> create(core_type type,
                                                            const std::string& initializationString)
    {
        BrokerApp brk(type, initializationString);
        return brk.getCopyofBrokerPointer();
    }
    /** Create a broker from command line arguments*/
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> create(core_type type, int argc, char* argv[])
    {
        BrokerApp brk(type, argc, argv);
        return brk.getCopyofBrokerPointer();
    }

    /** Create a broker from command line arguments in a vector*/
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> create(core_type type,
                                                            std::vector<std::string> args)
    {
        BrokerApp brk(type, args);
        return brk.getCopyofBrokerPointer();
    }

    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> create(core_type type,
                                                            const std::string& broker_name,
                                                            const std::string& initializationString)
    {
        BrokerApp brk(type, broker_name, initializationString);
        return brk.getCopyofBrokerPointer();
    }

    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, int argc, char* argv[])
    {
        BrokerApp brk(type, broker_name, argc, argv);
        return brk.getCopyofBrokerPointer();
    }

    /** Create a broker from command line arguments in a vector*/
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, std::vector<std::string> args)
    {
        BrokerApp brk(type, broker_name, args);
        return brk.getCopyofBrokerPointer();
    }

    /** locate a coreBroker by name
@param brokerName the name of the broker
@return a shared_ptr to the Broker*/
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> findBroker(const std::string& brokerName)
    {
        return nullptr;
    }

    /** try to find a joinable broker of a specific type*/
    HELICS_SHARED_DEPRECATED std::shared_ptr<Broker> findJoinableBrokerOfType(core_type type)
    {
        return nullptr;
    }

    /** get all available brokers*/
    HELICS_SHARED_DEPRECATED std::vector<std::shared_ptr<Broker>> getAllBrokers() { return {}; }

    /** check if there are any active Brokers*/
    HELICS_SHARED_DEPRECATED bool brokersActive() { return false; }

    /** register a coreBroker so it can be found by others
@details also cleans up any leftover brokers that were previously unregistered this can be
controlled by calling cleanUpBrokers earlier if desired
@param broker a pointer to a Broker object that should be able to be found globally
@return true if the registration was successful false otherwise*/
    HELICS_SHARED_DEPRECATED bool registerBroker(const std::shared_ptr<Broker>& broker)
    {
        return false;
    }
    /** remove a broker from the registry
@param name the name of the broker to unregister
*/
    HELICS_SHARED_DEPRECATED void unregisterBroker(const std::string& name) {}
    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a
new broker is registered or when the clean up function is called this prevents some odd threading
issues
@return the number of brokers still operating
*/
    HELICS_SHARED_DEPRECATED size_t cleanUpBrokers() { return 0; }

    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a
new broker is registered or when the clean up function is called this prevents some odd threading
issues
@param delay the number of milliseconds to wait to ensure stuff is cleaned up
@return the number of brokers still operating
*/
    HELICS_SHARED_DEPRECATED size_t cleanUpBrokers(std::chrono::milliseconds delay) { return 0; }

    /** make a copy of the broker pointer to allow access to the new name
@return true if successful
 */
    HELICS_SHARED_DEPRECATED bool copyBrokerIdentifier(const std::string& copyFromName,
                                                       const std::string& copyToName)
    {
        return false;
    }

    /** display the help listing for a particular core_type*/
    HELICS_SHARED_DEPRECATED void displayHelp(core_type type = core_type::UNRECOGNIZED) {}
}  // namespace BrokerFactory
}  // namespace helics
