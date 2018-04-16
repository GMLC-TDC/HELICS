##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

add_executable(hello_world_sender hello_world/hello_world_sender.c)
add_executable(hello_world_receiver hello_world/hello_world_receiver.c)

target_link_libraries(hello_world_sender helicsSharedLib )
target_link_libraries(hello_world_receiver helicsSharedLib )

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

set_target_properties (hello_world_sender PROPERTIES
    FOLDER examples)

set_target_properties (hello_world_receiver PROPERTIES
    FOLDER examples)


if (!MSVC)
target_compile_options(pi_sender PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(pi_receiver PRIVATE -ansi -Wall -pedantic -Werror)

target_compile_options(pi_sender2 PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(pi_receiver2 PRIVATE -ansi -Wall -pedantic -Werror)

target_compile_options(nonlings_fed1 PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(nonlings_fed2 PRIVATE -ansi -Wall -pedantic -Werror)

target_compile_options(hello_world_sender PRIVATE -ansi -Wall -pedantic -Werror)
target_compile_options(hello_world_receiver PRIVATE -ansi -Wall -pedantic -Werror)
endif()

add_subdirectory(messageFed1)
add_subdirectory(comboFederates1)

if (BUILD_HELICS_EXAMPLES AND BUILD_HELICS_TESTS AND NOT MSVC)
    add_test(NAME pi_example
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 $<TARGET_FILE:pi_sender> $<TARGET_FILE:pi_receiver>)
    add_test(NAME pi_example2
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 $<TARGET_FILE:pi_sender2> $<TARGET_FILE:pi_receiver2>)
    add_test(NAME nonlings_example
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 $<TARGET_FILE:nonlings_fed1> $<TARGET_FILE:nonlings_fed2>)
    add_test(NAME hello_world_example
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 $<TARGET_FILE:hello_world_sender> $<TARGET_FILE:hello_world_receiver>)
    set_tests_properties(pi_example pi_example2 nonlings_example hello_world_example PROPERTIES LABELS Examples)
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

