/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics
{
class Broker;
class helicsCLI11App;

namespace apps
{
/** class implementing a Broker object.  This object is meant to a be a very simple broker executor with a similar
 * interface to the other apps
 */
class BrokerApp
{
  public:
    /** default constructor*/
    BrokerApp () = default;
    /** construct from command line arguments in a vector
 @param args the command line arguments to pass in a reverse vector
 */
    explicit BrokerApp (std::vector<std::string> args);
    /** construct from command line arguments in a vector
     @param ctype the type of broker to create
@param args the command line arguments to pass in a reverse vector
*/
    BrokerApp (core_type ctype, std::vector<std::string> args);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerApp (int argc, char *argv[]);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerApp (core_type ctype, int argc, char *argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit BrokerApp (const std::string &argString);
    /** construct from command line arguments parsed as a single string
    @param ctype the type of broker to create
    @param argString a merged string with all the arguments
    */
    BrokerApp (core_type ctype, const std::string &argString);
    /** move construction*/
    BrokerApp (BrokerApp &&brokerApp) = default;
    /** move assignment*/
    BrokerApp &operator= (BrokerApp &&brokerApp) = default;
    /** the destructor will wait until the broker is finished before returning- unless forceTerminate() is used*/
    ~BrokerApp ();

    /** check if the Broker is running*/
    bool isActive () const;

    /** forceably disconnect the broker*/
    void forceTerminate ();
    /** overload the -> operator so broker functions can be called if needed
     */
    auto *operator-> () const { return broker.operator-> (); }

  private:
    void processArgs (std::unique_ptr<helicsCLI11App> &app);
    std::unique_ptr<helicsCLI11App> generateParser ();
    core_type type = core_type::DEFAULT;
    std::shared_ptr<Broker> broker;  //!< the actual endpoint objects
    std::string name;  //!< the name of the broker
};
}  // namespace apps
}  // namespace helics
