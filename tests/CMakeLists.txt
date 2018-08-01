#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

if(BOOST_VERSION_LEVEL GREATER 0)
  add_subdirectory(helics)
else()
  message(
    WARNING
      "Boost version >=1.61 required for building HELICS tests (Found Boost version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION})"
    )
endif()

if(BUILD_PYTHON_INTERFACE)
  add_subdirectory(python_helics)
endif()

if(BUILD_JAVA_INTERFACE)
  add_subdirectory(java_helics)
endif()

if(BUILD_MATLAB_INTERFACE)
  add_subdirectory(matlab_helics)
endif()

if(BUILD_OCTAVE_INTERFACE)
  # add_subdirectory(octave_helics)
endif()
