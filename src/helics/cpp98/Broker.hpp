/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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
    Broker () HELICS_NOTHROW: broker (HELICS_NULL_POINTER){};

    Broker (std::string type, std::string name, std::string initString)
    {
        broker = helicsCreateBroker (type.c_str (), name.c_str (), initString.c_str (), hThrowOnError ());
        if (helicsBrokerIsConnected (broker) != helics_true)
        {
            throw (std::runtime_error ("broker creation failed"));
        }
    }

    Broker (std::string type, std::string name, int argc, char **argv)
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
    Broker (Broker &&brk) HELICS_NOTHROW
    {
        broker = brk.broker;
        brk.broker = HELICS_NULL_POINTER;
    }
    Broker &operator= (Broker &&brk) HELICS_NOTHROW
    {
        broker = brk.broker;
        brk.broker = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    virtual ~Broker ()
    {
        if (broker != HELICS_NULL_POINTER)
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
