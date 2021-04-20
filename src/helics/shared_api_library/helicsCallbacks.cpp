/*
Copyright (c) 2017-2021,
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
#include <string>
#include <vector>

void helicsBrokerSetLoggingCallback(helics_broker broker,
                                    void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                    void* userdata,
                                    helics_error* err)
{
    auto* brk = getBroker(broker, err);
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
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsCoreSetLoggingCallback(helics_core core,
                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                  void* userdata,
                                  helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    try {
        if (logger == nullptr) {
            cr->setLoggingCallback(helics::local_core_id, {});
        } else {
            cr->setLoggingCallback(helics::local_core_id,
                                   [logger, userdata](int loglevel, const std::string& ident, const std::string& message) {
                                       logger(loglevel, ident.c_str(), message.c_str(), userdata);
                                   });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetLoggingCallback(helics_federate fed,
                                      void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                      void* userdata,
                                      helics_error* err)
{
    auto* fedptr = getFed(fed, err);
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
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetQueryCallback(helics_federate fed,
                                    void (*queryAnswer)(const char* query, int querySize, helics_query_buffer buffer, void* userdata),
                                    void* userdata,
                                    helics_error* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (queryAnswer == nullptr) {
            fedptr->setQueryCallback({});
        } else {
            fedptr->setQueryCallback([queryAnswer, userdata](const std::string& query) {
                std::string buffer(1, '>');
                queryAnswer(query.c_str(), static_cast<int>(query.size()), &buffer, userdata);
                buffer.pop_back();
                return buffer;
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsQueryBufferFill(helics_query_buffer buffer, const char* string, int stringSize, helics_error* err)
{
    static const char* invalidBuffer = "The given buffer is not valid";

    if (((err) != nullptr) && ((err)->error_code != 0)) {
        return;
    }
    if (buffer == nullptr) {
        assignError(err, helics_error_invalid_object, invalidBuffer);
        return;
    }

    auto* bufferStr = reinterpret_cast<std::string*>(buffer);
    if (bufferStr->empty() || bufferStr->back() != '>') {
        assignError(err, helics_error_invalid_object, invalidBuffer);
        return;
    }
    if (stringSize <= 0 || string == nullptr) {
        bufferStr->clear();
        bufferStr->push_back('>');
    }
    bufferStr->reserve(stringSize + 1);
    bufferStr->assign(string, string + stringSize);
    bufferStr->push_back('>');
}
