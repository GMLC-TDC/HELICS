/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <functional>
#include <string>
#include <chrono>

namespace helics
{
/** virtual class defining a public interface to a broker*/
class Broker
{
  public:
    /**default constructor
    @param setAsRootBroker  set to true to indicate this object is a root broker*/
    Broker () = default;
    /** destructor*/
    virtual ~Broker () = default;

    /** connect the core to its broker
    @details should be done after initialization has complete*/
    virtual bool connect () = 0;
    /** disconnect the broker from any other brokers and communications
     */
    virtual void disconnect () = 0;

    /** check if the broker is connected*/
    virtual bool isConnected () const = 0;
    /** set the broker to be a root broker
    @details only valid before the initialization function is called*/
    virtual void setAsRoot () = 0;
    /** return true if the broker is a root broker
     */
    virtual bool isRoot () const = 0;

    /** check if the broker is ready to accept new federates or cores
     */
    virtual bool isOpenToNewFederates () const = 0;
    /** start up the broker with an initialization string containing commands and parameters*/
    virtual void initialize (const std::string &initializationString) = 0;
    /** initialize from command line arguments
     */
    virtual void initializeFromArgs (int argc, const char *const *argv) = 0;

    /** get the local identification for the broker*/
    virtual const std::string &getIdentifier () const = 0;
    /** get the connection address for the broker*/
    virtual const std::string &getAddress () const = 0;
    /** set the broker logging level*/
    virtual void setLoggingLevel (int logLevel) =0;
    /** set the logging callback function
    @param logFunction a function with a signature of void(int level,  const std::string &source,  const
    std::string &message) the function takes a level indicating the logging level string with the source name and a
    string with the message
    */
    virtual void
    setLoggingCallback (const std::function<void(int, const std::string &, const std::string &)> &logFunction) = 0;

    /** waits in the current thread until the broker is disconnected
	@param msToWait  the timeout to wait for disconnect 
	@return true if the disconnect was successful false if it timed out
     */
    virtual bool waitForDisconnect (std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const = 0;
    /** make a query for information from the co-simulation
    @details the format is somewhat unspecified  target is the name of an object typically one of
    "federation",  "broker", or the name of a specific object
    query is a broken
    @param target the specific target of the query
    @param queryStr the actual query
      @return a string containing the response to the query.  Query is a blocking call and will not return until
      the query is answered so use with caution
    */
    virtual std::string query (const std::string &target, const std::string &queryStr) = 0;

	/** set a federation global value
	@details this overwrites any previous value for this name
	globals can be queried with a target of "global" and queryStr of the value to Query
	@param valueName the name of the global to set
	@param value the value of the global
	*/
	virtual void setGlobal(const std::string &valueName, const std::string &value) = 0;

    /** load a file containing connection information 
    @param file a JSON or TOML file containing connection information*/
    virtual void makeConnections(const std::string &file) = 0;
    /** create a data Link between a named publication and a named input
    @param source the name of the publication
    @param target the name of the input*/
    virtual void dataLink (const std::string &source, const std::string &target) = 0;
    /** create a source filter connection between a named filter and a named endpoint
    @param source the name of the filter
    @param target the name of the source target*/
    virtual void addSourceFilterToEndpoint (const std::string &filter, const std::string &target) = 0;
    /** create a destination filter connection between a named filter and an endpoint
    @param source the name of the filter
    @param target the name of the source target*/
    virtual void addDestinationFilterToEndpoint (const std::string &filter, const std::string &target) = 0;
};
}  // namespace helics
