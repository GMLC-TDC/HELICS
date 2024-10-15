/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/CoreTypes.hpp"
#include "../core/helicsTime.hpp"
#include "helics_cxx_export.h"

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class Broker;
class helicsCLI11App;

/** class implementing a Broker object.  This object is meant to a be a very simple broker executor
 * with a similar interface to the other apps
 */
class HELICS_CXX_EXPORT BrokerApp {
  public:
    /** default constructor*/
    BrokerApp() = default;
    /** construct from command line arguments in a vector
    @param args the command line arguments to pass in a reverse vector
    */
    explicit BrokerApp(std::vector<std::string>&& args);
    /** construct from command line arguments in a vector
     @param ctype the type of broker to create
     @param args the command line arguments to pass in a reverse vector
    */
    BrokerApp(CoreType ctype, std::vector<std::string>&& args);
    /** construct from command line arguments in a vector
     @param ctype the type of broker to create
     @param broker_name the name of the broker
     @param args the command line arguments to pass in a reverse vector
    */
    BrokerApp(CoreType ctype, const std::string& broker_name, std::vector<std::string>&& args);
    /** construct from command line arguments in a vector
    @param args the command line arguments to pass in a reverse vector
    */
    explicit BrokerApp(std::vector<std::string>& args);
    /** construct from command line arguments in a vector
    @param ctype the type of broker to create
    @param args the command line arguments to pass in a reverse vector
    */
    BrokerApp(CoreType ctype, std::vector<std::string>& args);
    /** construct from command line arguments in a vector
    @param ctype the type of broker to create
    @param broker_name the name of the broker
    @param args the command line arguments to pass in a reverse vector
    */
    BrokerApp(CoreType ctype, const std::string& broker_name, std::vector<std::string>& args);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerApp(int argc, char* argv[]);
    /** construct from command line arguments
    @param ctype the type of broker to create
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerApp(CoreType ctype, int argc, char* argv[]);
    /** construct from command line arguments
    @param ctype the type of broker to create
    @param brokerName the name of the broker
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerApp(CoreType ctype, std::string_view brokerName, int argc, char* argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit BrokerApp(std::string_view argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of broker to create
    @param argString a merged string with all the arguments
    */
    explicit BrokerApp(CoreType ctype, std::string_view argString = std::string_view{});
    /** construct from command line arguments parsed as a single string
    @param ctype the type of broker to create
    @param brokerName the name of the broker
    @param argString a merged string with all the arguments
    */
    BrokerApp(CoreType ctype, std::string_view brokerName, std::string_view argString);

    /** create a BrokerApp from a broker pointer*/
    explicit BrokerApp(std::shared_ptr<Broker> brk);

    /** check if the Broker is running*/
    bool isConnected() const;
    /** connect the broker to the network*/
    bool connect();
    /** check if the broker is ready to accept new federates or cores
     */
    bool isOpenToNewFederates() const;

    /** forcibly disconnect the broker*/
    void forceTerminate();
    /** wait for the broker to normally disconnect for a certain amount of time*/
    bool waitForDisconnect(std::chrono::milliseconds waitTime = std::chrono::milliseconds(0));
    /** link two endpoints source to destination*/
    void linkEndpoints(std::string_view source, std::string_view target);
    /** link a publication and input*/
    void dataLink(std::string_view source, std::string_view target);
    /** add a source Filter to an endpoint*/
    void addSourceFilterToEndpoint(std::string_view filter, std::string_view endpoint);
    /** add a destination Filter to an endpoint*/
    void addDestinationFilterToEndpoint(std::string_view filter, std::string_view endpoint);
    /** add an alias for an interface*/
    void addAlias(std::string_view interfaceName, std::string_view alias);
    /** make connections between interfaces with a file*/
    void makeConnections(const std::string& file);
    /** get the identifier of the broker*/
    const std::string& getIdentifier() const;
    /** get the network address of the broker*/
    const std::string& getAddress() const;
    /** make a query at the broker
    @param target the target of the query "federation", "parent", "broker", or a specific named
    object
    @param queryStr the query to make
    @param mode the ordering mode to use (fast for asynchronous priority channels, and ordered for
    slower but well ordered queries)
    @return a string containing the query results
    */
    std::string query(std::string_view target,
                      std::string_view queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);
    /** set a federation global value
     @details this overwrites any previous value for this name
     globals can be queried with a target of "global" or "global_value" and queryStr of the value to
     Query
     @param valueName the name of the global to set
     @param value the value of the global
     */
    void setGlobal(std::string_view valueName, std::string_view value);

    /** send a command to a specific target
   @details the format is somewhat unspecified; target is the name of an object, typically one of
   "federation",  "broker", "core", or the name of a specific object/core/broker
   @param target the specific target of the command
   @param commandStr the actual command
   @param mode the ordering mode to use (fast for asynchronous priority channels, and ordered for
    slower but well ordered commands)
   */
    void sendCommand(std::string_view target,
                     std::string_view commandStr,
                     HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** set the log file to use for the broker*/
    void setLogFile(std::string_view logFile);
    /** set the minimum log level to use in the broker*/
    void setLoggingLevel(int loglevel);
    /** clear the pointer to the broker*/
    void reset();
#ifdef HELICS_CXX_STATIC_DEFINE
    /** overload the -> operator so all broker functions can be called if needed
     */
    auto* operator->() const { return broker.operator->(); }
#else
    BrokerApp* operator->() { return this; }
    const BrokerApp* operator->() const { return this; }
#endif
    /** get a copy of the core pointer*/
    std::shared_ptr<Broker> getCopyofBrokerPointer() const { return broker; }

    /** set a global time Barrier*/
    void setTimeBarrier(Time barrierTime);
    /** clear a global time Barrier*/
    void clearTimeBarrier();
    /** generate a global error that will halt a co-simulation*/
    void globalError(int32_t errorCode, std::string_view errorString);

  private:
    void processArgs(std::unique_ptr<helicsCLI11App>& app);
    std::unique_ptr<helicsCLI11App> generateParser(bool noTypeOption = false);
    std::shared_ptr<Broker> broker;  //!< the actual endpoint objects
    std::string name;  //!< the name of the broker
};

/** class that waits for a broker to terminate before finishing the destructor*/
class BrokerKeeper {
  public:
    template<class... Args>
    explicit BrokerKeeper(Args&&... args): brk(std::forward<Args...>(args...))
    {
    }
    BrokerKeeper(BrokerKeeper&& brkeep) = default;
    BrokerKeeper(const BrokerKeeper& brkeep) = default;
    BrokerKeeper& operator=(BrokerKeeper&& brkeep) = default;
    BrokerKeeper& operator=(const BrokerKeeper& brkeep) = default;
    /// is the broker connected
    bool isConnected() { return brk.isConnected(); }
    /// Force terminate the broker
    void forceTerminate() { brk.forceTerminate(); }
    /// the destructor waits for the broker to terminate
    ~BrokerKeeper()
    {
        if (brk.isConnected()) {
            brk.waitForDisconnect();
        }
    }

  private:
    BrokerApp brk;
};

}  // namespace helics
