##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

include_directories ("${PROJECT_BINARY_DIR}/include/shared_api_library")

add_executable(pi_sender pi_sender.c)
add_executable(pi_receiver pi_receiver.c)

target_link_libraries(pi_sender helicsSharedLib  ${external_library_list})
target_link_libraries(pi_receiver helicsSharedLib  ${external_library_list})

