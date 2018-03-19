##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

install(FILES flag-definitions.h helics.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics COMPONENT headers)

IF(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
add_subdirectory(apps)
ENDIF()

if (BUILD_C_SHARED_LIB OR BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE)
add_subdirectory(shared_api_library)
add_subdirectory(cpp98)
install(FILES chelics.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics COMPONENT headers)
endif()

