##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

include_directories ("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")

add_executable(pi_sender pi_sender.c)
add_executable(pi_receiver pi_receiver.c)

target_link_libraries(pi_sender helicsSharedLib  ${external_library_list})
target_link_libraries(pi_receiver helicsSharedLib  ${external_library_list})

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET pi_sender POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:pi_sender>/")                 # <--this is out-file path
endforeach(keyfile)

add_custom_command(TARGET pi_sender POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
        "$<TARGET_FILE_DIR:pi_sender>/")                 # <--this is out-file path