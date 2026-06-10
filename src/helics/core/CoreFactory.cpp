/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#define ENABLE_TRIPWIRE

#include "CoreFactory.hpp"

#include "CommonCore.hpp"
#include "CoreTypes.hpp"
#include "EmptyCore.hpp"
#include "core-exceptions.hpp"
#include "coreTypeOperations.hpp"
#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"
#include "gmlc/libguarded/shared_guarded.hpp"
#include "helics/helics-config.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

// NOLINTNEXTLINE
DECLARE_TRIPLINE()

namespace helics::CoreFactory {

static constexpr std::string_view gHelicsEmptyString;

namespace {
    bool debugCleanupEnabled()
    {
        static const bool enabled = []() {
            const auto* env = std::getenv("HELICS_DEBUG_CLEANUP");
            return env != nullptr && env[0] != '\0' && env[0] != '0';
        }();
        return enabled;
    }

    void cleanupTrace(std::string_view stage, std::string_view identifier = {})
    {
        if (!debugCleanupEnabled()) {
            return;
        }
        std::cerr << "[helics-cleanup][core] " << stage;
        if (!identifier.empty()) {
            std::cerr << " id=" << identifier;
        }
        std::cerr << " tid=" << std::this_thread::get_id() << '\n';
    }

    void destroyCoreFirst(std::shared_ptr<Core>& core);
    gmlc::concurrency::TripWireTrigger& tripTriggerInstance();

    /*** class to hold the set of builders
    @details this doesn't work as a global since it tends to get initialized after some of the
    things that call it so it needs to be a static member of function call*/
    class MasterCoreBuilder {
      public:
        using BuilderData = std::tuple<int, std::string, std::shared_ptr<CoreBuilder>>;

        static void addBuilder(std::shared_ptr<CoreBuilder> builder,
                               std::string_view name,
                               int code)
        {
            instance()->builders.emplace_back(code, name, std::move(builder));
        }
        static const std::shared_ptr<CoreBuilder>& getBuilder(int code)
        {
            for (auto& builder : instance()->builders) {
                if (std::get<0>(builder) == code) {
                    return std::get<2>(builder);
                }
            }
            throw(HelicsException("core type is not available"));
        }
        static const std::shared_ptr<CoreBuilder>& getIndexedBuilder(std::size_t index)
        {
            const auto& blder = instance();
            if (blder->builders.size() <= index) {
                throw(HelicsException("core type index is not available"));
            }
            return std::get<2>(blder->builders[index]);
        }
        static const std::shared_ptr<MasterCoreBuilder>& instance()
        {
            static const std::shared_ptr<MasterCoreBuilder> iptr(new MasterCoreBuilder());
            return iptr;
        }
        static size_t size()
        {
            const auto& blder = instance();
            return blder->builders.size();
        }
        static const std::string& getIndexedBuilderName(std::size_t index)
        {
            const auto& blder = instance();
            if (blder->builders.size() <= index) {
                throw(HelicsException("core type index is not available"));
            }
            return std::get<1>(blder->builders[index]);
        }

      private:
        /** private constructor since we only really want one of them
        accessed through the instance static member*/
        MasterCoreBuilder() = default;
        std::vector<BuilderData> builders;  //!< container for the different builders
    };

    std::shared_ptr<Core>& emptyCoreInstance()
    {
        static auto* emptyCore = new std::shared_ptr<Core>(std::make_shared<EmptyCore>());
        return *emptyCore;
    }

    gmlc::concurrency::DelayedDestructor<Core>& delayedDestroyerInstance()
    {
        (void)tripTriggerInstance();
        static auto* destroyer = new gmlc::concurrency::DelayedDestructor<Core>(destroyCoreFirst);
        return *destroyer;
    }

    gmlc::concurrency::SearchableObjectHolder<Core, CoreType>& searchableCoresInstance()
    {
        static auto* searchableCores =
            new gmlc::concurrency::SearchableObjectHolder<Core, CoreType>();
        return *searchableCores;
    }

    gmlc::concurrency::TripWireTrigger& tripTriggerInstance()
    {
        static auto* tripTrigger = new gmlc::concurrency::TripWireTrigger();
        return *tripTrigger;
    }

    std::shared_ptr<Core> makeCore(CoreType type, std::string_view name)
    {
        if (type == CoreType::NULLCORE) {
            throw(HelicsException("nullcore is explicitly not available nor will ever be"));
        }
        if (type == CoreType::DEFAULT) {
            return MasterCoreBuilder::getIndexedBuilder(0)->build(name);
        }
        if (type == CoreType::EMPTY) {
            return emptyCoreInstance();
        }
        return MasterCoreBuilder::getBuilder(static_cast<int>(type))->build(name);
    }

    std::shared_ptr<Core> create(std::string_view initializationString)
    {
        return create(CoreType::EXTRACT, gHelicsEmptyString, initializationString);
    }

