/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
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

namespace helics {

namespace CoreFactory {
    static const std::string emptyString;

    /*** class to holder the set of builders
   @details this doesn't work as a global since it tends to get initialized after some of the things that call it
   so it needs to be a static member of function call*/
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
            auto& blder = instance();
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
        std::vector<BuildT> builders; //!< container for the different builders
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
    static auto destroyerCallFirst = [](std::shared_ptr<Core>& core) {
        auto ccore = dynamic_cast<CommonCore*>(core.get());
        if (ccore != nullptr) {
            ccore->processDisconnect(true);
            ccore->joinAllThreads();
        }
    };

    /** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be
without issue*/
    static gmlc::concurrency::DelayedDestructor<Core>
        delayedDestroyer(destroyerCallFirst); //!< the object handling the delayed destruction

    static gmlc::concurrency::SearchableObjectHolder<Core>
        searchableCores; //!< the object managing the searchable cores

    // this will trip the line when it is destroyed at global destruction time
    static gmlc::concurrency::TripWireTrigger tripTrigger;

    std::shared_ptr<Core> findCore(const std::string& name)
    {
        return searchableCores.findObject(name);
    }

    static bool isJoinableCoreOfType(core_type type, const std::shared_ptr<Core>& ptr)
    {
        if (!ptr->isOpenToNewFederates()) {
            return false;
        }
        try {
            return MasterCoreBuilder::getBuilder(static_cast<int>(type))->checkType(ptr.get());
        }
        catch (const helics::HelicsException&) {
            // the error will throw if the MasterCoreBuilder can't find the core type, in which case it is open yet so return true
            return true;
        }
    }

    static bool isJoinableCoreForType(core_type type, const std::shared_ptr<Core>& ptr)
    {
        if (type == core_type::INPROC || type == core_type::TEST) {
            return isJoinableCoreOfType(core_type::INPROC, ptr) ||
                isJoinableCoreOfType(core_type::TEST, ptr);
        }
        return isJoinableCoreOfType(type, ptr);
    }

    std::shared_ptr<Core> findJoinableCoreOfType(core_type type)
    {
        return searchableCores.findObject(
            [type](auto& ptr) { return isJoinableCoreForType(type, ptr); });
    }

    bool registerCore(const std::shared_ptr<Core>& core)
    {
        bool res = false;
        if (core) {
            res = searchableCores.addObject(core->getIdentifier(), core);
        }
        cleanUpCores();
        if (res) {
            delayedDestroyer.addObjectsToBeDestroyed(core);
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
