/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ConnectorFederateManager.hpp"

#include "../core/Core.hpp"
#include "../core/EmptyCore.hpp"
#include "Federate.hpp"
#include "helics/core/core-exceptions.hpp"

#include <utility>

namespace helics {
ConnectorFederateManager::ConnectorFederateManager(Core* coreObj, Federate* ffed, LocalFederateId id):
    coreObject(coreObj), fed(ffed), fedID(id)
{
}
ConnectorFederateManager::~ConnectorFederateManager() = default;

Filter& ConnectorFederateManager::registerFilter(const std::string& name,
                                              const std::string& type_in,
                                              const std::string& type_out)
{
    auto handle = coreObject->registerFilter(name, type_in, type_out);
    if (handle.isValid()) {
        auto filt = std::make_unique<Filter>(fed, name, handle);
        Filter& f = *filt;
        auto filts = filters.lock();
        if (name.empty()) {
            filts->insert(coreObject->getHandleName(filt->getHandle()), std::move(filt));
        } else {
            filts->insert(name, std::move(filt));
        }
        return f;
    }
    throw(RegistrationFailure("Unable to register Filter"));
}

Translator& ConnectorFederateManager::registerTranslator(std::string_view name,
                                                         std::string_view endpointType,
                                                         std::string_view units)
{
    auto handle = coreObject->registerTranslator(name, endpointType, units);
    if (handle.isValid()) {
        auto tran = std::make_unique<Translator>(fed, name, handle);
        Translator& t = *tran;
        auto trans = translators.lock();
        if (name.empty()) {
            trans->insert(coreObject->getHandleName(tran->getHandle()), std::move(tran));
        } else {
            trans->insert(std::string(name), std::move(tran));
        }
        return t;
    }
    throw(RegistrationFailure("Unable to register translator"));
}

CloningFilter& ConnectorFederateManager::registerCloningFilter(const std::string& name,
                                                            const std::string& type_in,
                                                            const std::string& type_out)
{
    auto handle = coreObject->registerCloningFilter(name, type_in, type_out);
    if (handle.isValid()) {
        auto filt = std::make_unique<CloningFilter>(fed, name, handle);
        CloningFilter& f = *filt;
        auto filts = filters.lock();
        if (name.empty()) {
            filts->insert(coreObject->getHandleName(filt->getHandle()), std::move(filt));
        } else {
            filts->insert(name, std::move(filt));
        }
        return f;
    }
    throw(RegistrationFailure("Unable to register Filter"));
}

Filter& ConnectorFederateManager::registerFilter(FilterTypes type, const std::string& name)
{
    return make_filter(type, fed, name);
}

Translator& ConnectorFederateManager::registerTranslator(TranslatorTypes type, std::string_view name)
{
    return make_translator(type, fed, name);
}

CloningFilter& ConnectorFederateManager::registerCloningFilter(FilterTypes type,
                                                            const std::string& name)
{
    return make_cloning_filter(type, fed, std::string(), name);
}

static const Filter invalidFilt{};
static Filter invalidFiltNC{};

static const Translator invalidTran{};
static Translator invalidTranNC{};

Filter& ConnectorFederateManager::getFilter(const std::string& name)
{
    auto filts = filters.lock();
    auto filt = filts->find(name);
    return (filt != filts.end()) ? (**filt) : invalidFiltNC;
}
const Filter& ConnectorFederateManager::getFilter(const std::string& name) const
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

Translator& ConnectorFederateManager::getTranslator(const std::string& name)
{
    auto trans = translators.lock();
    auto tran = trans->find(name);
    return (tran != trans.end()) ? (**tran) : invalidTranNC;
}
const Translator& ConnectorFederateManager::getTranslator(const std::string& name) const
{
    auto sharedTran = translators.lock_shared();
    auto tran = sharedTran->find(name);
    return (tran != sharedTran.end()) ? (**tran) : invalidTran;
}

Translator& ConnectorFederateManager::getTranslator(int index)
{
    auto sharedTran = translators.lock();
    if (isValidIndex(index, *sharedTran)) {
        return *(*sharedTran)[index];
    }
    return invalidTranNC;
}

const Translator& ConnectorFederateManager::getTranslator(int index) const
{
    auto sharedTran = translators.lock_shared();
    if (isValidIndex(index, *sharedTran)) {
        return *(*sharedTran)[index];
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
        for (auto& filt : filts) {
            coreObject->closeHandle(filt->getHandle());
            filt->disconnectFromCore();
        }

        auto trans = translators.lock();
        for (auto& tran : trans) {
            coreObject->closeHandle(tran->getHandle());
            tran->disconnectFromCore();
        }
    }
}

void ConnectorFederateManager::disconnectAllConnectors()
{
    auto filts = filters.lock();
    for (auto& filt : filts) {
        filt->disconnectFromCore();
    }
    auto trans = translators.lock();
    for (auto& tran : trans) {
        tran->disconnectFromCore();
    }
}

static EmptyCore eCore;

void ConnectorFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = &eCore;
}

}  // namespace helics
