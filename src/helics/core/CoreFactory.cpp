/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#define ENABLE_TRIPWIRE

#include "CoreFactory.hpp"

#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "helics/helics-config.h"
#ifdef ENABLE_ZMQ_CORE
#    include "zmq/ZmqCore.h"
#endif

#ifdef ENABLE_MPI_CORE
#    include "mpi/MpiCore.h"
#endif

#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"

#ifdef ENABLE_TEST_CORE
#    include "test/TestCore.h"
#endif

#ifdef ENABLE_IPC_CORE
#    include "ipc/IpcCore.h"
#endif

#ifdef ENABLE_UDP_CORE
#    include "udp/UdpCore.h"
#endif

#ifdef ENABLE_TCP_CORE
#    include "tcp/TcpCore.h"
#endif

#ifdef ENABLE_INPROC_CORE
#    include "inproc/InprocCore.h"
#endif

#include "helicsCLI11.hpp"

#include <cassert>
#include <cstring>

namespace helics {
std::shared_ptr<Core> makeCore(core_type type, const std::string& name)
{
    std::shared_ptr<Core> core;
    if (type == core_type::DEFAULT) {
#ifdef ENABLE_ZMQ_CORE
        type = core_type::ZMQ;
#else
#    ifdef ENABLE_TCP_CORE
        type = core_type::TCP;
#    else
#        ifdef ENABLE_UDP_CORE
        type = core_type::UDP;
#        else
#            ifdef ENABLE_MPI_CORE
        type = core_type::MPI;
#            else
#                ifdef ENABLE_IPC_CORE
        type = core_type::IPC;
#                else
#                    ifdef ENABLE_TEST_CORE
        type = core_type::TEST;
#                    else
        type = core_type::UNRECOGNIZED;
#                    endif // ENABLE_TEST_CORE
#                endif // ENABLE_IPC_CORE
#            endif // ENABLE_MPI_CORE
#        endif // ENABLE_UDP_CORE
#    endif // ENABLE_TCP_CORE
#endif // ENABLE_ZMQ_CORE
    }

    switch (type) {
        case core_type::ZMQ:
#ifdef ENABLE_ZMQ_CORE
            if (name.empty()) {
                core = std::make_shared<zeromq::ZmqCore>();
            } else {
                core = std::make_shared<zeromq::ZmqCore>(name);
            }

#else
            throw(HelicsException("ZMQ core is not available"));
#endif
            break;
        case core_type::ZMQ_SS:
#ifdef ENABLE_ZMQ_CORE
            if (name.empty()) {
                core = std::make_shared<zeromq::ZmqCoreSS>();
            } else {
                core = std::make_shared<zeromq::ZmqCoreSS>(name);
            }

#else
            throw(HelicsException("ZMQ core is not available"));
#endif
            break;
        case core_type::MPI:
#ifdef ENABLE_MPI_CORE
            if (name.empty()) {
                core = std::make_shared<mpi::MpiCore>();
            } else {
                core = std::make_shared<mpi::MpiCore>(name);
            }
#else
            throw(HelicsException("MPI core is not available"));
#endif
            break;
        case core_type::TEST:
#ifdef ENABLE_TEST_CORE
            if (name.empty()) {
                core = std::make_shared<testcore::TestCore>();
            } else {
                core = std::make_shared<testcore::TestCore>(name);
            }
            break;
#else
            throw(HelicsException("TEST core is not available"));
#endif
        case core_type::INPROC:
#ifdef ENABLE_INPROC_CORE
            if (name.empty()) {
                core = std::make_shared<inproc::InprocCore>();
            } else {
                core = std::make_shared<inproc::InprocCore>(name);
            }
            break;
#else
            throw(HelicsException("Inproc core is not available"));
#endif
        case core_type::INTERPROCESS:
        case core_type::IPC:
#ifdef ENABLE_IPC_CORE
            if (name.empty()) {
                core = std::make_shared<ipc::IpcCore>();
            } else {
                core = std::make_shared<ipc::IpcCore>(name);
            }
            break;
#else
            throw(HelicsException("IPC core is not available"));
#endif
        case core_type::UDP:
#ifdef ENABLE_UDP_CORE
            if (name.empty()) {
                core = std::make_shared<udp::UdpCore>();
            } else {
                core = std::make_shared<udp::UdpCore>(name);
            }
            break;
#else
            throw(HelicsException("UDP core is not available"));
#endif
        case core_type::TCP:
#ifdef ENABLE_TCP_CORE
            if (name.empty()) {
                core = std::make_shared<tcp::TcpCore>();
            } else {
                core = std::make_shared<tcp::TcpCore>(name);
            }
#else
            throw(HelicsException("TCP core is not available"));
#endif
            break;
        case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
            if (name.empty()) {
                core = std::make_shared<tcp::TcpCoreSS>();
            } else {
                core = std::make_shared<tcp::TcpCoreSS>(name);
            }
#else
            throw(HelicsException("TCP single socket core is not available"));
#endif
            break;
        default:
            throw(HelicsException("unrecognized core type"));
    }
    return core;
}

namespace CoreFactory {
    static const std::string emptyString;

