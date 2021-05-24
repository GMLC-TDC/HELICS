/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Core.hpp"
#include "core-types.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace helics {
/**
 * Factory for building Core API instances.
 */

/** available core types*/

namespace CoreFactory {
    /** builder for cores*/
    class CoreBuilder {
      public:
        virtual std::shared_ptr<Core> build(const std::string& name) = 0;
    };

    /** template for making a Core builder*/
    template<class CoreTYPE>
    class CoreTypeBuilder final: public CoreBuilder {
      public:
        static_assert(std::is_base_of<Core, CoreTYPE>::value,
                      "Type does not inherit from helics::Core");

        using core_build_type = CoreTYPE;
        virtual std::shared_ptr<Core> build(const std::string& name) override
        {
            return std::make_shared<CoreTYPE>(name);
        }
    };

    //** define a new Core Builder from the builder give a name and build code*/
    void defineCoreBuilder(std::shared_ptr<CoreBuilder> cb,
                           const std::string& coreTypeName,
                           int code);

    /** template function to create a builder and link it into the library*/
    template<class CoreTYPE>
    std::shared_ptr<CoreBuilder> addCoreType(const std::string& coreTypeName, int code)
    {
        auto bld = std::make_shared<CoreTypeBuilder<CoreTYPE>>();
        std::shared_ptr<CoreBuilder> cbld = std::static_pointer_cast<CoreBuilder>(bld);
        defineCoreBuilder(cbld, coreTypeName, code);
        return cbld;
    }

    /** create a core from a type, name, and initializationString
@param type the type of core to create
@param coreName the name for the core
@param configureString a string containing arguments for configuration of the core
*/
    std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, const std::string& configureString);
    /**
     * Creates a Core API object of the specified type.
     *
     * Invokes initialize() on the instantiated Core object.
     */
    std::shared_ptr<Core> create(core_type type, const std::string& configureString);

    /** create a core from a type and command line arguments
@param type the type of core to create
@param argc the number of command line arguments
@param argv the actual string parameters for the command line
*/
    std::shared_ptr<Core> create(core_type type, int argc, char* argv[]);

    /** create a core from arguments
@details an argument of '--coretype' must be specified to define the type,  otherwise the default
type is used
@param args a vector of reversed command line arguments
@return a pointer to the created core
*/
    std::shared_ptr<Core> create(std::vector<std::string> args);

    /** create a core from a type and command line arguments
@param type the type of core to create
@param args a vector of reversed command line arguments
*/
    std::shared_ptr<Core> create(core_type type, std::vector<std::string> args);

    /** create a core from arguments
@details an argument of '--coretype' must be specified to define the type,  otherwise the default
type is used
@param argc the number of arguments
@param argv the actual argument parameters
@return a pointer to the created core
*/
    std::shared_ptr<Core> create(int argc, char* argv[]);

    /** create a core from a type, name, and arguments
@param type the type of core to create
@param coreName the name for the core
@param argc the number of arguments
@param argv the actual argument parameters
@return a pointer to the created core
*/
    std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, int argc, char* argv[]);

    /** create a core from a type, name, and arguments
@param type the type of core to create
@param coreName the name for the core
@param args a vector of reversed command line arguments
@return a pointer to the created core
*/
    std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, std::vector<std::string> args);

    /** tries to find a named core if it fails it creates a new one
     */
    std::shared_ptr<Core> FindOrCreate(core_type type,
                                       const std::string& coreName,
                                       const std::string& configureString);

    /** tries to find a named core if it fails it creates a new one
     */
    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, int argc, char* argv[]);

    /** tries to find a named core if it fails it creates a new one
     */
    std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, std::vector<std::string> args);
    /** try to find a joinable core of a specific type*/
    std::shared_ptr<Core> findJoinableCoreOfType(core_type type);

    /** locate a registered Core by name
@param name the name of the core to find
@return a shared_ptr to the testCore*/
    std::shared_ptr<Core> findCore(const std::string& name);

    /** register a testCore so it can be found by others
@details also cleans up any leftover Cores that were previously unregistered this can be controlled
by calling cleanUpCores earlier if desired
@param core a pointer to a testCore object that should be found globally
@return true if the registration was successful false otherwise*/
    bool registerCore(const std::shared_ptr<Core>& core, core_type type);

    /** remove a Core from the registry
@param name the name of the Core to unregister
*/
    void unregisterCore(const std::string& name);

    /** add a type associated with a core*/
    void addAssociatedCoreType(const std::string& name, core_type type);

    /** clean up unused cores
@details when Cores are unregistered they get put in a holding area that gets cleaned up when a new
Core is registered or when the clean up function is called this prevents some odd threading issues
@return the number of cores still operating
*/
    size_t cleanUpCores();
    /** clean up unused cores
@details when Cores are unregistered they get put in a holding area that gets cleaned up when a new
Core is registered or when the clean up function is called this prevents some odd threading issues
@param delay the delay time in milliseconds to wait for the cores to finish before destroying
@return the number of cores still operating
*/
    size_t cleanUpCores(std::chrono::milliseconds delay);

    /** make a copy of the core pointer to allow access to the new name
@return true if the copyFromName was found and the copy successful
 */
    bool copyCoreIdentifier(const std::string& copyFromName, const std::string& copyToName);

    /** display the help listing for a particular core_type*/
    void displayHelp(core_type type = core_type::UNRECOGNIZED);

    /** terminate All existing cores */
    void terminateAllCores();
    /** abort all cores */
    void abortAllCores(int errorCode, const std::string& errorString);

    /** get the current number of cores */
    size_t getCoreCount();
}  // namespace CoreFactory

}  // namespace helics
