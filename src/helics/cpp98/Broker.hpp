/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

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
        helicsFreeBroker (broker);
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
