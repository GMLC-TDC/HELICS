/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helicsCallbacks.h"
#include "../core/Broker.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "internal/api_objects.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

helics_status helicsBrokerAddLoggingCallback (helics_broker broker, void (*logger) (int, const char *, const char *))
{
    if (broker == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto brk = getBroker (broker);
        if (brk == nullptr)
        {
            return helics_invalid_object;
        }
        brk->setLoggingCallback ([logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsCoreAddLoggingCallback (helics_core core, void (*logger) (int, const char *, const char *))
{
    if (core == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto cr = getCore (core);
        if (cr == nullptr)
        {
            return helics_invalid_object;
        }
        cr->setLoggingCallback (helics::local_core_id, [logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateAddLoggingCallback (helics_federate fed, void (*logger) (int, const char *, const char *))
{
    if (fed == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto fedptr = getFed (fed);
        if (fedptr == nullptr)
        {
            return helics_invalid_object;
        }
        fedptr->setLoggingCallback ([logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
