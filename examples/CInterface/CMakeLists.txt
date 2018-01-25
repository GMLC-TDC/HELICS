##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

include_directories ("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")

add_executable(pi_sender pi_sender.c)
add_executable(pi_receiver pi_receiver.c)

target_link_libraries(pi_sender helicsSharedLib )
target_link_libraries(pi_receiver helicsSharedLib )

add_executable(pi_sender2 pi_sender2.c)
add_executable(pi_receiver2 pi_receiver2.c)

target_link_libraries(pi_sender2 helicsSharedLib )
target_link_libraries(pi_receiver2 helicsSharedLib )

add_executable(nonlings_fed1 nonlings_fed1.c)
add_executable(nonlings_fed2 nonlings_fed2.c)

target_link_libraries(nonlings_fed1 helicsSharedLib )
target_link_libraries(nonlings_fed2 helicsSharedLib )

set_target_properties (pi_sender PROPERTIES
    FOLDER examples)

set_target_properties (pi_sender2 PROPERTIES
    FOLDER examples)
	
set_target_properties (pi_receiver PROPERTIES
    FOLDER examples)

set_target_properties (pi_receiver2 PROPERTIES
    FOLDER examples)

set_target_properties (nonlings_fed1 PROPERTIES
    FOLDER examples)

set_target_properties (nonlings_fed2 PROPERTIES
    FOLDER examples)




if (!MSVC)
target_compile_options(pi_sender PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(pi_receiver PRIVATE -ansi -Wall -pedantic -Werror)

target_compile_options(pi_sender2 PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(pi_receiver2 PRIVATE -ansi -Wall -pedantic -Werror)

target_compile_options(nonlings_fed1 PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(nonlings_fed2 PRIVATE -ansi -Wall -pedantic -Werror)
endif()




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
