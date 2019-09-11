/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_CORE_HPP_
#define HELICS_CPP98_CORE_HPP_
#pragma once

#include "../cpp98/Filter.hpp"
#include "../shared_api_library/MessageFilters.h"
#include "../shared_api_library/helics.h"
#include "config.hpp"
#include <stdexcept>
#include <string>

namespace helicscpp
{
class Core
{
  public:
    /** Default constructor*/
    Core () HELICS_NOTHROW: core (HELICS_NULL_POINTER){};
    /** construct with type, core name and initialization string */
    Core (const std::string &type, const std::string &name, const std::string &initString)
    {
        core = helicsCreateCore (type.c_str (), name.c_str (), initString.c_str (), hThrowOnError ());
    }
    /** construct with type, core name and command line arguments */
    Core (const std::string &type, const std::string &name, int argc, char **argv)
    {
        core = helicsCreateCoreFromArgs (type.c_str (), name.c_str (), argc, argv, hThrowOnError ());
    }
    /** destructor*/
    ~Core () { helicsCoreFree (core); }
    /** implicit operator so the object can be used with the c api functions natively*/
    operator helics_core () { return core; }
    /** explicity get the base helics_core object*/
    helics_core baseObject () const { return core; }
    bool isConnected () const { return helicsCoreIsConnected (core); }

    Core (const Core &cr) { core = helicsCoreClone (cr.core, hThrowOnError ()); }
    Core &operator= (const Core &cr)
    {
        core = helicsCoreClone (cr.core, hThrowOnError ());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    Core (Core &&cr) HELICS_NOTHROW
    {
        core = cr.core;
        cr.core = HELICS_NULL_POINTER;
    }
    Core &operator= (Core &&cr) HELICS_NOTHROW
    {
        core = cr.core;
        cr.core = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    void setReadyToInit () { helicsCoreSetReadyToInit (core, hThrowOnError ()); }
    void disconnect () { helicsCoreDisconnect (core, hThrowOnError ()); }

    const char *getIdentifier () const { return helicsCoreGetIdentifier (core); }

    /** create a destination Filter on the specified federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param fed the fed to register through
    @param type the type of filter to create
    @param target the name of endpoint to target
    @param name the name of the filter (can be NULL)
    @return a helics_filter object
    */
    Filter registerFilter (helics_filter_type type, const std::string &name = std::string ())
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
    CloningFilter registerCloningFilter (const std::string &deliveryEndpoint)
    {
        return CloningFilter (helicsCoreRegisterCloningFilter (core, deliveryEndpoint.c_str (), hThrowOnError ()));
    }
    /** set a global federation value*/
    void setGlobal (const std::string &valueName, const std::string &value)
    {
        helicsCoreSetGlobal (core, valueName.c_str (), value.c_str (), hThrowOnError ());
    }

  protected:
    helics_core core;
};

}  // namespace helicscpp
#endif
