/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics-time.hpp"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace helics {
/** virtual class defining a public interface to a broker*/
class Broker {
  public:
    /**default constructor*/
    Broker() = default;
    /** destructor*/
    virtual ~Broker() = default;

    /** connect the core to its broker
    @details should be done after initialization has complete*/
    virtual bool connect() = 0;
    /** disconnect the broker from any other brokers and communications
     */
    virtual void disconnect() = 0;

    /** check if the broker is connected*/
    virtual bool isConnected() const = 0;
    /** set the broker to be a root broker
    @details only valid before the initialization function is called*/
    virtual void setAsRoot() = 0;
    /** return true if the broker is a root broker
     */
    virtual bool isRoot() const = 0;

    /** check if the broker is ready to accept new federates or cores
     */
    virtual bool isOpenToNewFederates() const = 0;
    /** start up the broker with an initialization string containing commands and parameters*/
    virtual void configure(const std::string& configureString) = 0;
    /** initialize from command line arguments
     */
    virtual void configureFromArgs(int argc, char* argv[]) = 0;
    /** start up the broker with an initialization string containing commands and parameters*/
    [[deprecated("please use configure instead")]] void
        initialize(const std::string& configureString)
    {
        configure(configureString);
    }
    /** initialize from command line arguments
     */
    [[deprecated("please use configureFromArgs instead")]] void initializeFromArgs(int argc,
                                                                                   char* argv[])
    {
        configureFromArgs(argc, argv);
    }
    /** Initialize the Broker from command line arguments contained in a vector
     * Should be invoked a single time to initialize the co-simulation broker.
     */
    virtual void configureFromVector(std::vector<std::string> args) = 0;
    /** get the local identification for the broker*/
    virtual const std::string& getIdentifier() const = 0;
    /** get the connection address for the broker*/
    virtual const std::string& getAddress() const = 0;
    /** set the broker logging level*/
    virtual void setLoggingLevel(int logLevel) = 0;
    /** set the logging callback function
    @param logFunction a function with a signature of void(int level,  const std::string &source,
    const std::string &message) the function takes a level indicating the logging level string with
    the source name and a string with the message
    */
    virtual void setLoggingCallback(
        const std::function<void(int, const std::string&, const std::string&)>& logFunction) = 0;

    /** set the broker logging file*/
    virtual void setLogFile(const std::string& lfile) = 0;

    /** waits in the current thread until the broker is disconnected
    @param msToWait  the timeout to wait for disconnect
    @return true if the disconnect was successful false if it timed out
     */
    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const = 0;
    /** make a query for information from the co-simulation
    @details the format is somewhat unspecified  target is the name of an object typically one of
    "federation",  "broker", or the name of a specific object
    query is a broken
    @param target the specific target of the query
    @param queryStr the actual query
    @param mode fast (asynchronous; default) means the query goes on priority channels, ordered
    (synchronous) is slower but has more ordering guarantees
      @return a string containing the response to the query.  Query is a blocking call and will not
    return until the query is answered so use with caution
    */
    virtual std::string query(const std::string& target,
                              const std::string& queryStr,
                              helics_sequencing_mode mode = helics_sequencing_mode_fast) = 0;

    /** set a federation global value
    @details this overwrites any previous value for this name
    globals can be queried with a target of "global" and queryStr of the value to Query
    @param valueName the name of the global to set
    @param value the value of the global
    */
    virtual void setGlobal(const std::string& valueName, const std::string& value) = 0;

    /** load a file containing connection information
    @param file a JSON or TOML file containing connection information*/
    virtual void makeConnections(const std::string& file) = 0;
    /** create a data Link between a named publication and a named input
    @param source the name of the publication
    @param target the name of the input*/
    virtual void dataLink(const std::string& source, const std::string& target) = 0;
    /** create a filter connection between a named filter and a named endpoint for messages coming
    from that endpoint
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addSourceFilterToEndpoint(const std::string& filter,
                                           const std::string& target) = 0;
    /** create a filter connection between a named filter and a named endpoint for destination
    processing
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& target) = 0;

    /** update a time barrier with a new time*/
    virtual void setTimeBarrier(Time barrierTime) = 0;

    /** update a time barrier with a new time*/
    virtual void clearTimeBarrier() = 0;

    /**
    * generate a global error and halt the federation
    @param errorCode the code to use for the error
    @param errorString the error message to associate with the error
    */
    virtual void globalError(int32_t errorCode, const std::string& errorString) = 0;
};
}  // namespace helics
