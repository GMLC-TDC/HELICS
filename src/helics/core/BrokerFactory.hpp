/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Broker.hpp"
#include "CoreTypes.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace helics {
/**
 * Factory for building Core API instances.
 */
namespace BrokerFactory {

    class BrokerBuilder {
      public:
        /** build a new broker of the builder type*/
        virtual std::shared_ptr<Broker> build(std::string_view name) = 0;
    };

    /** template for making a Broker builder*/
    template<class BrokerTYPE>
    class BrokerTypeBuilder final: public BrokerBuilder {
      public:
        static_assert(std::is_base_of<Broker, BrokerTYPE>::value,
                      "Type does not inherit from helics::Broker");

        using broker_build_type = BrokerTYPE;
        virtual std::shared_ptr<Broker> build(std::string_view name) override
        {
            return std::make_shared<BrokerTYPE>(name);
        }
    };

    /** define a new Broker Builder from the builder give a name and build code*/
    void defineBrokerBuilder(std::shared_ptr<BrokerBuilder> builder,
                             std::string_view name,
                             int code);

    /** template function to create a builder and link it into the library*/
    template<class BrokerTYPE>
    std::shared_ptr<BrokerBuilder> addBrokerType(std::string_view brokerTypeName, int code)
    {
        auto bld = std::make_shared<BrokerTypeBuilder<BrokerTYPE>>();
        std::shared_ptr<BrokerBuilder> bbld = std::static_pointer_cast<BrokerBuilder>(bld);
        defineBrokerBuilder(bbld, brokerTypeName, code);
        return bbld;
    }

    /**
     * Creates a Broker object of the specified type.
     *
     * Invokes initialize() on the instantiated Core object.
     */
    std::shared_ptr<Broker> create(CoreType type, std::string_view configureString);
    /** Create a broker from command line arguments*/
    std::shared_ptr<Broker> create(CoreType type, int argc, char* argv[]);
    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker> create(CoreType type, std::vector<std::string> args);

    std::shared_ptr<Broker>
        create(CoreType type, std::string_view brokerName, std::string_view configureString);

    std::shared_ptr<Broker>
        create(CoreType type, std::string_view brokerName, int argc, char* argv[]);

    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker>
        create(CoreType type, std::string_view brokerName, std::vector<std::string> args);

    /** locate a coreBroker by name
@param brokerName the name of the broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> findBroker(std::string_view brokerName);
    /** get the first available Connected broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> getConnectedBroker();
    /**  get a broker by index (0 based)
@param index the index counter value of the broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> getBrokerByIndex(size_t index);

    /** try to find a joinable broker of a specific type*/
    std::shared_ptr<Broker> findJoinableBrokerOfType(CoreType type);

    /** get all available brokers*/
    std::vector<std::shared_ptr<Broker>> getAllBrokers();

    /** check if there are any active Brokers*/
    bool brokersActive();

    /** register a coreBroker so it can be found by others
@details also cleans up any leftover brokers that were previously unregistered this can be
controlled by calling cleanUpBrokers earlier if desired
@param broker a pointer to a Broker object that should be able to be found globally
@param type the core type associated with a broker
@return true if the registration was successful false otherwise*/
    bool registerBroker(const std::shared_ptr<Broker>& broker, CoreType type);
    /** remove a broker from the registry
@param name the name of the broker to unregister
*/
    void unregisterBroker(std::string_view name);

    /** add a type associated with a broker*/
    void addAssociatedBrokerType(std::string_view name, CoreType type);

    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a
new broker is registered or when the clean up function is called this prevents some odd threading
issues
@return the number of brokers still operating
*/
    size_t cleanUpBrokers();

    /** clean up unused brokers
@details when brokers are unregistered they get put in a holding area that gets cleaned up when a
new broker is registered or when the clean up function is called this prevents some odd threading
issues
@param delay the number of milliseconds to wait to ensure stuff is cleaned up
@return the number of brokers still operating
*/
    size_t cleanUpBrokers(std::chrono::milliseconds delay);

    /** make a copy of the broker pointer to allow access to the new name
@return true if successful
 */
    bool copyBrokerIdentifier(std::string_view copyFromName, std::string_view copyToName);

    /** display the help listing for a particular CoreType*/
    void displayHelp(CoreType type = CoreType::UNRECOGNIZED);

    /** terminate all running Brokers*/
    void terminateAllBrokers();
    /** abort all brokers */
    void abortAllBrokers(int errorCode, std::string_view errorString);
}  // namespace BrokerFactory
}  // namespace helics
