##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

add_subdirectory(common)

add_subdirectory (core)
add_subdirectory(application_api)

if (BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    add_subdirectory(shared_library)
endif()

if(BUILD_PLAYER OR BUILD_RECORDER OR BUILD_APPS)
    add_subdirectory(apps)
endif()

