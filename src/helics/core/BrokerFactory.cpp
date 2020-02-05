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

#ifdef ENABLE_ZMQ_CORE
#    include "zmq/ZmqBroker.h"
#endif

#ifdef ENABLE_MPI_CORE
#    include "mpi/MpiBroker.h"
#endif

#ifdef ENABLE_TEST_CORE
#    include "test/TestBroker.h"
#endif

#ifdef ENABLE_IPC_CORE
#    include "ipc/IpcBroker.h"
#endif

#ifdef ENABLE_UDP_CORE
#    include "udp/UdpBroker.h"
#endif

#ifdef ENABLE_TCP_CORE
#    include "tcp/TcpBroker.h"
#endif

#ifdef ENABLE_INPROC_CORE
#    include "inproc/InprocBroker.h"
#endif

#include <cassert>

namespace helics {
const std::string emptyString;
std::shared_ptr<Broker> makeBroker(core_type type, const std::string& name)
{
    std::shared_ptr<Broker> broker;

    if (type == core_type::DEFAULT) {
#ifdef ENABLE_ZMQ_CORE
        type = core_type::ZMQ;
#else
#    ifdef ENABLE_TCP_CORE
        type = core_type::TCP;
#    else
#        ifdef ENABLE_MPI_CORE
        type = core_type::MPI;
#        else
        type = core_type::UDP;
#        endif
#    endif
#endif
    }

    switch (type) {
        case core_type::ZMQ:
#ifdef ENABLE_ZMQ_CORE
            if (name.empty()) {
                broker = std::make_shared<zeromq::ZmqBroker>();
            } else {
                broker = std::make_shared<zeromq::ZmqBroker>(name);
            }

#else
            throw(HelicsException("ZMQ broker type is not available"));
#endif
            break;
        case core_type::ZMQ_SS:
#ifdef ENABLE_ZMQ_CORE
            if (name.empty()) {
                broker = std::make_shared<zeromq::ZmqBrokerSS>();
            } else {
                broker = std::make_shared<zeromq::ZmqBrokerSS>(name);
            }
#else
            throw(HelicsException("ZMQ single socket broker type is not available"));
#endif
            break;
        case core_type::MPI:
#ifdef ENABLE_MPI_CORE
            if (name.empty()) {
                broker = std::make_shared<mpi::MpiBroker>();
            } else {
                broker = std::make_shared<mpi::MpiBroker>(name);
            }
#else
            throw(HelicsException("mpi broker type is not available"));
#endif
            break;
        case core_type::TEST:
#ifdef ENABLE_TEST_CORE
            if (name.empty()) {
                broker = std::make_shared<testcore::TestBroker>();
            } else {
                broker = std::make_shared<testcore::TestBroker>(name);
            }
            break;
#else
            throw(HelicsException("Test broker type is not available"));
#endif
        case core_type::INPROC:
#ifdef ENABLE_INPROC_CORE
            if (name.empty()) {
                broker = std::make_shared<inproc::InprocBroker>();
            } else {
                broker = std::make_shared<inproc::InprocBroker>(name);
            }
            break;
#else
            throw(HelicsException("in process broker type is not available"));
#endif
        case core_type::INTERPROCESS:
        case core_type::IPC:
#ifdef ENABLE_IPC_CORE
            if (name.empty()) {
                broker = std::make_shared<ipc::IpcBroker>();
            } else {
                broker = std::make_shared<ipc::IpcBroker>(name);
            }
            break;
#else
            throw(HelicsException("ipc broker type is not available"));
#endif
        case core_type::UDP:
#ifdef ENABLE_UDP_CORE
            if (name.empty()) {
                broker = std::make_shared<udp::UdpBroker>();
            } else {
                broker = std::make_shared<udp::UdpBroker>(name);
            }
            break;
#else
            throw(HelicsException("udp broker type is not available"));
#endif
        case core_type::TCP:
#ifdef ENABLE_TCP_CORE
            if (name.empty()) {
                broker = std::make_shared<tcp::TcpBroker>();
            } else {
                broker = std::make_shared<tcp::TcpBroker>(name);
            }
#else
            throw(HelicsException("tcp broker type is not available"));
#endif
            break;
        case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
            if (name.empty()) {
                broker = std::make_shared<tcp::TcpBrokerSS>();
            } else {
                broker = std::make_shared<tcp::TcpBrokerSS>(name);
            }
#else
            throw(HelicsException("tcp single socket broker type is not available"));
#endif
            break;
        default:
            throw(HelicsException("unrecognized broker type"));
    }
    return broker;
}

namespace BrokerFactory {
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
        searchableObjects; //!< the object managing the searchable objects

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Broker> findBroker(const std::string& brokerName)
    {
        return searchableObjects.findObject(brokerName);
    }

    static bool isJoinableBrokerOfType(core_type type, const std::shared_ptr<Broker>& ptr)
    {
        if (ptr->isOpenToNewFederates()) {
            switch (type) {
                case core_type::ZMQ:
#ifdef ENABLE_ZMQ_CORE
                    return (dynamic_cast<zeromq::ZmqBroker*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::MPI:
#ifdef ENABLE_MPI_CORE
                    return (dynamic_cast<mpi::MpiBroker*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::TEST:
#ifdef ENABLE_TEST_CORE
                    return (dynamic_cast<testcore::TestBroker*>(ptr.get()) != nullptr);
#else
                    return false;
#endif
                case core_type::INPROC:
#ifdef ENABLE_INPROC_CORE
                    return (dynamic_cast<inproc::InprocBroker*>(ptr.get()) != nullptr);
#else
                    return false;
#endif
                case core_type::INTERPROCESS:
                case core_type::IPC:
#ifdef ENABLE_IPC_CORE
                    return (dynamic_cast<ipc::IpcBroker*>(ptr.get()) != nullptr);
#else
                    return false;
#endif
                case core_type::UDP:
#ifdef ENABLE_UDP_CORE
                    return (dynamic_cast<udp::UdpBroker*>(ptr.get()) != nullptr);
#else
                    return false;
#endif
                case core_type::TCP:
#ifdef ENABLE_TCP_CORE
                    return (dynamic_cast<tcp::TcpBroker*>(ptr.get()) != nullptr);
#else
                    return false;
#endif
                default:
                    return true;
            }
        }
        return false;
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
        return searchableObjects.findObject(
            [type](auto& ptr) { return isJoinableBrokerForType(type, ptr); });
    }

    std::vector<std::shared_ptr<Broker>> getAllBrokers() { return searchableObjects.getObjects(); }

    bool brokersActive() { return !searchableObjects.empty(); }

    bool registerBroker(const std::shared_ptr<Broker>& broker)
    {
        bool registered = false;
        if (broker) {
            registered = searchableObjects.addObject(broker->getIdentifier(), broker);
        }
        cleanUpBrokers();
        if ((!registered) && (broker)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            registered = searchableObjects.addObject(broker->getIdentifier(), broker);
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

    bool copyBrokerIdentifier(const std::string& copyFromName, const std::string& copyToName)
    {
        return searchableObjects.copyObject(copyFromName, copyToName);
    }

    void unregisterBroker(const std::string& name)
    {
        if (!searchableObjects.removeObject(name)) {
            searchableObjects.removeObject(
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
