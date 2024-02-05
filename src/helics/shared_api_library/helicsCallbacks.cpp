/*
Copyright (c) 2017-2024,
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

void helicsBrokerSetLoggingCallback(HelicsBroker broker,
                                    void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                    void* userdata,
                                    HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    try {
        if (logger == nullptr) {
            brk->setLoggingCallback({});
        } else {
            brk->setLoggingCallback([logger, userdata](int loglevel, std::string_view ident, std::string_view message) {
                const std::string identifier(ident);
                const std::string mess(message);
                logger(loglevel, identifier.c_str(), mess.c_str(), userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsCoreSetLoggingCallback(HelicsCore core,
                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                  void* userdata,
                                  HelicsError* err)
{
    auto* coreObj = getCore(core, err);
    if (coreObj == nullptr) {
        return;
    }
    try {
        if (logger == nullptr) {
            coreObj->setLoggingCallback(helics::gLocalCoreId, {});
        } else {
            coreObj->setLoggingCallback(helics::gLocalCoreId,
                                        [logger, userdata](int loglevel, std::string_view ident, std::string_view message) {
                                            const std::string identifier(ident);
                                            const std::string mess(message);
                                            logger(loglevel, identifier.c_str(), mess.c_str(), userdata);
                                        });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetLoggingCallback(HelicsFederate fed,
                                      void (*logger)(int loglevel, const char* identifier, const char* message, void* userdata),
                                      void* userdata,
                                      HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (logger == nullptr) {
            fedptr->setLoggingCallback({});
        } else {
            fedptr->setLoggingCallback([logger, userdata](int loglevel, std::string_view ident, std::string_view message) {
                const std::string identifier(ident);
                const std::string mess(message);
                logger(loglevel, identifier.c_str(), mess.c_str(), userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetQueryCallback(HelicsFederate fed,
                                    void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                    void* userdata,
                                    HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (queryAnswer == nullptr) {
            fedptr->setQueryCallback({});
        } else {
            fedptr->setQueryCallback([queryAnswer, userdata](std::string_view query) {
                std::string buffer(1, '>');
                queryAnswer(query.data(), static_cast<int>(query.size()), &buffer, userdata);
                buffer.pop_back();
                return buffer;
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetTimeUpdateCallback(HelicsFederate fed,
                                         void (*timeUpdate)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                         void* userdata,
                                         HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (timeUpdate == nullptr) {
            fedptr->setTimeUpdateCallback({});
        } else {
            fedptr->setTimeUpdateCallback([timeUpdate, userdata](helics::Time newTime, bool iterating) {
                timeUpdate(static_cast<HelicsTime>(newTime), iterating ? HELICS_TRUE : HELICS_FALSE, userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsQueryBufferFill(HelicsQueryBuffer buffer, const char* string, int stringSize, HelicsError* err)
{
    static constexpr const char* invalidBuffer = "The given buffer is not valid";

    if (((err) != nullptr) && ((err)->error_code != 0)) {
        return;
    }
    if (buffer == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidBuffer);
        return;
    }

    auto* bufferStr = reinterpret_cast<std::string*>(buffer);
    if (bufferStr->empty() || bufferStr->back() != '>') {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidBuffer);
        return;
    }
    if (stringSize <= 0 || string == nullptr) {
        bufferStr->clear();
        bufferStr->push_back('>');
        return;
    }
    bufferStr->reserve(static_cast<std::size_t>(stringSize) + 1);
    bufferStr->assign(string, string + stringSize);
    bufferStr->push_back('>');
}
