/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_CPP98_CORE_HPP_
#define HELICS_CPP98_CORE_HPP_
#pragma once

#include "../shared_api_library/helics.h"

#include <string>

namespace helics
{

class Core
{
  public:
    // Default constructor, not meant to be used
    Core () {};

    Core (const std::string &type, const std::string &name, const std::string &initString)
    {
        core = helicsCreateCore (type.c_str(), name.c_str(), initString.c_str());
    }

    Core (const std::string &type, const std::string &name, int argc, const char **argv)
    {
        core = helicsCreateCoreFromArgs (type.c_str(), name.c_str(), argc, argv);
    }

    virtual ~Core ()
    {
        helicsCoreFree (core);
    }

    bool isConnected ()
    {
        return helicsCoreIsConnected (core);
    }

  protected:
    helics_core core;
};

} //namespace helics
#endif
