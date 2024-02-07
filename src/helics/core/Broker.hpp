/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helicsTime.hpp"

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

    /** check if the broker is CONNECTED*/
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
    virtual void configure(std::string_view configureString) = 0;
    /** initialize from command line arguments
     */
    virtual void configureFromArgs(int argc, char* argv[]) = 0;
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
        std::function<void(int, std::string_view, std::string_view)> logFunction) = 0;

    /** set the broker logging file*/
    virtual void setLogFile(std::string_view lfile) = 0;

    /** waits in the current thread until the broker is DISCONNECTED
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
    virtual std::string query(std::string_view target,
                              std::string_view queryStr,
                              HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) = 0;

    /** set a federation global value
    @details this overwrites any previous value for this name
    globals can be queried with a target of "global" or "global_value" and queryStr of the value to
    Query
    @param valueName the name of the global to set
    @param value the value of the global
    */
    virtual void setGlobal(std::string_view valueName, std::string_view value) = 0;

    /** send a command to a specific target
   @details the format is somewhat unspecified; target is the name of an object, typically one of
   "federation","root",  "broker", "core", or the name of a specific object/core/broker
   @param target the specific target of the command
   @param commandStr the actual command
    @param mode fast (asynchronous; default) means the command goes on priority channels, ordered
    (synchronous) is slower but has more ordering guarantees
   */
    virtual void sendCommand(std::string_view target,
                             std::string_view commandStr,
                             HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) = 0;

    /** load a file containing connection information
    @param file a JSON or TOML file containing connection information*/
    virtual void makeConnections(const std::string& file) = 0;
    /** create a data Link between two endpoints sending data from source to destination
    @param source the name of the endpoint to send from
    @param target the name of the endpoint to send the data to*/
    virtual void linkEndpoints(std::string_view source, std::string_view target) = 0;
    /** create a data Link between a named publication and a named input
    @param source the name of the publication
    @param target the name of the input*/
    virtual void dataLink(std::string_view source, std::string_view target) = 0;
    /** create a filter connection between a named filter and a named endpoint for messages coming
    from that endpoint
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addSourceFilterToEndpoint(std::string_view filter, std::string_view target) = 0;
    /** create a filter connection between a named filter and a named endpoint for destination
    processing
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addDestinationFilterToEndpoint(std::string_view filter,
                                                std::string_view target) = 0;
    /** make an alias for an interface, which is a second string that may be used for making
    connections
    @param interfaceKey the name of the original interface
    @param alias the second name by which it can be used
    */
    virtual void addAlias(std::string_view interfaceKey, std::string_view alias) = 0;
    /** update a time barrier with a new time*/
    virtual void setTimeBarrier(Time barrierTime) = 0;

    /** update a time barrier with a new time*/
    virtual void clearTimeBarrier() = 0;

    /**
    * generate a global ERROR_STATE and halt the federation
    @param errorCode the code to use for the ERROR_STATE
    @param errorString the ERROR_STATE message to associate with the ERROR_STATE
    */
    virtual void globalError(int32_t errorCode, std::string_view errorString) = 0;
};
}  // namespace helics
