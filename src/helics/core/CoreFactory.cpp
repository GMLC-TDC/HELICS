/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#define ENABLE_TRIPWIRE

#include "CoreFactory.hpp"

#include "CommonCore.hpp"
#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "gmlc/concurrency/DelayedDestructor.hpp"
#include "gmlc/concurrency/SearchableObjectHolder.hpp"
#include "gmlc/libguarded/shared_guarded.hpp"
#include "helics/helics-config.h"
#include "helicsCLI11.hpp"

#include <cassert>
#include <cstring>
#include <tuple>
#include <utility>

DECLARE_TRIPLINE()

namespace helics {

namespace CoreFactory {
    static const std::string emptyString;

    /*** class to hold the set of builders
   @details this doesn't work as a global since it tends to get initialized after some of the things
   that call it so it needs to be a static member of function call*/
    class MasterCoreBuilder {
      public:
        using BuildT = std::tuple<int, std::string, std::shared_ptr<CoreBuilder>>;

        static void addBuilder(std::shared_ptr<CoreBuilder> cb, const std::string& name, int code)
        {
            instance()->builders.emplace_back(code, name, std::move(cb));
        }
        static const std::shared_ptr<CoreBuilder>& getBuilder(int code)
        {
            for (auto& bb : instance()->builders) {
                if (std::get<0>(bb) == code) {
                    return std::get<2>(bb);
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
            static std::shared_ptr<MasterCoreBuilder> iptr(new MasterCoreBuilder());
            return iptr;
        }

      private:
        /** private constructor since we only really want one of them
        accessed through the instance static member*/
        MasterCoreBuilder() = default;
        std::vector<BuildT> builders;  //!< container for the different builders
    };

    void defineCoreBuilder(std::shared_ptr<CoreBuilder> cb, const std::string& name, int code)
    {
        MasterCoreBuilder::addBuilder(std::move(cb), name, code);
    }

    std::shared_ptr<Core> makeCore(core_type type, const std::string& name)
    {
        if (type == core_type::NULLCORE) {
            throw(HelicsException("nullcore is explicitly not available nor will ever be"));
        }
        if (type == core_type::DEFAULT) {
            return MasterCoreBuilder::getIndexedBuilder(0)->build(name);
        }
        return MasterCoreBuilder::getBuilder(static_cast<int>(type))->build(name);
    }

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
        create(core_type type, const std::string& coreName, const std::string& configureString)
    {
        auto core = makeCore(type, coreName);
        if (!core) {
            throw(helics::RegistrationFailure("unable to create core"));
        }
        core->configure(configureString);
        if (!registerCore(core, type)) {
            throw(helics::RegistrationFailure(std::string("core ") + core->getIdentifier() +
                                              " failed to register properly"));
        }

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
        create(core_type type, const std::string& coreName, std::vector<std::string> args)
    {
        auto core = makeCore(type, coreName);
        core->configureFromVector(std::move(args));
        if (!registerCore(core, type)) {
            throw(helics::RegistrationFailure(std::string("core ") + core->getIdentifier() +
                                              " failed to register properly"));
        }

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
        create(core_type type, const std::string& coreName, int argc, char* argv[])
    {
        auto core = makeCore(type, coreName);
        core->configureFromArgs(argc, argv);
        if (!registerCore(core, type)) {
            throw(helics::RegistrationFailure(std::string("core ") + core->getIdentifier() +
                                              " failed to register properly"));
        }

        return core;
    }

    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, std::vector<std::string> args)
    {
        std::shared_ptr<Core> core = findCore(coreName);
        if (core) {
            return core;
        }
        core = makeCore(type, coreName);
        core->configureFromVector(std::move(args));

        bool success = registerCore(core, type);
        if (!success) {
            core = findCore(coreName);
            if (core) {
                return core;
            }
        }

        return core;
    }

    std::shared_ptr<Core> FindOrCreate(core_type type,
                                       const std::string& coreName,
                                       const std::string& configureString)
    {
        std::shared_ptr<Core> core = findCore(coreName);
        if (core) {
            return core;
        }
        core = makeCore(type, coreName);
        core->configure(configureString);

        bool success = registerCore(core, type);
        if (!success) {
            core = findCore(coreName);
            if (core) {
                return core;
            }
        }

        return core;
    }

    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, int argc, char* argv[])
    {
        std::shared_ptr<Core> core = findCore(coreName);
        if (core) {
            return core;
        }
        core = makeCore(type, coreName);

        core->configureFromArgs(argc, argv);
        bool success = registerCore(core, type);
        if (!success) {
            core = findCore(coreName);
            if (core) {
                return core;
            }
        }

        return core;
    }

    /** lambda function to join cores before the destruction happens to avoid potential problematic
     * calls in the loops*/
    static auto destroyerCallFirst = [](std::shared_ptr<Core>& core) {
        auto* ccore = dynamic_cast<CommonCore*>(core.get());
        if (ccore != nullptr) {
            ccore->processDisconnect(true);
            ccore->joinAllThreads();
        }
    };

    /** so the problem this is addressing is that unregister can potentially cause a destructor to
fire that destructor can delete a thread variable, unfortunately it is possible that a thread stored
in this variable can do the unregister operation and destroy itself meaning it is unable to join and
thus will call std::terminate what we do is delay the destruction until it is called in a different
thread which allows the destructor to fire if need be without issue*/
    static gmlc::concurrency::DelayedDestructor<Core>
        delayedDestroyer(destroyerCallFirst);  //!< the object handling the delayed destruction

    static gmlc::concurrency::SearchableObjectHolder<Core, core_type>
        searchableCores;  //!< the object managing the searchable cores

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Core> findCore(const std::string& name)
    {
        return searchableCores.findObject(name);
    }

    std::shared_ptr<Core> findJoinableCoreOfType(core_type type)
    {
        return searchableCores.findObject([](auto& ptr) { return ptr->isOpenToNewFederates(); },
                                          type);
    }

    static void addExtraTypes(const std::string& name, core_type type)
    {
        switch (type) {
            case core_type::INPROC:
                searchableCores.addType(name, core_type::TEST);
                break;
            case core_type::TEST:
                searchableCores.addType(name, core_type::INPROC);
                break;
            case core_type::IPC:
                searchableCores.addType(name, core_type::INTERPROCESS);
                break;
            case core_type::INTERPROCESS:
                searchableCores.addType(name, core_type::IPC);
                break;
            default:
                break;
        }
    }

    bool registerCore(const std::shared_ptr<Core>& core, core_type type)
    {
        bool res = false;
        const std::string& cname = (core) ? core->getIdentifier() : std::string{};
        if (core) {
            res = searchableCores.addObject(cname, core, type);
        }
        cleanUpCores();
        if (res) {
            delayedDestroyer.addObjectsToBeDestroyed(core);
            addExtraTypes(cname, type);
        }
        return res;
    }

    size_t cleanUpCores() { return delayedDestroyer.destroyObjects(); }

    size_t cleanUpCores(std::chrono::milliseconds delay)
    {
        return delayedDestroyer.destroyObjects(delay);
    }

    void terminateAllCores()
    {
        auto cores = searchableCores.getObjects();
        for (auto& cr : cores) {
            cr->disconnect();
        }
        cleanUpCores(std::chrono::milliseconds(250));
    }

    void abortAllCores(int errorCode, const std::string& errorString)
    {
        auto cores = searchableCores.getObjects();
        for (auto& cr : cores) {
            cr->globalError(local_core_id,
                            errorCode,
                            cr->getIdentifier() + " sending-> " + errorString);
            cr->disconnect();
        }
        cleanUpCores(std::chrono::milliseconds(250));
    }

    size_t getCoreCount() { return searchableCores.getObjects().size(); }
    bool copyCoreIdentifier(const std::string& copyFromName, const std::string& copyToName)
    {
        return searchableCores.copyObject(copyFromName, copyToName);
    }

    void unregisterCore(const std::string& name)
    {
        if (!searchableCores.removeObject(name)) {
            searchableCores.removeObject(
                [&name](auto& obj) { return (obj->getIdentifier() == name); });
        }
    }

    void addAssociatedCoreType(const std::string& name, core_type type)
    {
        searchableCores.addType(name, type);
        addExtraTypes(name, type);
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

}  // namespace CoreFactory
}  // namespace helics
