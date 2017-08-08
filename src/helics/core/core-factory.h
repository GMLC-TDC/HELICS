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
  static std::unique_ptr<Core> create (helics_core_type type, const std::string &initializationString);

  /**
   * Returns true if type specified is available in current compilation.
   */
  static bool available (helics_core_type type);
};

} // namespace helics

#endif
