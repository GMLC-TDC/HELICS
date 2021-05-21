/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Broker.hpp"
#include "core-types.hpp"

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
        virtual std::shared_ptr<Broker> build(const std::string& name) = 0;
    };

    /** template for making a Broker builder*/
    template<class BrokerTYPE>
    class BrokerTypeBuilder final: public BrokerBuilder {
      public:
        static_assert(std::is_base_of<Broker, BrokerTYPE>::value,
                      "Type does not inherit from helics::Core");

        using broker_build_type = BrokerTYPE;
        virtual std::shared_ptr<Broker> build(const std::string& name) override
        {
            return std::make_shared<BrokerTYPE>(name);
        }
    };

    /** define a new Broker Builder from the builder give a name and build code*/
    void defineBrokerBuilder(std::shared_ptr<BrokerBuilder> cb, const std::string& name, int code);

    /** template function to create a builder and link it into the library*/
    template<class BrokerTYPE>
    std::shared_ptr<BrokerBuilder> addBrokerType(const std::string& brokerTypeName, int code)
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
    std::shared_ptr<Broker> create(core_type type, const std::string& configureString);
    /** Create a broker from command line arguments*/
    std::shared_ptr<Broker> create(core_type type, int argc, char* argv[]);
    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker> create(core_type type, std::vector<std::string> args);

    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, const std::string& configureString);

    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, int argc, char* argv[]);

    /** Create a broker from command line arguments in a vector*/
    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, std::vector<std::string> args);

    /** locate a coreBroker by name
@param brokerName the name of the broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> findBroker(const std::string& brokerName);
    /** get the first available Connected broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> getConnectedBroker();
    /**  get a broker by index (0 based)
@param index, the index counter value of the broker
@return a shared_ptr to the Broker*/
    std::shared_ptr<Broker> getBrokerByIndex(size_t index);

    /** try to find a joinable broker of a specific type*/
    std::shared_ptr<Broker> findJoinableBrokerOfType(core_type type);

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
    bool registerBroker(const std::shared_ptr<Broker>& broker, core_type type);
    /** remove a broker from the registry
@param name the name of the broker to unregister
*/
    void unregisterBroker(const std::string& name);

    /** add a type associated with a broker*/
    void addAssociatedBrokerType(const std::string& name, core_type type);

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
    bool copyBrokerIdentifier(const std::string& copyFromName, const std::string& copyToName);

    /** display the help listing for a particular core_type*/
    void displayHelp(core_type type = core_type::UNRECOGNIZED);

    /** terminate all running Brokers*/
    void terminateAllBrokers();
    /** abort all brokers */
    void abortAllBrokers(int errorCode, const std::string& errorString);
}  // namespace BrokerFactory
}  // namespace helics
