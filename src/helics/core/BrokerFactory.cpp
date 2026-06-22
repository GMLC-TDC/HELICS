/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#define ENABLE_TRIPWIRE

#include "BrokerFactory.hpp"

#include "CoreBroker.hpp"
#include "CoreTypes.hpp"
#include "core-exceptions.hpp"
#include "coreTypeOperations.hpp"
#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics/helics-config.h"

#include <cassert>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

namespace helics::BrokerFactory {

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
        std::cerr << "[helics-cleanup][broker] " << stage;
        if (!identifier.empty()) {
            std::cerr << " id=" << identifier;
        }
        std::cerr << " tid=" << std::this_thread::get_id() << '\n';
    }

    void destroyBrokerFirst(std::shared_ptr<Broker>& broker);
    gmlc::concurrency::TripWireTrigger& tripTriggerInstance();

    /*** class to hold the set of builders
    @details this doesn't work as a global since it tends to get initialized after some of the
    things that call it so it needs to be a static member of function call*/
    class MasterBrokerBuilder {
      public:
        using BuilderData = std::tuple<int, std::string, std::shared_ptr<BrokerBuilder>>;

        static void
            addBuilder(std::shared_ptr<BrokerBuilder> builder, std::string_view name, int code)
        {
            instance()->builders.emplace_back(code, name, std::move(builder));
        }
        static const std::shared_ptr<BrokerBuilder>& getBuilder(int code)
        {
            for (auto& builder : instance()->builders) {
                if (std::get<0>(builder) == code) {
                    return std::get<2>(builder);
                }
            }
            throw(HelicsException("core type is not available"));
        }
        static const std::shared_ptr<BrokerBuilder>& getIndexedBuilder(std::size_t index)
        {
            const auto& blder = instance();
            if (blder->builders.size() <= index) {
                throw(HelicsException("broker type index is not available"));
            }
            return std::get<2>(blder->builders[index]);
        }

        static const std::shared_ptr<BrokerBuilder>& getDefaultBuilder()
        {
            const auto& blder = instance();
            for (auto& builder : instance()->builders) {
                if (std::get<0>(builder) <= 10) {
                    return std::get<2>(builder);
                }
            }
            if (blder->builders.empty()) {
                throw(HelicsException("core type is not available"));
            }
            return std::get<2>(blder->builders[0]);
        }

        static const std::shared_ptr<MasterBrokerBuilder>& instance()
        {
            static const std::shared_ptr<MasterBrokerBuilder> iptr(new MasterBrokerBuilder());
            return iptr;
        }

      private:
        /** private constructor since we only really want one of them
        accessed through the instance static member*/
        MasterBrokerBuilder() = default;
        std::vector<BuilderData> builders;  //!< container for the builders
    };

    gmlc::concurrency::DelayedDestructor<Broker>& delayedDestroyerInstance()
    {
        (void)tripTriggerInstance();
        static auto* destroyer =
            new gmlc::concurrency::DelayedDestructor<Broker>(destroyBrokerFirst);
        return *destroyer;
    }

    gmlc::concurrency::SearchableObjectHolder<Broker, CoreType>& searchableBrokersInstance()
    {
        static auto* searchableBrokers =
            new gmlc::concurrency::SearchableObjectHolder<Broker, CoreType>();
        return *searchableBrokers;
    }

    gmlc::concurrency::TripWireTrigger& tripTriggerInstance()
    {
        static auto* tripTrigger = new gmlc::concurrency::TripWireTrigger();
        return *tripTrigger;
    }

    std::shared_ptr<Broker> makeBroker(CoreType type, std::string_view name)
    {
        if (type == CoreType::NULLCORE) {
            throw(HelicsException("nullcore is explicitly not available nor will ever be"));
        }
        if (type == CoreType::DEFAULT) {
            return MasterBrokerBuilder::getDefaultBuilder()->build(name);
        }
        return MasterBrokerBuilder::getBuilder(static_cast<int>(type))->build(name);
    }

    void destroyBrokerFirst(std::shared_ptr<Broker>& broker)
    {
        auto tbroker = std::dynamic_pointer_cast<CoreBroker>(broker);
        if (tbroker) {
            cleanupTrace("destroyer start", tbroker->getIdentifier());
            tbroker->processDisconnect(true);  // use true here as it is possible the
                                               // searchableObjectHolder is deleted already
            cleanupTrace("destroyer joining", tbroker->getIdentifier());
            tbroker->joinAllThreads();
            cleanupTrace("destroyer complete", tbroker->getIdentifier());
        }
    }
}  // namespace

