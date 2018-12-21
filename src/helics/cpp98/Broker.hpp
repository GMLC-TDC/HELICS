/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_BROKER_HPP_
#define HELICS_CPP98_BROKER_HPP_
#pragma once

#include "../shared_api_library/helics.h"
#include "config.hpp"
#include "helicsExceptions.hpp"
#include <stdexcept>
#include <string>

namespace helicscpp
{
class Broker
{
  public:
    /** Default constructor */
    Broker () : broker (NULL){};

    Broker (std::string type, std::string name, std::string initString)
    {
        broker = helicsCreateBroker (type.c_str (), name.c_str (), initString.c_str (), hThrowOnError ());
        if (helicsBrokerIsConnected (broker) != helics_true)
        {
            throw (std::runtime_error ("broker creation failed"));
        }
    }

    Broker (std::string type, std::string name, int argc, const char **argv)
    {
        broker = helicsCreateBrokerFromArgs (type.c_str (), name.c_str (), argc, argv, hThrowOnError ());
    }

    Broker (const Broker &brk) { broker = helicsBrokerClone (brk.broker, hThrowOnError ()); }
    Broker &operator= (const Broker &brk)
    {
        broker = helicsBrokerClone (brk.broker, hThrowOnError ());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    Broker (Broker &&brk) noexcept
    {
        broker = brk.broker;
        brk.broker = NULL;
    }
    Broker &operator= (Broker &&brk) noexcept
    {
        broker = brk.broker;
        brk.broker = NULL;
        return *this;
    }
#endif
    virtual ~Broker ()
    {
        if (broker != NULL)
        {
            helicsBrokerFree (broker);
        }
    }

    operator helics_broker () { return broker; }

    helics_broker baseObject () const { return broker; }

    bool isConnected () const { return helicsBrokerIsConnected (broker); }
    bool waitForDisconnect (int msToWait = -1)
    {
        return helicsBrokerWaitForDisconnect (broker, msToWait, hThrowOnError ());
    }
    void disconnect () { helicsBrokerDisconnect (broker, hThrowOnError ()); }
    const char *getIdentifier () const { return helicsBrokerGetIdentifier (broker); }

    const char *getAddress () const { return helicsBrokerGetAddress (broker); }

    /** set a global federation value*/
    void setGlobal (const std::string &valueName, const std::string &value)
    {
        helicsBrokerSetGlobal (broker, valueName.c_str (), value.c_str (), hThrowOnError ());
    }

  protected:
    helics_broker broker;
};

}  // namespace helicscpp
#endif