    void destroyCoreFirst(std::shared_ptr<Core>& core)
    {
        auto* ccore = dynamic_cast<CommonCore*>(core.get());
        if (ccore != nullptr) {
            cleanupTrace("destroyer start", ccore->getIdentifier());
            ccore->processDisconnect(true);
            cleanupTrace("destroyer joining", ccore->getIdentifier());
            ccore->joinAllThreads();
            cleanupTrace("destroyer complete", ccore->getIdentifier());
        }
    }
}  // namespace

void defineCoreBuilder(std::shared_ptr<CoreBuilder> builder, std::string_view name, int code)
{
    MasterCoreBuilder::addBuilder(std::move(builder), name, code);
}

std::vector<std::string> getAvailableCoreTypes()
{
    std::vector<std::string> availableCores;
    auto builderCount = MasterCoreBuilder::size();
    availableCores.reserve(builderCount);
    for (size_t ii = 0; ii < builderCount; ++ii) {
        availableCores.push_back(MasterCoreBuilder::getIndexedBuilderName(ii));
    }
    return availableCores;
}

std::shared_ptr<Core> getEmptyCore()
{
    return emptyCoreInstance();
}

Core* getEmptyCorePtr()
{
    static EmptyCore eCore;
    return &eCore;
}

std::shared_ptr<Core> create(CoreType type, std::string_view configureString)
{
    return create(type, gHelicsEmptyString, configureString);
}

std::shared_ptr<Core>
    create(CoreType type, std::string_view coreName, std::string_view configureString)
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || coreName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(std::string{configureString});
        if (coreName.empty() && !newName.empty()) {
            coreName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }
    auto core = makeCore(type, coreName);
    if (!core) {
        throw(helics::RegistrationFailure("unable to create core"));
    }
    core->configure(configureString);
    if (!registerCore(core, type)) {
        throw(helics::RegistrationFailure(
            fmt::format("core {} failed to register properly", core->getIdentifier())));
    }

    return core;
}

std::shared_ptr<Core> create(std::vector<std::string> args)
{
    return create(CoreType::EXTRACT, gHelicsEmptyString, std::move(args));
}

std::shared_ptr<Core> create(CoreType type, std::vector<std::string> args)
{
    return create(type, gHelicsEmptyString, std::move(args));
}

std::shared_ptr<Core>
    create(CoreType type, std::string_view coreName, std::vector<std::string> args)
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || coreName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(args);
        if (coreName.empty() && !newName.empty()) {
            coreName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }

    auto core = makeCore(type, coreName);
    core->configureFromVector(std::move(args));
    if (!registerCore(core, type)) {
        throw(helics::RegistrationFailure(
            fmt::format("core {} failed to register properly", core->getIdentifier())));
    }

    return core;
}

std::shared_ptr<Core> create(int argc, char* argv[])
{
    return create(CoreType::EXTRACT, gHelicsEmptyString, argc, argv);
}

std::shared_ptr<Core> create(CoreType type, int argc, char* argv[])
{
    return create(type, gHelicsEmptyString, argc, argv);
}

std::shared_ptr<Core> create(CoreType type, std::string_view coreName, int argc, char* argv[])
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || coreName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(argc, argv);
        if (coreName.empty() && !newName.empty()) {
            coreName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }
    auto core = makeCore(type, coreName);
    core->configureFromArgs(argc, argv);
    if (!registerCore(core, type)) {
        throw(helics::RegistrationFailure(std::string("core ") + core->getIdentifier() +
                                          " failed to register properly"));
    }

    return core;
}

std::shared_ptr<Core>
    FindOrCreate(CoreType type, std::string_view coreName, std::vector<std::string> args)
{
    std::shared_ptr<Core> core = findCore(coreName);
    if (core) {
        return core;
    }
    core = makeCore(type, coreName);
    core->configureFromVector(std::move(args));

    if (!registerCore(core, type)) {
        core = findCore(coreName);
        if (core) {
            return core;
        }
    }

    return core;
}

std::shared_ptr<Core>
    FindOrCreate(CoreType type, std::string_view coreName, std::string_view configureString)
{
    std::shared_ptr<Core> core = findCore(coreName);
    if (core) {
        return core;
    }
    core = makeCore(type, coreName);
    core->configure(configureString);

    if (!registerCore(core, type)) {
        core = findCore(coreName);
        if (core) {
            return core;
        }
    }

    return core;
}

std::shared_ptr<Core> FindOrCreate(CoreType type, std::string_view coreName, int argc, char* argv[])
{
    std::shared_ptr<Core> core = findCore(coreName);
    if (core) {
        return core;
    }
    core = makeCore(type, coreName);

    core->configureFromArgs(argc, argv);
    if (!registerCore(core, type)) {
        core = findCore(coreName);
        if (core) {
            return core;
        }
    }

    return core;
}

