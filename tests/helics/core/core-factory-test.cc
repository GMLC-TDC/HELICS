/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/core/core-factory.h"

#include <iostream>

int main (int argc, char** argv)
{
  bool passed = true;

  std::cout << "Core Factory Test" << std::endl;

#if HELICS_HAVE_ZEROMQ
  const bool haveZmq = true;
#else
  const bool haveZmq = false;
#endif

  const bool foundZmq = helics::CoreFactory::available (HELICS_ZMQ);
  passed &= (haveZmq == foundZmq);
  if (haveZmq == foundZmq)
    {
      std::cout << "ZeroMQ available : " <<  foundZmq << std::endl;
    } else
    {
      std::cout << "Failed ZeroMQ available : " <<  foundZmq << " should be : " << haveZmq << std::endl;
    }

#if HELICS_HAVE_MPI
  const bool haveMpi = true;
#else
  const bool haveMpi = false;
#endif

  const bool foundMpi = helics::CoreFactory::available (HELICS_MPI);
  passed &= (haveMpi == foundMpi);
  if (haveMpi == foundMpi)
    {
      std::cout << "MPI available : " <<  foundMpi << std::endl;
    } else
    {
      std::cout << "Failed MPI available : " <<  foundMpi << " should be : " << haveMpi << std::endl;
    }

#if HELICS_HAVE_MPI
  if (haveMpi)
    {
      const char *initializationString = "";
      helics::Core* core = helics::CoreFactory::create (HELICS_MPI, initializationString);

      if(core)
        {
          std::cout << "Created MPI Core" << std::endl;
        } else
        {
          std::cout << "Failed to create MPI Core" << std::endl;
          passed &= false;
        }
    }
#endif // HELICS_HAVE_MPI

  return !passed;
}

