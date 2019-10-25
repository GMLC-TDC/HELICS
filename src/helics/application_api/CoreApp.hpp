/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-types.hpp"
#include "helics_cxx_export.h"

#include <memory>
#include <string>
#include <vector>
#include <chrono>

namespace helics
{
class Core;
class helicsCLI11App;

/** class implementing a Core object.  This object is meant to a be a very simple broker executor with a similar
 * interface to the other apps
 */
class HELICS_CXX_EXPORT CoreApp
{
  public:
    /** default constructor*/
    CoreApp () = default;
    /** construct from command line arguments in a vector
 @param args the command line arguments to pass in a reverse vector
 */
    explicit CoreApp (std::vector<std::string> args);
    /** construct from command line arguments in a vector
     @param ctype the type of broker to create
@param args the command line arguments to pass in a reverse vector
*/
    CoreApp (core_type ctype, std::vector<std::string> args);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp (int argc, char *argv[]);
    /** construct from command line arguments
    @param ctype the type of broker to create
    @param argc the number of arguments
    @param argv the strings in the input
    */
    CoreApp (core_type ctype, int argc, char *argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit CoreApp (const std::string &argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of broker to create
    @param argString a merged string with all the arguments
    */
    CoreApp (core_type ctype, const std::string &argString);

    /** check if the Core is running*/
    bool isConnected () const;

    /** forceably disconnect the broker*/
    void forceTerminate ();
    /** wait for the broker to normally disconnect for a certain amount of time*/
    bool waitForDisconnect (std::chrono::milliseconds waitTime = std::chrono::milliseconds (0));
    /** link a publication and input*/
    void dataLink (const std::string &source, const std::string &target);
    /** add a source Filter to an endpoint*/
    void addSourceFilterToEndpoint (const std::string &filter, const std::string &endpoint);
    /** add a destination Filter to an endpoint*/
    void addDestinationFilterToEndpoint (const std::string &filter, const std::string &endpoint);

    /** get the identifier of the broker*/
    const std::string &getIdentifier () const;
    /** get the network address of the broker*/
    const std::string &getAddress () const;
    /** make a query at the broker*/
    std::string query (const std::string &target, const std::string &query);
    /** set the log file to use for the broker*/
    void setLogFile (const std::string &logFile);
    /** tell the core that is ready to enter initialization mode*/
	void setReadyToInit ();
#ifdef HELICS_CXX_STATIC_DEFINE
    /** overload the -> operator so broker functions can be called if needed
     */
    auto *operator-> () const { return core.operator-> (); }
#endif
    /** get a copy of the core pointer*/
    std::shared_ptr<Core> getCopyofCorePointer () const { return core; }
  private:
    void processArgs (std::unique_ptr<helicsCLI11App> &app);
    std::unique_ptr<helicsCLI11App> generateParser ();
    std::shared_ptr<Core> core;  //!< the actual endpoint objects
    std::string name;  //!< the name of the broker
};

/** class that waits for a core to terminate before finishing the destructor*/
class CoreKeeper
{
  public:
    template <class... Args>
    CoreKeeper (Args &&... args) : cr (std::forward<... Args> (args))
    {
    }
    CoreKeeper (CoreKeeper &&brkeep) = default;
    CoreKeeper (const CoreKeeper &crkeep) = default;
    CoreKeeper &operator= (CoreKeeper &&crkeep) = default;
    CoreKeeper &operator= (const CoreKeeper &crkeep) = default;
    /// is the core connected
    bool isConnected () { return cr.isConnected (); }
    /// Force terminate the core
    void forceTerminate () { cr.forceTerminate (); }
    /// the destructor waits for the broker to terminate
    ~CoreKeeper ()
    {
		if (cr.isConnected ())
        {
            cr.waitForDisconnect ();
        }
    }

  private:
   CoreApp cr;
};
}  // namespace helics
