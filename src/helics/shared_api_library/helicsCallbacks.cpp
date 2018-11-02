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

void helicsBrokerAddLoggingCallback (helics_broker broker,
                                     void (*logger) (int loglevel, const char *identifier, const char *message),
                                     helics_error *err)
{
    auto brk = getBroker (broker,err);
    if (brk == nullptr)
    {
        return;
    }
    try
    {
        brk->setLoggingCallback ([logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsCoreAddLoggingCallback (helics_core core,
                                   void (*logger) (int loglevel, const char *identifier, const char *message),
                                   helics_error *err)
{
    auto cr = getCore (core,err);
    if (cr == nullptr)
    {
        return;
    }
    try
    {
        cr->setLoggingCallback (helics::local_core_id, [logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFederateAddLoggingCallback (helics_federate fed,
                                       void (*logger) (int loglevel, const char *identifier, const char *message),
                                       helics_error *err)
{
    auto fedptr = getFed (fed, err);
    if (fedptr == nullptr)
    {
        return;
    }
    try
    {
        fedptr->setLoggingCallback ([logger](int loglevel, const std::string &ident, const std::string &message) {
            logger (loglevel, ident.c_str (), message.c_str ());
        });
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}
