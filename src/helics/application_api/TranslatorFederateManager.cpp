/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TranslatorFederateManager.hpp"

#include "../core/Core.hpp"
#include "../core/EmptyCore.hpp"
#include "Federate.hpp"
#include "helics/core/core-exceptions.hpp"

#include <utility>

namespace helics {
TranslatorFederateManager::TranslatorFederateManager(Core* coreObj, Federate* ffed, LocalFederateId id):
    coreObject(coreObj), fed(ffed), fedID(id)
{
}
TranslatorFederateManager::~TranslatorFederateManager() = default;

Translator& TranslatorFederateManager::registerTranslator(const std::string& name,
                                              const std::string& type_in,
                                              const std::string& type_out)
{
    auto handle = coreObject->registerTranslator(name, type_in, type_out);
    if (handle.isValid()) {
        auto trans = std::make_unique<Translator>(fed, name, handle);
        Translator& f = *trans;
        auto filts = translators.lock();
        if (name.empty()) {
            filts->insert(coreObject->getHandleName(trans->getHandle()), std::move(trans));
        } else {
            filts->insert(name, std::move(trans));
        }
        return f;
    }
    throw(RegistrationFailure("Unable to register Translator"));
}



Translator& TranslatorFederateManager::registerTranslator(TranslatorTypes type, const std::string& name)
{
    return make_translator(type, fed, name);
}


static const Translator invalidFilt{};
static Translator invalidFiltNC{};

Translator& TranslatorFederateManager::getTranslator(const std::string& name)
{
    auto filts = translators.lock();
    auto trans = filts->find(name);
    return (trans != filts.end()) ? (**trans) : invalidFiltNC;
}
const Translator& TranslatorFederateManager::getTranslator(const std::string& name) const
{
    auto sharedFilt = translators.lock_shared();
    auto trans = sharedFilt->find(name);
    return (trans != sharedFilt.end()) ? (**trans) : invalidFilt;
}

Translator& TranslatorFederateManager::getTranslator(int index)
{
    auto sharedFilt = translators.lock();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFiltNC;
}

const Translator& TranslatorFederateManager::getTranslator(int index) const
{
    auto sharedFilt = translators.lock_shared();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFilt;
}

int TranslatorFederateManager::getTranslatorCount() const
{
    return static_cast<int>(translators.lock_shared()->size());
}

void TranslatorFederateManager::closeAllTranslators()
{
    if (coreObject != nullptr) {
        auto filts = translators.lock();
        for (auto& trans : filts) {
            coreObject->closeHandle(trans->getHandle());
            trans->disconnectFromCore();
        }
    }
}

void TranslatorFederateManager::disconnectAllTranslators()
{
    auto filts = translators.lock();
    for (auto& trans : filts) {
        trans->disconnectFromCore();
    }
}

static EmptyCore eCore;

void TranslatorFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = &eCore;
}

}  // namespace helics