void defineBrokerBuilder(std::shared_ptr<BrokerBuilder> builder, std::string_view name, int code)
{
    MasterBrokerBuilder::addBuilder(std::move(builder), name, code);
}

std::shared_ptr<Broker> create(CoreType type, std::string_view configureString)
{
    static constexpr std::string_view emptyString{nullptr, 0};
    return create(type, emptyString, configureString);
}

std::shared_ptr<Broker>
    create(CoreType type, std::string_view brokerName, std::string_view configureString)
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || brokerName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(std::string{configureString});
        if (brokerName.empty() && !newName.empty()) {
            brokerName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }
    auto broker = makeBroker(type, brokerName);
    if (!broker) {
        throw(helics::RegistrationFailure("unable to create broker"));
    }
    broker->configure(configureString);
    if (!registerBroker(broker, type)) {
        throw(helics::RegistrationFailure("unable to register broker"));
    }
    broker->connect();
    return broker;
}

std::shared_ptr<Broker> create(CoreType type, int argc, char* argv[])
{
    static const std::string emptyString;
    return create(type, emptyString, argc, argv);
}

std::shared_ptr<Broker> create(CoreType type, std::string_view brokerName, int argc, char* argv[])
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || brokerName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(argc, argv);
        if (brokerName.empty() && !newName.empty()) {
            brokerName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }
    auto broker = makeBroker(type, brokerName);
    broker->configureFromArgs(argc, argv);
    if (!registerBroker(broker, type)) {
        throw(helics::RegistrationFailure("unable to register broker"));
    }
    broker->connect();
    return broker;
}

std::shared_ptr<Broker> create(CoreType type, std::vector<std::string> args)
{
    static const std::string emptyString;
    return create(type, emptyString, std::move(args));
}

std::shared_ptr<Broker>
    create(CoreType type, std::string_view brokerName, std::vector<std::string> args)
{
    std::string newName;
    CoreType newType;
    if (type == CoreType::EXTRACT || brokerName.empty()) {
        std::tie(newType, newName) = core::extractCoreType(args);
        if (brokerName.empty() && !newName.empty()) {
            brokerName = newName;
        }
        if (type == CoreType::EXTRACT) {
            type = newType;
        }
    }
    auto broker = makeBroker(type, brokerName);
    broker->configureFromVector(std::move(args));
    if (!registerBroker(broker, type)) {
        throw(helics::RegistrationFailure("unable to register broker"));
    }
    broker->connect();
    return broker;
}

/** so the problem this is addressing is that unregister can potentially cause a destructor to
fire that destructor can delete a thread variable, unfortunately it is possible that a thread stored
in this variable can do the unregister operation and destroy itself meaning it is unable to join and
thus will call std::terminate what we do is delay the destruction until it is called in a different
thread which allows the destructor to fire if need be without issue*/

std::shared_ptr<Broker> findBroker(std::string_view brokerName)
{
    auto brk = searchableBrokersInstance().findObject(std::string(brokerName));
    if (brk) {
        return brk;
    }
    if (brokerName.empty()) {
        return getConnectedBroker();
    }
    if (brokerName.front() == '#') {
        char* ept = nullptr;
        auto val = std::strtoull(brokerName.data() + 1, &ept, 10);
        return (ept > brokerName.data() + 1) ? getBrokerByIndex(val) : nullptr;
    }
    return nullptr;
}

std::shared_ptr<Broker> getConnectedBroker()
{
    return searchableBrokersInstance().findObject([](auto& ptr) { return ptr->isConnected(); });
}

std::shared_ptr<Broker> getBrokerByIndex(size_t index)
{
    auto brks = searchableBrokersInstance().getObjects();
    return brks.size() > index ? brks[index] : nullptr;
}

std::shared_ptr<Broker> findJoinableBrokerOfType(CoreType type)
{
    return searchableBrokersInstance().findObject(
        [](auto& ptr) { return ptr->isOpenToNewFederates(); }, type);
}

std::vector<std::shared_ptr<Broker>> getAllBrokers()
{
    return searchableBrokersInstance().getObjects();
}

