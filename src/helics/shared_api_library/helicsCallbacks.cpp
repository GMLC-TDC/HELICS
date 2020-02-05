/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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

void helicsBrokerSetLoggingCallback(
    helics_broker broker,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
    void* userdata,
    helics_error* err)
{
    auto brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    try {
        if (logger == nullptr) {
            brk->setLoggingCallback({});
        } else {
            brk->setLoggingCallback([logger, userdata](int loglevel, const std::string& ident, const std::string& message) {
                logger(loglevel, ident.c_str(), message.c_str(), userdata);
            });
        }
    }
    catch (...) { // LCOV_EXCL_LINE
        helicsErrorHandler(err); // LCOV_EXCL_LINE
    }
}

void helicsCoreSetLoggingCallback(
    helics_core core,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
    void* userdata,
    helics_error* err)
{
    auto cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    try {
        if (logger == nullptr) {
            cr->setLoggingCallback(helics::local_core_id, {});
        } else {
            cr->setLoggingCallback(
                helics::local_core_id, [logger, userdata](int loglevel, const std::string& ident, const std::string& message) {
                    logger(loglevel, ident.c_str(), message.c_str(), userdata);
                });
        }
    }
    catch (...) { // LCOV_EXCL_LINE
        helicsErrorHandler(err); // LCOV_EXCL_LINE
    }
}

void helicsFederateSetLoggingCallback(
    helics_federate fed,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
    void* userdata,
    helics_error* err)
{
    auto fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (logger == nullptr) {
            fedptr->setLoggingCallback({});
        } else {
            fedptr->setLoggingCallback([logger, userdata](int loglevel, const std::string& ident, const std::string& message) {
                logger(loglevel, ident.c_str(), message.c_str(), userdata);
            });
        }
    }
    catch (...) { // LCOV_EXCL_LINE
        helicsErrorHandler(err); // LCOV_EXCL_LINE
    }
}
