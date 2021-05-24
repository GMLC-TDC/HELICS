/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#define ENABLE_TRIPWIRE

#include "BrokerFactory.hpp"

#include "CoreBroker.hpp"
#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics/helics-config.h"

#include <cassert>
#include <tuple>
#include <utility>

namespace helics {

namespace BrokerFactory {

    /*** class to hold the set of builders
    @details this doesn't work as a global since it tends to get initialized after some of the
    things that call it so it needs to be a static member of function call*/
    class MasterBrokerBuilder {
      public:
        using BuildT = std::tuple<int, std::string, std::shared_ptr<BrokerBuilder>>;

        static void addBuilder(std::shared_ptr<BrokerBuilder> cb, const std::string& name, int code)
        {
            instance()->builders.emplace_back(code, name, std::move(cb));
        }
        static const std::shared_ptr<BrokerBuilder>& getBuilder(int code)
        {
            for (auto& bb : instance()->builders) {
                if (std::get<0>(bb) == code) {
                    return std::get<2>(bb);
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
            for (auto& bb : instance()->builders) {
                if (std::get<0>(bb) <= 10) {
                    return std::get<2>(bb);
                }
            }
            if (blder->builders.empty()) {
                throw(HelicsException("core type is not available"));
            }
            return std::get<2>(blder->builders[0]);
        }

        static const std::shared_ptr<MasterBrokerBuilder>& instance()
        {
            static std::shared_ptr<MasterBrokerBuilder> iptr(new MasterBrokerBuilder());
            return iptr;
        }

      private:
        /** private constructor since we only really want one of them
        accessed through the instance static member*/
        MasterBrokerBuilder() = default;
        std::vector<BuildT> builders;  //!< container for the builders
    };

    void defineBrokerBuilder(std::shared_ptr<BrokerBuilder> cb, const std::string& name, int code)
    {
        MasterBrokerBuilder::addBuilder(std::move(cb), name, code);
    }

    std::shared_ptr<Broker> makeBroker(core_type type, const std::string& name)
    {
        if (type == core_type::NULLCORE) {
            throw(HelicsException("nullcore is explicitly not available nor will ever be"));
        }
        if (type == core_type::DEFAULT) {
            return MasterBrokerBuilder::getDefaultBuilder()->build(name);
        }
        return MasterBrokerBuilder::getBuilder(static_cast<int>(type))->build(name);
    }

    std::shared_ptr<Broker> create(core_type type, const std::string& configureString)
    {
        static const std::string emptyString;
        return create(type, emptyString, configureString);
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, const std::string& configureString)
    {
        auto broker = makeBroker(type, brokerName);
        if (!broker) {
            throw(helics::RegistrationFailure("unable to create broker"));
        }
        broker->configure(configureString);
        bool reg = registerBroker(broker, type);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    std::shared_ptr<Broker> create(core_type type, int argc, char* argv[])
    {
        static const std::string emptyString;
        return create(type, emptyString, argc, argv);
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, int argc, char* argv[])
    {
        auto broker = makeBroker(type, brokerName);
        broker->configureFromArgs(argc, argv);
        bool reg = registerBroker(broker, type);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    std::shared_ptr<Broker> create(core_type type, std::vector<std::string> args)
    {
        static const std::string emptyString;
        return create(type, emptyString, std::move(args));
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& brokerName, std::vector<std::string> args)
    {
        auto broker = makeBroker(type, brokerName);
        broker->configureFromVector(std::move(args));
        bool reg = registerBroker(broker, type);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    /** lambda function to join cores before the destruction happens to avoid potential problematic
     * calls in the loops*/
    static auto destroyerCallFirst = [](auto& broker) {
        auto tbroker = std::dynamic_pointer_cast<CoreBroker>(broker);
        if (tbroker) {
            tbroker->processDisconnect(true);  // use true here as it is possible the
                                               // searchableObjectHolder is deleted already
            tbroker->joinAllThreads();
        }
    };
    /** so the problem this is addressing is that unregister can potentially cause a destructor to
fire that destructor can delete a thread variable, unfortunately it is possible that a thread stored
in this variable can do the unregister operation and destroy itself meaning it is unable to join and
thus will call std::terminate what we do is delay the destruction until it is called in a different
thread which allows the destructor to fire if need be without issue*/

    static gmlc::concurrency::DelayedDestructor<Broker>
        delayedDestroyer(destroyerCallFirst);  //!< the object handling the delayed destruction

    static gmlc::concurrency::SearchableObjectHolder<Broker, core_type>
        searchableBrokers;  //!< the object managing the searchable objects

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Broker> findBroker(const std::string& brokerName)
    {
        auto brk = searchableBrokers.findObject(brokerName);
        if (brk) {
            return brk;
        }
        if (brokerName.empty()) {
            return getConnectedBroker();
        }
        if (brokerName.front() == '#') {
            try {
                auto val = std::stoull(brokerName.substr(1));
                return getBrokerByIndex(val);
            }
            catch (...) {
                return nullptr;
            }
        }
        return nullptr;
    }

    std::shared_ptr<Broker> getConnectedBroker()
    {
        return searchableBrokers.findObject([](auto& ptr) { return ptr->isConnected(); });
    }

    std::shared_ptr<Broker> getBrokerByIndex(size_t index)
    {
        auto brks = searchableBrokers.getObjects();
        return brks.size() > index ? brks[index] : nullptr;
    }

    std::shared_ptr<Broker> findJoinableBrokerOfType(core_type type)
    {
        return searchableBrokers.findObject([](auto& ptr) { return ptr->isOpenToNewFederates(); },
                                            type);
    }

    std::vector<std::shared_ptr<Broker>> getAllBrokers() { return searchableBrokers.getObjects(); }

    bool brokersActive() { return !searchableBrokers.empty(); }

    static void addExtraTypes(const std::string& name, core_type type)
    {
        switch (type) {
            case core_type::INPROC:
                searchableBrokers.addType(name, core_type::TEST);
                break;
            case core_type::TEST:
                searchableBrokers.addType(name, core_type::INPROC);
                break;
            case core_type::IPC:
                searchableBrokers.addType(name, core_type::INTERPROCESS);
                break;
            case core_type::INTERPROCESS:
                searchableBrokers.addType(name, core_type::IPC);
                break;
            default:
                break;
        }
    }
    bool registerBroker(const std::shared_ptr<Broker>& broker, core_type type)
    {
        bool registered = false;
        const std::string& bname = (broker) ? broker->getIdentifier() : std::string{};
        if (broker) {
            registered = searchableBrokers.addObject(bname, broker, type);
        }
        cleanUpBrokers();
        if ((!registered) && (broker)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            registered = searchableBrokers.addObject(bname, broker, type);
        }
        if (registered) {
            delayedDestroyer.addObjectsToBeDestroyed(broker);
            addExtraTypes(bname, type);
        }

        return registered;
    }

    size_t cleanUpBrokers() { return delayedDestroyer.destroyObjects(); }
    size_t cleanUpBrokers(std::chrono::milliseconds delay)
    {
        return delayedDestroyer.destroyObjects(delay);
    }

    void terminateAllBrokers()
    {
        auto brokers = getAllBrokers();
        for (auto& brk : brokers) {
            brk->disconnect();
        }
        cleanUpBrokers(std::chrono::milliseconds(250));
    }

    void abortAllBrokers(int errorCode, const std::string& errorString)
    {
        auto brokers = getAllBrokers();
        for (auto& brk : brokers) {
            brk->globalError(errorCode, brk->getIdentifier() + " sending-> " + errorString);
            brk->disconnect();
        }
        cleanUpBrokers(std::chrono::milliseconds(250));
    }
    bool copyBrokerIdentifier(const std::string& copyFromName, const std::string& copyToName)
    {
        return searchableBrokers.copyObject(copyFromName, copyToName);
    }

    void unregisterBroker(const std::string& name)
    {
        if (!searchableBrokers.removeObject(name)) {
            searchableBrokers.removeObject(
                [&name](auto& obj) { return (obj->getIdentifier() == name); });
        }
    }

    void addAssociatedBrokerType(const std::string& name, core_type type)
    {
        searchableBrokers.addType(name, type);
        addExtraTypes(name, type);
    }

    static const std::string helpStr{"--help"};

    void displayHelp(core_type type)
    {
        if (type == core_type::DEFAULT || type == core_type::UNRECOGNIZED) {
            std::cout << "All core types have similar options\n";
            auto brk = makeBroker(core_type::DEFAULT, std::string{});
            brk->configure(helpStr);
#ifdef ENABLE_TCP_CORE
            brk = makeBroker(core_type::TCP_SS, std::string{});
            brk->configure(helpStr);
#endif
        } else {
            auto brk = makeBroker(type, std::string{});
            brk->configure(helpStr);
        }
    }

}  // namespace BrokerFactory
}  // namespace helics
