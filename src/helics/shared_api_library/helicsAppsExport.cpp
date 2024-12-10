/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../common/configFileHelpers.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../helics.hpp"
#include "api-data.h"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics/helics_apps.hpp"
#include "helicsApps.h"
#include "internal/api_objects.h"

#include <memory>
#include <utility>

namespace helics {

/** this is a random identifier put in place when the federate or core or broker gets created*/
static constexpr int appValidationIdentifier = 0x7A8F'1C4D;

static constexpr const char* invalidAppString = "app object is not valid";

AppObject* getAppObject(HelicsApp app, HelicsError* err) noexcept
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (app == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidAppString);
        return nullptr;
    }
    auto* appObj = reinterpret_cast<helics::AppObject*>(app);
    if (appObj->valid == appValidationIdentifier) {
        return appObj;
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidAppString);
    return nullptr;
}

}  // namespace helics

helics::apps::App* getApp(HelicsApp app, HelicsError* err)
{
    auto* appObj = helics::getAppObject(app, err);
    if (appObj == nullptr) {
        return nullptr;
    }
    return appObj->app.get();
}

std::shared_ptr<helics::apps::App> getAppSharedPtr(HelicsApp app, HelicsError* err)
{
    auto* appObj = helics::getAppObject(app, err);
    if (appObj == nullptr) {
        return nullptr;
    }
    return appObj->app;
}

namespace {
std::shared_ptr<helics::apps::App> buildApp(std::string_view type, std::string_view appName, helics::FederateInfo& fedInfo)
{
    if (type == "player") {
        return std::make_shared<helics::apps::Player>(appName, fedInfo);
    }
    if (type == "recorder") {
        return std::make_shared<helics::apps::Recorder>(appName, fedInfo);
    }
    if (type == "connector") {
        return std::make_shared<helics::apps::Connector>(appName, fedInfo);
    }
    if (type == "echo") {
        return std::make_shared<helics::apps::Echo>(appName, fedInfo);
    }
    if (type == "clone") {
        return std::make_shared<helics::apps::Clone>(appName, fedInfo);
    }
    if (type == "probe") {
        return std::make_shared<helics::apps::Probe>(appName, fedInfo);
    }
    if (type == "tracer") {
        return std::make_shared<helics::apps::Tracer>(appName, fedInfo);
    }
    if (type == "source") {
        return std::make_shared<helics::apps::Source>(appName, fedInfo);
    }
    return nullptr;
}
}  // namespace

HelicsBool helicsAppEnabled()
{
    return HELICS_TRUE;
}
HelicsApp helicsCreateApp(const char* appName, const char* appType, const char* configFile, HelicsFederateInfo fedInfo, HelicsError* err)
{
    static constexpr const char* invalidAppTypeString =
        "app type must be one of 'connector', 'source','recorder','player','echo','clone','probe','tracer'";
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }

    try {
        auto cstring = AS_STRING(configFile);
        auto app = std::make_unique<helics::AppObject>();
        app->valid = helics::appValidationIdentifier;
        auto nstring = AS_STRING_VIEW(appName);
        if (appType == nullptr) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidAppTypeString);
            return nullptr;
        }

        const std::string_view appTypeName(appType);
        bool loadFile = !cstring.empty();
        if (fedInfo == nullptr && helics::fileops::getConfigType(cstring) != helics::fileops::ConfigType::NONE) {
            helics::FederateInfo newFedInfo = helics::loadFederateInfo(cstring);
            app->app = buildApp(appTypeName, nstring, newFedInfo);
            loadFile = false;
        } else {
            auto* info = getFedInfo(fedInfo, err);
            if (info == nullptr) {
                return nullptr;
            }
            app->app = buildApp(appTypeName, nstring, *info);
        }

        if (!app->app) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidAppTypeString);
            return nullptr;
        }
        if (loadFile && !cstring.empty()) {
            app->app->loadFile(cstring);
        }
        auto* retapp = reinterpret_cast<HelicsApp>(app.get());
        getMasterHolder()->addApp(std::move(app));
        return retapp;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsFederate helicsAppGetFederate(HelicsApp app, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return nullptr;
    }
    try {
        return generateNewHelicsFederateObject(happ->getUnderlyingFederatePointer(), helics::FederateType::COMBINATION);
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

void helicsAppLoadFile(HelicsApp app, const char* configFile, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return;
    }
    try {
        happ->loadFile(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsAppInitialize(HelicsApp app, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return;
    }
    try {
        happ->initialize();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsAppRun(HelicsApp app, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return;
    }
    try {
        happ->run();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsAppRunTo(HelicsApp app, HelicsTime stopTime, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return;
    }
    try {
        happ->runTo(stopTime);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsAppFinalize(HelicsApp app, HelicsError* err)
{
    auto* happ = getApp(app, err);
    if (happ == nullptr) {
        return;
    }
    try {
        happ->finalize();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsAppFree(HelicsApp app)
{
    auto* appObj = helics::getAppObject(app, nullptr);
    if (appObj != nullptr) {
        appObj->valid = 0;
        getMasterHolder()->clearApp(appObj->index);
    }

    helics::CoreFactory::cleanUpCores();
}

void helicsAppDestroy(HelicsApp app)
{
    helicsAppFinalize(app, nullptr);
    helicsAppFree(app);
}

HelicsBool helicsAppIsActive(HelicsApp app)
{
    auto* happ = getApp(app, nullptr);
    if (happ == nullptr) {
        return HELICS_FALSE;
    }
    return (happ->isActive() ? HELICS_TRUE : HELICS_FALSE);
}
