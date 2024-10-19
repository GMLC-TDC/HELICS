/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ConnectorFederateManager.hpp"

#include "../core/Core.hpp"
#include "../core/EmptyCore.hpp"
#include "Federate.hpp"
#include "helics/core/core-exceptions.hpp"

#include <memory>
#include <utility>

namespace helics {
ConnectorFederateManager::ConnectorFederateManager(Core* coreObj,
                                                   Federate* ffed,
                                                   LocalFederateId fid,
                                                   bool singleThreaded):
    coreObject(coreObj), filters(!singleThreaded), translators(!singleThreaded), fed(ffed),
    fedID(fid)
{
}
ConnectorFederateManager::~ConnectorFederateManager() = default;

Filter& ConnectorFederateManager::registerFilter(std::string_view name,
                                                 std::string_view type_in,
                                                 std::string_view type_out)
{
    auto handle = coreObject->registerFilter(name, type_in, type_out);
    if (handle.isValid()) {
        auto filtPtr = std::make_unique<Filter>(fed, name, handle);
        Filter& filt = *filtPtr;
        auto filts = filters.lock();
        if (name.empty()) {
            filts->insert(coreObject->getHandleName(filtPtr->getHandle()), std::move(filtPtr));
        } else {
            filts->insert(name, std::move(filtPtr));
        }
        return filt;
    }
    throw(RegistrationFailure("Unable to register Filter"));
}

Translator& ConnectorFederateManager::registerTranslator(std::string_view name,
                                                         std::string_view endpointType,
                                                         std::string_view units)
{
    auto handle = coreObject->registerTranslator(name, endpointType, units);
    if (handle.isValid()) {
        auto trans = translators.lock();
        if (name.empty()) {
            trans->insert(coreObject->getHandleName(handle), fed, name, handle);
        } else {
            trans->insert(name, fed, name, handle);
        }
        return trans->back();
    }
    throw(RegistrationFailure("Unable to register translator"));
}

CloningFilter& ConnectorFederateManager::registerCloningFilter(std::string_view name,
                                                               std::string_view type_in,
                                                               std::string_view type_out)
{
    auto handle = coreObject->registerCloningFilter(name, type_in, type_out);
    if (handle.isValid()) {
        auto filtPtr = std::make_unique<CloningFilter>(fed, name, handle);
        CloningFilter& filt = *filtPtr;
        auto filts = filters.lock();
        if (name.empty()) {
            filts->insert(coreObject->getHandleName(handle), std::move(filtPtr));
        } else {
            filts->insert(name, std::move(filtPtr));
        }
        return filt;
    }
    throw(RegistrationFailure("Unable to register Filter"));
}

Filter& ConnectorFederateManager::registerFilter(FilterTypes type, std::string_view name)
{
    return make_filter(type, fed, name);
}

CloningFilter& ConnectorFederateManager::registerCloningFilter(FilterTypes type,
                                                               std::string_view name)
{
    return make_cloning_filter(type, fed, std::string_view(), name);
}

static const Filter invalidFilt{};
static Filter invalidFiltNC{};

static const Translator invalidTran{};
static Translator invalidTranNC{};

Filter& ConnectorFederateManager::getFilter(std::string_view name)
{
    auto filts = filters.lock();
    auto filt = filts->find(name);
    return (filt != filts.end()) ? (**filt) : invalidFiltNC;
}
const Filter& ConnectorFederateManager::getFilter(std::string_view name) const
{
    auto sharedFilt = filters.lock_shared();
    auto filt = sharedFilt->find(name);
    return (filt != sharedFilt.end()) ? (**filt) : invalidFilt;
}

Filter& ConnectorFederateManager::getFilter(int index)
{
    auto sharedFilt = filters.lock();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFiltNC;
}

const Filter& ConnectorFederateManager::getFilter(int index) const
{
    auto sharedFilt = filters.lock_shared();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFilt;
}

int ConnectorFederateManager::getFilterCount() const
{
    return static_cast<int>(filters.lock_shared()->size());
}

Translator& ConnectorFederateManager::getTranslator(std::string_view name)
{
    auto trans = translators.lock();
    auto tran = trans->find(name);
    return (tran != trans.end()) ? (*tran) : invalidTranNC;
}
const Translator& ConnectorFederateManager::getTranslator(std::string_view name) const
{
    auto sharedTran = translators.lock_shared();
    auto tran = sharedTran->find(name);
    return (tran != sharedTran.end()) ? (*tran) : invalidTran;
}

Translator& ConnectorFederateManager::getTranslator(int index)
{
    auto sharedTran = translators.lock();
    if (isValidIndex(index, *sharedTran)) {
        return (*sharedTran)[index];
    }
    return invalidTranNC;
}

const Translator& ConnectorFederateManager::getTranslator(int index) const
{
    auto sharedTran = translators.lock_shared();
    if (isValidIndex(index, *sharedTran)) {
        return (*sharedTran)[index];
    }
    return invalidTran;
}

int ConnectorFederateManager::getTranslatorCount() const
{
    return static_cast<int>(translators.lock_shared()->size());
}
void ConnectorFederateManager::closeAllConnectors()
{
    if (coreObject != nullptr) {
        auto filts = filters.lock();
        filts->modify([this](auto& filt) {
            coreObject->closeHandle(filt->getHandle());
            filt->disconnectFromCore();
        });

        auto trans = translators.lock();
        trans->modify([this](auto& tran) {
            coreObject->closeHandle(tran.getHandle());
            tran.disconnectFromCore();
        });
    }
}

void ConnectorFederateManager::disconnectAllConnectors()
{
    auto filts = filters.lock();
    filts->modify([](auto& filt) { filt->disconnectFromCore(); });

    auto trans = translators.lock();
    trans->modify([](auto& tran) { tran.disconnectFromCore(); });
}

static EmptyCore eCore;

void ConnectorFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = &eCore;
}

}  // namespace helics
