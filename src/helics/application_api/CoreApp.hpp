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

namespace helics
{
class Core;
class helicsCLI11App;

namespace apps
{
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
    /** move construction*/
    CoreApp (CoreApp &&coreApp) = default;
    /** move assignment*/
    CoreApp &operator= (CoreApp &&coreApp) = default;
    /** the destructor will wait until the broker is finished before returning- unless forceTerminate() is used*/
    ~CoreApp ();

    /** check if the Core is running*/
    bool isActive () const;

    /** forceably disconnect the broker*/
    void forceTerminate ();
    /** overload the -> operator so broker functions can be called if needed
     */
    auto *operator-> () const { return core.operator-> (); }
    /** get a copy of the core pointer*/
    std::shared_ptr<Core> getCopyofCorePointer () const { return core; }

  private:
    void processArgs (std::unique_ptr<helicsCLI11App> &app);
    std::unique_ptr<helicsCLI11App> generateParser ();
    std::shared_ptr<Core> core;  //!< the actual endpoint objects
    std::string name;  //!< the name of the broker
};
}  // namespace apps
}  // namespace helics
