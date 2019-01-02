#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

if(BOOST_VERSION_LEVEL GREATER 60)
  add_subdirectory(helics)
else()
  message(
    WARNING
      "Boost version >=1.61 required for building HELICS tests (Found Boost version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION})"
  )
endif()

if(BUILD_PYTHON_INTERFACE)
  add_subdirectory(python)
endif()

if(BUILD_JAVA_INTERFACE)
  add_subdirectory(java)
endif()

if(BUILD_MATLAB_INTERFACE)
  add_subdirectory(matlab)
endif()

if(BUILD_OCTAVE_INTERFACE)
   add_subdirectory(octave)
endif()

if(BUILD_CSHARP_INTERFACE)
   #add_subdirectory(csharp)
endif()
