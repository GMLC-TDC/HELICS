##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

add_subdirectory(player)

install(DIRECTORY common DESTINATION include/helics
        FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY core DESTINATION include/helics
        FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY application_api DESTINATION include/helics
        FILES_MATCHING PATTERN "*.h*")

if (BUILD_SHARED_LIBS)
add_subdirectory(shared_api_library)
install(DIRECTORY shared_api_library DESTINATION include
        FILES_MATCHING PATTERN "*.h*")
endif(BUILD_SHARED_LIBS)