##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

IF(BUILD_PLAYER OR BUILD_RECORDER)
add_subdirectory(player)
ENDIF()

if (BUILD_C_SHARED_LIB OR BUILD_PYTHON)
add_subdirectory(shared_api_library)
endif()