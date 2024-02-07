/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/CoreTypes.hpp"
#include "helics_cxx_export.h"

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class Core;
class helicsCLI11App;

/** class implementing a Core object.  This object is meant to a be a very simple core executor with
 * a similar interface to the other apps
 */
class HELICS_CXX_EXPORT CoreApp {
  public:
    /** default constructor*/
    CoreApp() = default;
    /** construct from command line arguments in a vector
 @param args the command line arguments to pass in a reverse vector
 */
    explicit CoreApp(std::vector<std::string> args);
    /** construct from command line arguments in a vector
     @param ctype the type of core to create
@param args the command line arguments to pass in a reverse vector
*/
    CoreApp(CoreType ctype, std::vector<std::string> args);

    /** construct from command line arguments in a vector
   @param ctype the type of core to create
   @param coreName the name of the core to use
   @param args the command line arguments to pass in a reverse vector
*/
    CoreApp(CoreType ctype, std::string_view coreName, std::vector<std::string> args);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp(int argc, char* argv[]);
    /** construct from command line arguments
    @param ctype the type of core to create
    @param coreName the name of the core to use
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp(CoreType ctype, std::string_view coreName, int argc, char* argv[]);
    /** construct from command line arguments
    @param ctype the type of core to create
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp(CoreType ctype, int argc, char* argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit CoreApp(std::string_view argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of core to create
    @param argString a merged string with all the arguments
    */
    CoreApp(CoreType ctype, std::string_view argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of core to create
    @param coreName the name of the core to use or create
    @param argString a merged string with all the arguments
    */
    CoreApp(CoreType ctype, std::string_view coreName, std::string_view argString);

    /** create a CoreApp from a core pointer*/
    explicit CoreApp(std::shared_ptr<Core> cr);
    /** check if the Core is running*/
    bool isConnected() const;

    /** connect the Core to its broker*/
    bool connect();

    /** check if the broker is ready to accept new federates or cores
     */
    bool isOpenToNewFederates() const;
    /** forcibly disconnect the core*/
    void forceTerminate();
    /** wait for the core to normally disconnect for a certain amount of time*/
    bool waitForDisconnect(std::chrono::milliseconds waitTime = std::chrono::milliseconds(0));
    /** link two endpoints together with source and destination*/
    void linkEndpoints(std::string_view source, std::string_view target);
    /** link a publication and input*/
    void dataLink(std::string_view source, std::string_view target);
    /** add a source Filter to an endpoint*/
    void addSourceFilterToEndpoint(std::string_view filter, std::string_view endpoint);
    /** add a destination Filter to an endpoint*/
    void addDestinationFilterToEndpoint(std::string_view filter, std::string_view endpoint);
    /** add an alias for an interface*/
    void addAlias(std::string_view interfaceName, std::string_view alias);
    /** make connections from a core using a file*/
    void makeConnections(const std::string& file);
    /** get the identifier of the core*/
    const std::string& getIdentifier() const;
    /** get the network address of the core*/
    const std::string& getAddress() const;

    /** make a query at the core
    @param target the target of the query "federation", "parent", "core","broker" or a specific
    named object
    @param queryStr the query to make
    @param mode defaults to fast (asynchronous) meaning the query goes into priority channels,
    ordered (synchronous) means slower queries but has more ordering guarantees
    @return a string containing the query results
    */
    std::string query(std::string_view target,
                      std::string_view queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** set a tag (key-value pair) for a core
  @details the tag is an arbitrary user defined string and value; the tags for a core are
  queryable through a "tags" query or "tag/<tagname>"
  @param tag the name of the tag to set the value for
  @param value the value for the given tag*/
    void setTag(std::string_view tag, std::string_view value = "true");
    /** get the value of a specific tag (key-value pair) for a core
    @details the tag is an arbitrary user defined string and value; the tags for a core are
    queryable
    @param tag the name of the tag to get the value for
    @return a std::string containing the value of the tag, if the tag is not defined the
    value is an empty string*/
    const std::string& getTag(std::string_view tag) const;

    /** set a federation global value
    @details this overwrites any previous value for this name
    globals can be queried with a target of "global" or "global_value" and queryStr of the value to
    Query
    @param valueName the name of the global to set
    @param value the value of the global
    */
    void setGlobal(std::string_view valueName, std::string_view value = "true");

    /** send a command to a specific target
   @details the format is somewhat unspecified; target is the name of an object, typically one of
   "federation",  "broker", "core", or the name of a specific object/core/broker
   @param target the specific target of the command
   @param commandStr the actual command
    @param mode defaults to fast (asynchronous) meaning the command goes into priority channels,
    ordered (synchronous) means slower commands but has more ordering guarantees
   */
    void sendCommand(std::string_view target,
                     std::string_view commandStr,
                     HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** set the log file to use for the core*/
    void setLogFile(std::string_view logFile);
    /** set the minimum log level to use in the core*/
    void setLoggingLevel(int loglevel);
    /** tell the core that is ready to enter initialization mode*/
    void setReadyToInit();
    /** tell the core to pause init even if otherwise ready*/
    void haltInit();
#ifdef HELICS_CXX_STATIC_DEFINE
    /** overload the -> operator so core functions can be called if needed
     */
    auto* operator->() const { return core.operator->(); }
#else
    CoreApp* operator->() { return this; }
    const CoreApp* operator->() const { return this; }
#endif
    /** get a copy of the core pointer*/
    std::shared_ptr<Core> getCopyofCorePointer() const { return core; }

    /** reset the app to default state*/
    void reset();
    /** generate a global error on a core*/
    void globalError(int32_t errorCode, std::string_view errorString);

  private:
    void processArgs(std::unique_ptr<helicsCLI11App>& app);
    std::unique_ptr<helicsCLI11App> generateParser();

    std::shared_ptr<Core> core;  //!< the actual core object
    std::string name;  //!< the name of the core
};

/** class that waits for a core to terminate before finishing the destructor*/
class CoreKeeper {
  public:
    template<class... Args>
    explicit CoreKeeper(Args&&... args): cr(std::forward<Args...>(args...))
    {
    }
    CoreKeeper(CoreKeeper&& brkeep) = default;
    CoreKeeper(const CoreKeeper& crkeep) = default;
    CoreKeeper& operator=(CoreKeeper&& crkeep) = default;
    CoreKeeper& operator=(const CoreKeeper& crkeep) = default;
    /// is the core connected
    bool isConnected() { return cr.isConnected(); }
    /// Force terminate the core
    void forceTerminate() { cr.forceTerminate(); }
    /// the destructor waits for the core to terminate
    ~CoreKeeper()
    {
        if (cr.isConnected()) {
            cr.waitForDisconnect();
        }
    }

  private:
    CoreApp cr;
};
}  // namespace helics
