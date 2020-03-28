/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#define ENABLE_TRIPWIRE

#include "BrokerFactory.hpp"

#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics/helics-config.h"
#include "CoreBroker.hpp"
#include <cassert>

namespace helics {
static const std::string emptyString;

namespace BrokerFactory {

    using BuildT = std::tuple<int, std::string, std::shared_ptr<BrokerBuilder>>;

    class MasterBrokerBuilder
    {
    public:
        static void addBuilder(std::shared_ptr<BrokerBuilder> cb, const std::string& name, int code)
        {
            instance()->builders.emplace_back(code, name, std::move(cb));
        }
        static std::shared_ptr<BrokerBuilder> &getBuilder(int code)
        {
            for (auto &bb : instance()->builders)
            {
                if (std::get<0>(bb) == code)
                {
                    return std::get<2>(bb);
                }
            }
            throw(HelicsException("core type is not available"));
        }
        static std::shared_ptr<BrokerBuilder> &getIndexedBuilder(std::size_t index)
        {
            auto &blder = instance();
            if (blder->builders.size() < index)
            {
                return std::get<2>(blder->builders[index]);
            }
            throw(HelicsException("core type is not available"));
        }
        static std::shared_ptr<MasterBrokerBuilder> &instance()
        {
            static std::shared_ptr<MasterBrokerBuilder> iptr(new MasterBrokerBuilder());
            return iptr;
        }
    private:
        MasterBrokerBuilder() = default;
        std::vector<BuildT> builders;
    };
    
    void defineBrokerBuilder(std::shared_ptr<BrokerBuilder> cb, const std::string& name, int code)
    {
        MasterBrokerBuilder::addBuilder(std::move(cb), name, code);
    }

    std::shared_ptr<Broker> makeBroker(core_type type, const std::string& name)
    {
        if (type == core_type::NULLCORE)
        {
            throw(HelicsException("nullcore is explicitly not available nor will ever be"));
        }
        if (type == core_type::DEFAULT)
        {
            return MasterBrokerBuilder::getIndexedBuilder(0)->build(name);
        }
        return MasterBrokerBuilder::getBuilder(static_cast<int>(type))->build(name);
    }

    std::shared_ptr<Broker> create(core_type type, const std::string& configureString)
    {
        return create(type, emptyString, configureString);
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, const std::string& configureString)
    {
        auto broker = makeBroker(type, broker_name);
        if (!broker) {
            throw(helics::RegistrationFailure("unable to create broker"));
        }
        broker->configure(configureString);
        bool reg = registerBroker(broker);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    std::shared_ptr<Broker> create(core_type type, int argc, char* argv[])
    {
        return create(type, emptyString, argc, argv);
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, int argc, char* argv[])
    {
        auto broker = makeBroker(type, broker_name);
        broker->configureFromArgs(argc, argv);
        bool reg = registerBroker(broker);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    std::shared_ptr<Broker> create(core_type type, std::vector<std::string> args)
    {
        return create(type, emptyString, std::move(args));
    }

    std::shared_ptr<Broker>
        create(core_type type, const std::string& broker_name, std::vector<std::string> args)
    {
        auto broker = makeBroker(type, broker_name);
        broker->configureFromVector(std::move(args));
        bool reg = registerBroker(broker);
        if (!reg) {
            throw(helics::RegistrationFailure("unable to register broker"));
        }
        broker->connect();
        return broker;
    }

    /** lambda function to join cores before the destruction happens to avoid potential problematic calls in the
 * loops*/
    static auto destroyerCallFirst = [](auto& broker) {
        auto tbroker = std::dynamic_pointer_cast<CoreBroker>(broker);
        if (tbroker) {
            tbroker->processDisconnect(
                true); // use true here as it is possible the searchableObjectHolder is deleted already
            tbroker->joinAllThreads();
        }
    };
    /** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be without issue*/

    static gmlc::concurrency::DelayedDestructor<Broker>
        delayedDestroyer(destroyerCallFirst); //!< the object handling the delayed destruction

    static gmlc::concurrency::SearchableObjectHolder<Broker>
        searchableBrokers; //!< the object managing the searchable objects

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Broker> findBroker(const std::string& brokerName)
    {
        return searchableBrokers.findObject(brokerName);
    }

    static bool isJoinableBrokerOfType(core_type type, const std::shared_ptr<Broker>& ptr)
    {
        if (!ptr->isOpenToNewFederates()) {
            return false;
        }
        try
        {
            return MasterBrokerBuilder::getBuilder(static_cast<int>(type))->checkType(ptr.get());
        }
        catch (const helics::HelicsException &)
        {
            return true;
        }
    }

    static bool isJoinableBrokerForType(core_type type, const std::shared_ptr<Broker>& ptr)
    {
        if (type == core_type::INPROC || type == core_type::TEST) {
            return isJoinableBrokerOfType(core_type::INPROC, ptr) ||
                isJoinableBrokerOfType(core_type::TEST, ptr);
        }
        return isJoinableBrokerOfType(type, ptr);
    }

    std::shared_ptr<Broker> findJoinableBrokerOfType(core_type type)
    {
        return searchableBrokers.findObject(
            [type](auto& ptr) { return isJoinableBrokerForType(type, ptr); });
    }

    std::vector<std::shared_ptr<Broker>> getAllBrokers() { return searchableBrokers.getObjects(); }

    bool brokersActive() { return !searchableBrokers.empty(); }

    bool registerBroker(const std::shared_ptr<Broker>& broker)
    {
        bool registered = false;
        if (broker) {
            registered = searchableBrokers.addObject(broker->getIdentifier(), broker);
        }
        cleanUpBrokers();
        if ((!registered) && (broker)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            registered = searchableBrokers.addObject(broker->getIdentifier(), broker);
        }
        if (registered) {
            delayedDestroyer.addObjectsToBeDestroyed(broker);
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

    static const std::string helpStr{"--help"};

    void displayHelp(core_type type)
    {
        if (type == core_type::DEFAULT || type == core_type::UNRECOGNIZED) {
            std::cout << "All core types have similar options\n";
            auto brk = makeBroker(core_type::DEFAULT, emptyString);
            brk->configure(helpStr);
#ifdef ENABLE_TCP_CORE
            brk = makeBroker(core_type::TCP_SS, emptyString);
            brk->configure(helpStr);
#endif
        } else {
            auto brk = makeBroker(type, emptyString);
            brk->configure(helpStr);
        }
    }

} // namespace BrokerFactory
} // namespace helics