    std::shared_ptr<Core> create(const std::string& initializationString)
    {
        helicsCLI11App tparser;
        tparser.remove_helics_specifics();
        tparser.addTypeOption();
        tparser.allow_extras();
        tparser.parse(initializationString);
        return create(tparser.getCoreType(), emptyString, tparser.remaining_for_passthrough());
    }

    std::shared_ptr<Core> create(core_type type, const std::string& configureString)
    {
        return create(type, emptyString, configureString);
    }

    std::shared_ptr<Core>
        create(core_type type, const std::string& core_name, const std::string& configureString)
    {
        auto core = makeCore(type, core_name);
        if (!core) {
            throw(helics::RegistrationFailure("unable to create core"));
        }
        core->configure(configureString);
        registerCore(core);

        return core;
    }

    std::shared_ptr<Core> create(std::vector<std::string> args)
    {
        helicsCLI11App tparser;
        tparser.remove_helics_specifics();
        tparser.addTypeOption();

        tparser.allow_extras();
        tparser.parse(std::move(args));
        return create(tparser.getCoreType(), emptyString, tparser.remaining_for_passthrough());
    }

    std::shared_ptr<Core> create(core_type type, std::vector<std::string> args)
    {
        return create(type, emptyString, std::move(args));
    }

    std::shared_ptr<Core>
        create(core_type type, const std::string& core_name, std::vector<std::string> args)
    {
        auto core = makeCore(type, core_name);
        core->configureFromVector(std::move(args));
        registerCore(core);

        return core;
    }

    std::shared_ptr<Core> create(int argc, char* argv[])
    {
        helicsCLI11App tparser;
        tparser.remove_helics_specifics();
        tparser.addTypeOption();

        tparser.allow_extras();
        tparser.parse(argc, argv);
        return create(tparser.getCoreType(), tparser.remaining_for_passthrough());
    }

    std::shared_ptr<Core> create(core_type type, int argc, char* argv[])
    {
        return create(type, emptyString, argc, argv);
    }

    std::shared_ptr<Core>
        create(core_type type, const std::string& core_name, int argc, char* argv[])
    {
        auto core = makeCore(type, core_name);
        core->configureFromArgs(argc, argv);
        registerCore(core);

        return core;
    }

    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& core_name, std::vector<std::string> args)
    {
        std::shared_ptr<Core> core = findCore(core_name);
        if (core) {
            return core;
        }
        core = makeCore(type, core_name);
        core->configureFromVector(std::move(args));

        bool success = registerCore(core);
        if (!success) {
            core = findCore(core_name);
            if (core) {
                return core;
            }
        }

        return core;
    }

