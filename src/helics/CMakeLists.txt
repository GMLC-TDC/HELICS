##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, 
operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory,
operated by the Alliance for Sustainable Energy, LLC; 
and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

install(FILES flag-definitions.h helics.hpp DESTINATION include/helics)

IF(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
add_subdirectory(apps)
ENDIF()

if (BUILD_C_SHARED_LIB OR BUILD_PYTHON OR BUILD_MATLAB OR BUILD_JAVA)
add_subdirectory(shared_api_library)
add_subdirectory(cpp98)
install(FILES chelics.h DESTINATION include/helics)
endif()
