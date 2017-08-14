/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_CORE_FACTORY_
#define _HELICS_CORE_FACTORY_


#include "helics/core/core-types.h"
#include <memory>
#include <string>

namespace helics {

class Core;
class CommonCore;
/**
 * Factory for building Core API instances.
 */
class CoreFactory {
public:
  /**
   * Creates a Core API object of the specified type.
   *
   * Invokes initialize() on the instantiated Core object.
   */
  static std::shared_ptr<Core> create (helics_core_type type, const std::string &initializationString);

  static std::shared_ptr<Core> create(helics_core_type type, const std::string &coreName, const std::string &initializationString);

  /**
   * Returns true if type specified is available in current compilation.
   */
  static bool available (helics_core_type type);
};



/** container for building a bunch of cores and brokers accessible in local memory*/
/** locate a Core by name
@param name the name of the core to find
@return a shared_ptr to the testCore*/
std::shared_ptr<CommonCore> findCore(const std::string &name);

/** register a testCore so it can be found by others
@param tcore a pointer to a testCore object that should be found globally
@return true if the registration was successful false otherwise*/
bool registerCore(std::shared_ptr<CommonCore> tcore);

/** remove a Core from the registry
@param name the name of the Core to unregister
*/
void unregisterCore(const std::string &name);

} // namespace helics

#endif