/** so the problem this is addressing is that unregister can potentially cause a destructor to
fire that destructor can delete a thread variable, unfortunately it is possible that a thread stored
in this variable can do the unregister operation and destroy itself meaning it is unable to join and
thus will call std::terminate what we do is delay the destruction until it is called in a different
thread which allows the destructor to fire if need be without issue*/
std::shared_ptr<Core> findCore(std::string_view name)
{
    return searchableCoresInstance().findObject(std::string{name});
}

std::shared_ptr<Core> findJoinableCoreOfType(CoreType type)
{
    return searchableCoresInstance().findObject(
        [](auto& ptr) { return ptr->isOpenToNewFederates(); }, type);
}

static void addExtraTypes(const std::string& name, CoreType type)
{
    switch (type) {
        case CoreType::INPROC:
            searchableCoresInstance().addType(name, CoreType::TEST);
            break;
        case CoreType::TEST:
            searchableCoresInstance().addType(name, CoreType::INPROC);
            break;
        case CoreType::IPC:
            searchableCoresInstance().addType(name, CoreType::INTERPROCESS);
            break;
        case CoreType::INTERPROCESS:
            searchableCoresInstance().addType(name, CoreType::IPC);
            break;
        default:
            break;
    }
}

bool registerCore(const std::shared_ptr<Core>& core, CoreType type)
{
    bool res = false;
    const std::string& cname = core ? core->getIdentifier() : std::string{};
    if (core) {
        res = searchableCoresInstance().addObject(cname, core, type);
    }
    if (res) {
        delayedDestroyerInstance().addObjectsToBeDestroyed(core);
        addExtraTypes(cname, type);
    }
    return res;
}

size_t cleanUpCores()
{
    cleanupTrace("cleanUpCores start");
    auto count = delayedDestroyerInstance().destroyObjects();
    if (debugCleanupEnabled()) {
        std::cerr << "[helics-cleanup][core] cleanUpCores complete destroyed=" << count
                  << " tid=" << std::this_thread::get_id() << '\n';
    }
    return count;
}

size_t cleanUpCores(std::chrono::milliseconds delay)
{
    cleanupTrace("cleanUpCores delayed start");
    auto count = delayedDestroyerInstance().destroyObjects(delay);
    if (debugCleanupEnabled()) {
        std::cerr << "[helics-cleanup][core] cleanUpCores delayed complete destroyed=" << count
                  << " tid=" << std::this_thread::get_id() << '\n';
    }
    return count;
}

void terminateAllCores()
{
    cleanupTrace("terminateAllCores start");
    auto cores = searchableCoresInstance().getObjects();
    for (auto& core : cores) {
        cleanupTrace("terminateAllCores disconnect", core->getIdentifier());
        core->disconnect();
    }
    cleanUpCores(std::chrono::milliseconds(250));
    cleanupTrace("terminateAllCores complete");
}

void abortAllCores(int errorCode, std::string_view errorString)
{
    auto cores = searchableCoresInstance().getObjects();
    for (auto& core : cores) {
        core->globalError(gLocalCoreId,
                          errorCode,
                          fmt::format("{} sent abort message: '{}'",
                                      core->getIdentifier(),
                                      errorString));

        core->disconnect();
    }
    cleanUpCores(std::chrono::milliseconds(250));
}

size_t getCoreCount()
{
    return searchableCoresInstance().getObjects().size();
}
bool copyCoreIdentifier(std::string_view copyFromName, std::string_view copyToName)
{
    return searchableCoresInstance().copyObject(std::string{copyFromName},
                                                std::string{copyToName});
}

void unregisterCore(std::string_view name)
{
    if (!searchableCoresInstance().removeObject(std::string{name})) {
        searchableCoresInstance().removeObject(
            [&name](auto& obj) { return (obj->getIdentifier() == name); });
    }
}

void addAssociatedCoreType(std::string_view name, CoreType type)
{
    searchableCoresInstance().addType(std::string{name}, type);
    addExtraTypes(std::string{name}, type);
}

static constexpr std::string_view helpStr{"--help"};

void displayHelp(CoreType type)
{
    if (type == CoreType::DEFAULT || type == CoreType::UNRECOGNIZED) {
        std::cout << "All core types have similar options\n";
        auto core = makeCore(CoreType::DEFAULT, gHelicsEmptyString);
        core->configure(helpStr);
#ifdef HELICS_ENABLE_TCP_CORE
        core = makeCore(CoreType::TCP_SS, gHelicsEmptyString);
        core->configure(helpStr);
#endif
    } else {
        auto core = makeCore(type, gHelicsEmptyString);
        core->configure(helpStr);
    }
}
}  // namespace helics::CoreFactory
