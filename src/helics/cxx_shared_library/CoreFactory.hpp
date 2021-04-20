/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/CoreApp.hpp"
#include "Core.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {
#define HELICS_SHARED_DEPRECATED_CORE                                                                 \
    [[deprecated(                                                                                     \
        "Core Factory deprecated for use in the C++ shared library use CoreApp instead if you "       \
        "really need the functionality and it is not available in CoreApp either link to the static " \
        "library and/or contact the developers to "                                                   \
        "potentially add it to CoreApp")]]
/**
 * Factory for building Core API instances.
 */

/** available core types*/

namespace CoreFactory {
    /** create a core from a type, name, and initializationString
@param type the type of core to create
@param coreName the name for the core
@param initializationString a string containing arguments for the core
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, const std::string& initializationString)
    {
        CoreApp cr(type, coreName, initializationString);
        return cr.getCopyofCorePointer();
    }
    /**
     * Creates a Core API object of the specified type.
     *
     * Invokes initialize() on the instantiated Core object.
     */
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        create(core_type type, const std::string& initializationString)
    {
        CoreApp cr(type, initializationString);
        return cr.getCopyofCorePointer();
    }

    /** create a core from a type and command line arguments
@param type the type of core to create
@param argc the number of command line arguments
@param argv the actual string parameters for the command line
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        create(core_type type, int argc, char* argv[])
    {
        CoreApp cr(type, argc, argv);
        return cr.getCopyofCorePointer();
    }

    /** create a core from arguments
@details an argument of '--coretype' must be specified to define the type,  otherwise the default
type is used
@param args a vector of reversed command line arguments
@return a pointer to the created core
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core> create(std::vector<std::string> args)
    {
        CoreApp cr(args);
        return cr.getCopyofCorePointer();
    }

    /** create a core from a type and command line arguments
@param type the type of core to create
@param args a vector of reversed command line arguments
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core> create(core_type type,
                                                               std::vector<std::string> args)
    {
        CoreApp cr(type, args);
        return cr.getCopyofCorePointer();
    }

    /** create a core from arguments
@details an argument of '--coretype' must be specified to define the type,  otherwise the default
type is used
@param argc the number of arguments
@param argv the actual argument parameters
@return a pointer to the created core
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core> create(int argc, char* argv[])
    {
        CoreApp cr(argc, argv);
        return cr.getCopyofCorePointer();
    }

    /** create a core from a type, name, and arguments
@param type the type of core to create
@param coreName the name for the core
@param argc the number of arguments
@param argv the actual argument parameters
@return a pointer to the created core
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, int argc, char* argv[])
    {
        CoreApp cr(type, coreName, argc, argv);
        return cr.getCopyofCorePointer();
    }

    /** create a core from a type, name, and arguments
@param type the type of core to create
@param coreName the name for the core
@param args a vector of reversed command line arguments
@return a pointer to the created core
*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        create(core_type type, const std::string& coreName, std::vector<std::string> args)
    {
        CoreApp cr(type, coreName, args);
        return cr.getCopyofCorePointer();
    }

    /** tries to find a named core if it fails it creates a new one
     */
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        FindOrCreate(core_type type,
                     const std::string& coreName,
                     const std::string& initializationString)
    {
        CoreApp cr(type, coreName, initializationString);
        return cr.getCopyofCorePointer();
    }

    /** tries to find a named core if it fails it creates a new one
     */
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, int argc, char* argv[])
    {
        CoreApp cr(type, coreName, argc, argv);
        return cr.getCopyofCorePointer();
    }

    /** tries to find a named core if it fails it creates a new one
     */
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core>
        FindOrCreate(core_type type, const std::string& coreName, std::vector<std::string> args)
    {
        CoreApp cr(type, coreName, args);
        return cr.getCopyofCorePointer();
    }

    /** try to find a joinable core of a specific type*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core> findJoinableCoreOfType(core_type type)
    {
        return nullptr;
    }

    /** locate a registered Core by name
@param name the name of the core to find
@return a shared_ptr to the testCore*/
    HELICS_SHARED_DEPRECATED_CORE std::shared_ptr<Core> findCore(const std::string& name)
    {
        return nullptr;
    }

    /** register a testCore so it can be found by others
@details also cleans up any leftover bCoresrokers that were previously unregistered this can be
controlled by calling cleanUpBrokers earlier if desired
@param core a pointer to a testCore object that should be found globally
@return true if the registration was successful false otherwise*/
    HELICS_SHARED_DEPRECATED_CORE bool registerCore(const std::shared_ptr<Core>& core) {}

    /** remove a Core from the registry
@param name the name of the Core to unregister
*/
    HELICS_SHARED_DEPRECATED_CORE void unregisterCore(const std::string& name) {}
    /** clean up unused cores
@details when Cores are unregistered they get put in a holding area that gets cleaned up when a new
Core is registered or when the clean up function is called this prevents some odd threading issues
@return the number of cores still operating
*/
    HELICS_SHARED_DEPRECATED_CORE size_t cleanUpCores() { return 0; }
    /** clean up unused cores
@details when Cores are unregistered they get put in a holding area that gets cleaned up when a new
Core is registered or when the clean up function is called this prevents some odd threading issues
@param delay the delay time in milliseconds to wait for the cores to finish before destroying
@return the number of cores still operating
*/
    HELICS_SHARED_DEPRECATED_CORE size_t cleanUpCores(std::chrono::milliseconds delay) { return 0; }

    /** make a copy of the core pointer to allow access to the new name
@return true if the copyFromName was found and the copy successful
 */
    HELICS_SHARED_DEPRECATED_CORE bool copyCoreIdentifier(const std::string& copyFromName,
                                                          const std::string& copyToName)
    {
        return false;
    }

}  // namespace CoreFactory

}  // namespace helics
