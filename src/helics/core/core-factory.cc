/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/core/core-factory.h"
#include "helics/core/core-types.h"

#if HELICS_HAVE_ZEROMQ
#include "helics/core/zmq/zmq-core.h"
#endif

#if HELICS_HAVE_MPI
#include "helics/core/mpi/mpi-core.h"
#endif

#include "helics/core/test-core.h"

#include <cassert>

namespace helics {

std::unique_ptr<Core> CoreFactory::create(helics_core_type type, const std::string &initializationString) {

  std::unique_ptr<Core> core;

  switch(type)
    {
    case HELICS_ZMQ:
      {
#if HELICS_HAVE_ZEROMQ
        core = std::make_unique<ZeroMQCore> ();
#else
        assert (false);
#endif
        break;
      }
    case HELICS_MPI:
      {
#if HELICS_HAVE_MPI
        core = std::make_unique<MpiCore> ();
#else
        assert (false);
#endif
        break;
      }
    case HELICS_TEST:
      {
        core = std::make_unique<TestCore> ();
        break;
      }
    default:
      assert (false);
    }

  core->initialize (initializationString);

  return core;
}

bool CoreFactory::available (helics_core_type type) {

  bool available = false;

  switch(type)
    {
    case HELICS_ZMQ:
      {
#if HELICS_HAVE_ZEROMQ
        available = true;
#endif
        break;
      }
    case HELICS_MPI:
      {
#if HELICS_HAVE_MPI
        available = true;
#endif
        break;
      }
    case HELICS_TEST:
      {
        available = true;
        break;
      }
    default:
      assert (false);
    }

  return available;
}

} // namespace 