    std::shared_ptr<Core> FindOrCreate(
        core_type type,
        const std::string& core_name,
        const std::string& configureString)
    {
        std::shared_ptr<Core> core = findCore(core_name);
        if (core) {
            return core;
        }
        core = makeCore(type, core_name);
        core->configure(configureString);

        bool success = registerCore(core);
        if (!success) {
            core = findCore(core_name);
            if (core) {
                return core;
            }
        }

        return core;
    }

    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& core_name, int argc, char* argv[])
    {
        std::shared_ptr<Core> core = findCore(core_name);
        if (core) {
            return core;
        }
        core = makeCore(type, core_name);

        core->configureFromArgs(argc, argv);
        bool success = registerCore(core);
        if (!success) {
            core = findCore(core_name);
            if (core) {
                return core;
            }
        }

        return core;
    }

    /** lambda function to join cores before the destruction happens to avoid potential problematic calls in the
 * loops*/
    static auto destroyerCallFirst = [](auto& core) {
        core->processDisconnect(true);
        core->joinAllThreads();
    };

    /** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be
without issue*/
    static gmlc::concurrency::DelayedDestructor<CommonCore>
        delayedDestroyer(destroyerCallFirst); //!< the object handling the delayed destruction

    static gmlc::concurrency::SearchableObjectHolder<CommonCore>
        searchableObjects; //!< the object managing the searchable objects

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Core> findCore(const std::string& name)
    {
        return searchableObjects.findObject(name);
    }

    static bool isJoinableCoreOfType(core_type type, const std::shared_ptr<CommonCore>& ptr)
    {
        if (ptr->isOpenToNewFederates()) {
            switch (type) {
                case core_type::ZMQ:
#ifdef ENABLE_ZMQ_CORE
                    return (dynamic_cast<zeromq::ZmqCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::MPI:
#ifdef ENABLE_MPI_CORE
                    return (dynamic_cast<mpi::MpiCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::TEST:
#ifdef ENABLE_TEST_CORE
                    return (dynamic_cast<testcore::TestCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::INPROC:
#ifdef ENABLE_INPROC_CORE
                    return (dynamic_cast<inproc::InprocCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::INTERPROCESS:
                case core_type::IPC:
#ifdef ENABLE_IPC_CORE
                    return (dynamic_cast<ipc::IpcCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::UDP:
#ifdef ENABLE_UDP_CORE
                    return (dynamic_cast<udp::UdpCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::TCP:
#ifdef ENABLE_TCP_CORE
                    return (dynamic_cast<tcp::TcpCore*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
                    return (dynamic_cast<tcp::TcpCoreSS*>(ptr.get()) != nullptr);
#else
                    break;
#endif
                default:
                    return true;
            }
        }
        return false;
    }

    static bool isJoinableCoreForType(core_type type, const std::shared_ptr<CommonCore>& ptr)
    {
        if (type == core_type::INPROC || type == core_type::TEST) {
            return isJoinableCoreOfType(core_type::INPROC, ptr) ||
                isJoinableCoreOfType(core_type::TEST, ptr);
        }
        return isJoinableCoreOfType(type, ptr);
    }

    std::shared_ptr<Core> findJoinableCoreOfType(core_type type)
    {
        return searchableObjects.findObject(
            [type](auto& ptr) { return isJoinableCoreForType(type, ptr); });
    }

    bool registerCore(const std::shared_ptr<Core>& core)
    {
        bool res = false;
        auto tcore = std::dynamic_pointer_cast<CommonCore>(core);
        if (tcore) {
            res = searchableObjects.addObject(tcore->getIdentifier(), tcore);
        }
        cleanUpCores();
        if (res) {
            delayedDestroyer.addObjectsToBeDestroyed(tcore);
        }
        return res;
    }

    size_t cleanUpCores() { return delayedDestroyer.destroyObjects(); }

    size_t cleanUpCores(std::chrono::milliseconds delay)
    {
        return delayedDestroyer.destroyObjects(delay);
    }

    bool copyCoreIdentifier(const std::string& copyFromName, const std::string& copyToName)
    {
        return searchableObjects.copyObject(copyFromName, copyToName);
    }

    void unregisterCore(const std::string& name)
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
            auto cr = makeCore(core_type::DEFAULT, emptyString);
            cr->configure(helpStr);
#ifdef ENABLE_TCP_CORE
            cr = makeCore(core_type::TCP_SS, emptyString);
            cr->configure(helpStr);
#endif
        } else {
            auto cr = makeCore(type, emptyString);
            cr->configure(helpStr);
        }
    }

} // namespace CoreFactory
} // namespace helics
