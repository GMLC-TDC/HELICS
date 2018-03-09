/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#ifndef HELICS_CPP98_BROKER_HPP_
#define HELICS_CPP98_BROKER_HPP_
#pragma once

#include "shared_api_library/helics.h"

#include <string>

namespace helics
{

class Broker
{
  public:
    // Default constructor, not meant to be used
    Broker () {};

    Broker (std::string type, std::string name, std::string initString)
    {
        broker = helicsCreateBroker (type.c_str(), name.c_str(), initString.c_str());
    }

    Broker (std::string type, std::string name, int argc, const char **argv)
    {
        broker = helicsCreateBrokerFromArgs (type.c_str(), name.c_str(), argc, argv);
    }

    virtual ~Broker ()
    {
        helicsBrokerFree (broker);
    }

    bool isConnected ()
    {
        return helicsBrokerIsConnected (broker);
    }

  protected:
    helics_broker broker;
};

} //namespace helics
#endif

