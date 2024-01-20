/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_BROKER_HPP_
#define HELICS_CPP98_BROKER_HPP_
#pragma once

#include "config.hpp"
#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <stdexcept>
#include <string>

namespace helicscpp {
class Broker {
  public:
    /** Default constructor */
    Broker() HELICS_NOTHROW: broker(HELICS_NULL_POINTER) {}
    /** construct broker
    @param type string with the type of the broker to create
    @param name the name of the broker
    @param initString command line argument for starting the broker
    */
    Broker(std::string type, std::string name, std::string initString)
    {
        broker =
            helicsCreateBroker(type.c_str(), name.c_str(), initString.c_str(), hThrowOnError());
        if (helicsBrokerIsConnected(broker) != HELICS_TRUE) {
            throw(std::runtime_error("broker creation failed"));
        }
    }
    /** construct broker
    @param type string with the type of the broker to create
    @param name the name of the broker
    @param argc the number of command line arguments
    @param argv command line argument strings
    */
    Broker(std::string type, std::string name, int argc, char** argv)
    {
        broker =
            helicsCreateBrokerFromArgs(type.c_str(), name.c_str(), argc, argv, hThrowOnError());
    }
    /** copy constructor*/
    Broker(const Broker& brk) { broker = helicsBrokerClone(brk.broker, hThrowOnError()); }
    /** copy assignment*/
    Broker& operator=(const Broker& brk)
    {
        broker = helicsBrokerClone(brk.broker, hThrowOnError());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor*/
    Broker(Broker&& brk) HELICS_NOTHROW: broker(brk.broker) { brk.broker = HELICS_NULL_POINTER; }
    /** move assignment*/
    Broker& operator=(Broker&& brk) HELICS_NOTHROW
    {
        broker = brk.broker;
        brk.broker = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    /** destructor*/
    virtual ~Broker()
    {
        if (broker != HELICS_NULL_POINTER) {
            helicsBrokerFree(broker);
        }
    }
    /** cast to the underlying broker*/
    operator HelicsBroker() { return broker; }
    /** get the underlying HelicsBroker object
    @return a HelicsBroker object*/
    HelicsBroker baseObject() const { return broker; }
    /** check if the broker is connected*/
    bool isConnected() const { return (helicsBrokerIsConnected(broker) != HELICS_FALSE); }
    /** waits in the current thread until the broker is disconnected
    @param msToWait  the timeout to wait for disconnect (-1) implies no timeout
    @return true if the disconnect was successful false if it timed out
     */
    bool waitForDisconnect(int msToWait = -1)
    {
        return (helicsBrokerWaitForDisconnect(broker, msToWait, hThrowOnError()) != HELICS_FALSE);
    }
    /** disconnect the broker from any other brokers and communications
     */
    void disconnect() { helicsBrokerDisconnect(broker, hThrowOnError()); }
    /** get the local identification for the broker*/
    const char* getIdentifier() const { return helicsBrokerGetIdentifier(broker); }
    /** get the connection address for the broker*/
    const char* getAddress() const { return helicsBrokerGetAddress(broker); }

    /** set a federation global value
    @details this overwrites any previous value for this name
    globals can be queried with a target of "global" or "global_value" and queryStr of the value to
    Query "global_value" will return the string directly.  "global" will return the value and name
    in a JSON structure
    @param valueName the name of the global to set
    @param value the value of the global
    */
    void setGlobal(const std::string& valueName, const std::string& value)
    {
        helicsBrokerSetGlobal(broker, valueName.c_str(), value.c_str(), hThrowOnError());
    }
    /** add a global alias for an interface
    @param interfaceName the given name of the interface
    @param alias the new name by which the interface can be referenced
    */
    void addAlias(const std::string& interfaceName, const std::string& alias)
    {
        helicsBrokerAddAlias(broker, interfaceName.c_str(), alias.c_str(), hThrowOnError());
    }
    /** create a data link between a named publication and a named input
     @param source the name of the publication
     @param target the name of the input*/
    void dataLink(const std::string& source, const std::string& target)
    {
        helicsBrokerDataLink(broker, source.c_str(), target.c_str(), hThrowOnError());
    }
    /** create a filter connection between a named filter and a named endpoint for messages coming
    from that endpoint
    @param filter the name of the filter
    @param target the name of the source target*/
    void addSourceFilterToEndpoint(const std::string& filter, const std::string& target)
    {
        helicsBrokerAddSourceFilterToEndpoint(broker,
                                              filter.c_str(),
                                              target.c_str(),
                                              hThrowOnError());
    }
    /** create a filter connection between a named filter and a named endpoint for destination
    processing
    @param filter the name of the filter
    @param target the name of the source target*/
    void addDestinationFilterToEndpoint(const std::string& filter, const std::string& target)
    {
        helicsBrokerAddDestinationFilterToEndpoint(broker,
                                                   filter.c_str(),
                                                   target.c_str(),
                                                   hThrowOnError());
    }

    /** make a query of the broker
  @details this call is blocking until the value is returned which may take some time depending
  on the size of the federation and the specific string being queried
  @param target  the target of the query can be "federation", "federate", "broker", "core", or a
  specific name of a federate, core, or broker
  @param queryStr a string with the query, see other documentation for specific properties to
  query, can be defined by the federate
  @param mode the ordering mode to use for the query (fast for priority channels, ordered for normal
  channels ordered with all other messages)
  @return a string with the value requested.  this is either going to be a vector of strings value
  or a JSON string stored in the first element of the vector.  The string "#invalid" is returned
  if the query was not valid
  */
    std::string query(const std::string& target,
                      const std::string& queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) const
    {
        // returns HelicsQuery
        HelicsQuery q = helicsCreateQuery(target.c_str(), queryStr.c_str());
        if (mode != HELICS_SEQUENCING_MODE_FAST) {
            helicsQuerySetOrdering(q, mode, HELICS_IGNORE_ERROR);
        }
        std::string result(helicsQueryBrokerExecute(q, broker, hThrowOnError()));
        helicsQueryFree(q);
        return result;
    }

    /** send an asynchronous command to another object
    @param target the name of the target of the command
    @param command the command message to send
    */
    void sendCommand(const std::string& target, const std::string& command)
    {
        helicsBrokerSendCommand(broker, target.c_str(), command.c_str(), hThrowOnError());
    }

    void setTimeBarrier(HelicsTime barrierTime)
    {
        helicsBrokerSetTimeBarrier(broker, barrierTime, HELICS_IGNORE_ERROR);
    }

    void clearTimeBarrier() { helicsBrokerClearTimeBarrier(broker); }

    void globalError(int errorCode, const std::string& errorString)
    {
        helicsBrokerGlobalError(broker, errorCode, errorString.c_str(), HELICS_IGNORE_ERROR);
    }

  protected:
    HelicsBroker broker;  //!< underlying broker information
};

}  // namespace helicscpp
#endif
