/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsApps.h"
#include "internal/api_objects.h"

static constexpr const char* invalidAppString = "app object is not valid";
static constexpr const char* notLoadedString = "helics apps not compiled into library, enable apps when building";

helics::apps::App* getApp(HelicsApp app, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (app == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidAppString);
    } else {
        assignError(err, HELICS_ERROR_OTHER, notLoadedString);
    }
    return nullptr;
}

static constexpr char nullcstr[] = "";

HelicsBool helicsAppEnabled()
{
    return HELICS_FALSE;
}

HelicsApp helicsCreateApp(const char* /*appName*/,
                          const char* /*appType*/,
                          const char* /*configFile*/,
                          HelicsFederateInfo /*fedInfo*/,
                          HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    assignError(err, HELICS_ERROR_OTHER, notLoadedString);
    return nullptr;
}

HelicsFederate helicsAppGetFederate(HelicsApp app, HelicsError* err)
{
    getApp(app, err);
    return nullptr;
}

void helicsAppLoadFile(HelicsApp app, const char* /*configFile*/, HelicsError* err)
{
    getApp(app, err);
}

void helicsAppInitialize(HelicsApp app, HelicsError* err)
{
    getApp(app, err);
}

void helicsAppRun(HelicsApp app, HelicsError* err)
{
    getApp(app, err);
}

void helicsAppRunTo(HelicsApp app, HelicsTime stopTime, HelicsError* err)
{
    getApp(app, err);
}

void helicsAppFinalize(HelicsApp app, HelicsError* err)
{
    getApp(app, err);
}

HelicsBool helicsAppIsActive(HelicsApp app)
{
    return HELICS_FALSE;
}