bool brokersActive()
{
    return !searchableBrokersInstance().empty();
}

static void addExtraTypes(const std::string& name, CoreType type)
{
    switch (type) {
        case CoreType::INPROC:
            searchableBrokersInstance().addType(name, CoreType::TEST);
            break;
        case CoreType::TEST:
            searchableBrokersInstance().addType(name, CoreType::INPROC);
            break;
        case CoreType::IPC:
            searchableBrokersInstance().addType(name, CoreType::INTERPROCESS);
            break;
        case CoreType::INTERPROCESS:
            searchableBrokersInstance().addType(name, CoreType::IPC);
            break;
        default:
            break;
    }
}
bool registerBroker(const std::shared_ptr<Broker>& broker, CoreType type)
{
    bool registered = false;
    const std::string& bname = broker ? broker->getIdentifier() : std::string{};
    if (broker) {
        registered = searchableBrokersInstance().addObject(bname, broker, type);
    }
    if (!registered && broker) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cleanUpBrokers(std::chrono::milliseconds(200));
        registered = searchableBrokersInstance().addObject(bname, broker, type);
    }
    if (registered) {
        delayedDestroyerInstance().addObjectsToBeDestroyed(broker);
        addExtraTypes(bname, type);
    }

    return registered;
}

size_t cleanUpBrokers()
{
    cleanupTrace("cleanUpBrokers start");
    auto count = delayedDestroyerInstance().destroyObjects();
    if (debugCleanupEnabled()) {
        std::cerr << "[helics-cleanup][broker] cleanUpBrokers complete destroyed=" << count
                  << " tid=" << std::this_thread::get_id() << '\n';
    }
    return count;
}
size_t cleanUpBrokers(std::chrono::milliseconds delay)
{
    cleanupTrace("cleanUpBrokers delayed start");
    auto count = delayedDestroyerInstance().destroyObjects(delay);
    if (debugCleanupEnabled()) {
        std::cerr << "[helics-cleanup][broker] cleanUpBrokers delayed complete destroyed=" << count
                  << " tid=" << std::this_thread::get_id() << '\n';
    }
    return count;
}

void terminateAllBrokers()
{
    cleanupTrace("terminateAllBrokers start");
    auto brokers = getAllBrokers();
    for (auto& brk : brokers) {
        cleanupTrace("terminateAllBrokers disconnect", brk->getIdentifier());
        brk->disconnect();
    }
    cleanUpBrokers(std::chrono::milliseconds(250));
    cleanupTrace("terminateAllBrokers complete");
}

void abortAllBrokers(int errorCode, std::string_view errorString)
{
    auto brokers = getAllBrokers();
    for (auto& brk : brokers) {
        brk->globalError(errorCode,
                         fmt::format("{} sent abort message: '{}'",
                                     brk->getIdentifier(),
                                     errorString));

        brk->disconnect();
    }
    cleanUpBrokers(std::chrono::milliseconds(250));
}
bool copyBrokerIdentifier(std::string_view copyFromName, std::string_view copyToName)
{
    return searchableBrokersInstance().copyObject(std::string(copyFromName),
                                                  std::string(copyToName));
}

void unregisterBroker(std::string_view name)
{
    if (!searchableBrokersInstance().removeObject(std::string(name))) {
        searchableBrokersInstance().removeObject(
            [&name](auto& obj) { return (obj->getIdentifier() == name); });
    }
}

void addAssociatedBrokerType(std::string_view name, CoreType type)
{
    const std::string sname{name};
    searchableBrokersInstance().addType(sname, type);
    addExtraTypes(sname, type);
}

static constexpr std::string_view helpStr{"--help"};

void displayHelp(CoreType type)
{
    if (type == CoreType::DEFAULT || type == CoreType::UNRECOGNIZED) {
        std::cout << "All core types have similar options\n";
        auto brk = makeBroker(CoreType::DEFAULT, std::string{});
        brk->configure(helpStr);
#ifdef HELICS_ENABLE_TCP_CORE
        brk = makeBroker(CoreType::TCP_SS, std::string{});
        brk->configure(helpStr);
#endif
    } else {
        auto brk = makeBroker(type, std::string{});
        brk->configure(helpStr);
    }
}

}  // namespace helics::BrokerFactory
