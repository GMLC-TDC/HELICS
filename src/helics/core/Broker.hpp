/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once

#include <string>

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
    virtual std::string getAddress () const = 0;
};
}  // namespace helics

