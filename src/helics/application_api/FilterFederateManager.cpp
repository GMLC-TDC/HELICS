/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterFederateManager.hpp"

#include "../core/Core.hpp"
#include "Federate.hpp"
#include "helics/core/core-exceptions.hpp"

#include <utility>

namespace helics {
FilterFederateManager::FilterFederateManager(Core* coreObj, Federate* ffed, local_federate_id id):
    coreObject(coreObj), fed(ffed), fedID(id)
{
}
FilterFederateManager::~FilterFederateManager() = default;

Filter& FilterFederateManager::registerFilter(const std::string& name,
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

CloningFilter& FilterFederateManager::registerCloningFilter(const std::string& name,
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

Filter& FilterFederateManager::registerFilter(filter_types type, const std::string& name)
{
    return make_filter(type, fed, name);
}

CloningFilter& FilterFederateManager::registerCloningFilter(filter_types type,
                                                            const std::string& name)
{
    return make_cloning_filter(type, fed, std::string(), name);
}

static const Filter invalidFilt{};
static Filter invalidFiltNC{};

Filter& FilterFederateManager::getFilter(const std::string& name)
{
    auto filts = filters.lock();
    auto filt = filts->find(name);
    return (filt != filts.end()) ? (**filt) : invalidFiltNC;
}
const Filter& FilterFederateManager::getFilter(const std::string& name) const
{
    auto sharedFilt = filters.lock_shared();
    auto filt = sharedFilt->find(name);
    return (filt != sharedFilt.end()) ? (**filt) : invalidFilt;
}

Filter& FilterFederateManager::getFilter(int index)
{
    auto sharedFilt = filters.lock();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFiltNC;
}

const Filter& FilterFederateManager::getFilter(int index) const
{
    auto sharedFilt = filters.lock_shared();
    if (isValidIndex(index, *sharedFilt)) {
        return *(*sharedFilt)[index];
    }
    return invalidFilt;
}

int FilterFederateManager::getFilterCount() const
{
    return static_cast<int>(filters.lock_shared()->size());
}

void FilterFederateManager::closeAllFilters()
{
    if (coreObject != nullptr) {
        auto filts = filters.lock();
        for (auto& filt : filts) {
            coreObject->closeHandle(filt->getHandle());
        }
    }
}
}  // namespace helics
