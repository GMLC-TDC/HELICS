/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

    bool isConnected () const
    {
        return helicsCoreIsConnected (core);
    }

    Core(const Core &cr)
    {
        core = helicsCoreClone(cr.core);
    }
    Core &operator=(const Core &cr)
    {
        core = helicsCoreClone(cr.core);
        return *this;
    }
    void setReadyToInit()
    {
        helicsCoreSetReadyToInit(core);
    }
    void disconnect()
    {
        helicsCoreDisconnect(core);
    }

    std::string getIdentifier() const
    {
        char str[255];
        helicsCoreGetIdentifier(core, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

  protected:
    helics_core core;
};

} //namespace helics
#endif
