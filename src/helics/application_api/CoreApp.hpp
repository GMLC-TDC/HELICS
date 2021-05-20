/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-types.hpp"
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
    CoreApp(core_type ctype, std::vector<std::string> args);

    /** construct from command line arguments in a vector
   @param ctype the type of core to create
   @param coreName the name of the core to use
   @param args the command line arguments to pass in a reverse vector
*/
    CoreApp(core_type ctype, const std::string& coreName, std::vector<std::string> args);
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
    CoreApp(core_type ctype, const std::string& coreName, int argc, char* argv[]);
    /** construct from command line arguments
    @param ctype the type of core to create
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp(core_type ctype, int argc, char* argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit CoreApp(const std::string& argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of core to create
    @param argString a merged string with all the arguments
    */
    CoreApp(core_type ctype, const std::string& argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of core to create
    @param coreName the name of the core to use or create
    @param argString a merged string with all the arguments
    */
    CoreApp(core_type ctype, const std::string& coreName, const std::string& argString);

    /** create a CoreApp from a core pointer*/
    explicit CoreApp(std::shared_ptr<Core> cr);
    /** check if the Core is running*/
    bool isConnected() const;

    /** connect the Core to its broker*/
    bool connect();

    /** check if the broker is ready to accept new federates or cores
     */
    bool isOpenToNewFederates() const;
    /** forceably disconnect the core*/
    void forceTerminate();
    /** wait for the core to normally disconnect for a certain amount of time*/
    bool waitForDisconnect(std::chrono::milliseconds waitTime = std::chrono::milliseconds(0));
    /** link a publication and input*/
    void dataLink(const std::string& source, const std::string& target);
    /** add a source Filter to an endpoint*/
    void addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint);
    /** add a destination Filter to an endpoint*/
    void addDestinationFilterToEndpoint(const std::string& filter, const std::string& endpoint);
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
    std::string query(const std::string& target,
                      const std::string& queryStr,
                      helics_sequencing_mode mode = helics_sequencing_mode_fast);

    /** set a federation global value
    @details this overwrites any previous value for this name
    globals can be queried with a target of "global" and queryStr of the value to Query
    @param valueName the name of the global to set
    @param value the value of the global
    */
    void setGlobal(const std::string& valueName, const std::string& value);

    /** set the log file to use for the core*/
    void setLogFile(const std::string& logFile);
    /** set the minimum log level to use in the core*/
    void setLoggingLevel(int loglevel);
    /** tell the core that is ready to enter initialization mode*/
    void setReadyToInit();
    /** tell the core to pause init even if otherwise ready*/
    void haltInit();
#ifdef HELICS_CXX_STATIC_DEFINE
    /** overload the -> operator so core functions can be called if needed
     */
    auto* operator-> () const { return core.operator->(); }
#else
    CoreApp* operator->() { return this; }
    const CoreApp* operator->() const { return this; }
#endif
    /** get a copy of the core pointer*/
    std::shared_ptr<Core> getCopyofCorePointer() const { return core; }

    /** reset the app to default state*/
    void reset();
    /** generate a global error on a core*/
    void globalError(int32_t errorCode, const std::string& errorString);

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
