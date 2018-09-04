/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_CORE_HPP_
#define HELICS_CPP98_CORE_HPP_
#pragma once

#include "../shared_api_library/helics.h"
#include "../shared_api_library/MessageFilters.h"
#include "config.hpp"
#include <string>
#include "../cpp98/Filter.hpp"
#include <exception>

#define HELICS_IGNORE_ERROR NULL
namespace helicscpp
{

class Core
{
  public:
    // Default constructor, not meant to be used
    Core ():core(NULL) {};

    Core (const std::string &type, const std::string &name, const std::string &initString)
    {
        core = helicsCreateCore (type.c_str(), name.c_str(), initString.c_str(),hThrowOnError());
    }

    Core (const std::string &type, const std::string &name, int argc, const char **argv)
    {
        core = helicsCreateCoreFromArgs (type.c_str(), name.c_str(), argc, argv,hThrowOnError());
    }

    ~Core ()
    {
        helicsCoreFree(core);
    }
    operator helics_core() { return core; }

    helics_core baseObject() const { return core; }
    bool isConnected () const
    { return helicsCoreIsConnected (core);
    }

    Core(const Core &cr)
    {
        core = helicsCoreClone(cr.core,hThrowOnError());
    }
    Core &operator=(const Core &cr)
    {
        core = helicsCoreClone (cr.core, hThrowOnError ());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    Core(Core &&cr) noexcept
    {
        core = cr.core;
        cr.core = NULL;
    }
    Core &operator=(Core &&cr) noexcept
    {
        core = cr.core;
        cr.core = NULL;
        return *this;
    }
#endif
    void setReadyToInit()
    { helicsCoreSetReadyToInit (core, hThrowOnError ());
    }
    void disconnect()
    { helicsCoreDisconnect (core, hThrowOnError ());
    }

    const char *getIdentifier() const
    {
        return helicsCoreGetIdentifier(core);
    }

    /** create a destination Filter on the specified federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param fed the fed to register through
    @param type the type of filter to create
    @param target the name of endpoint to target
    @param name the name of the filter (can be NULL)
    @return a helics_filter object
    */
    Filter registerFilter(helics_filter_type_t type,
        const std::string &name = std::string())
    {
        return Filter (helicsCoreRegisterFilter (core, type, name.c_str (), hThrowOnError ()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param fed the fed to register through
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a helics_filter object
    */
    CloningFilter registerCloningFilter( const std::string &deliveryEndpoint)
    {
        return CloningFilter (helicsCoreRegisterCloningFilter (core, deliveryEndpoint.c_str (), hThrowOnError ()));
    }

  protected:
    helics_core core;
};

} //namespace helicscpp
#endif
