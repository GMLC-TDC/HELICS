##############################################################################
#Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

include_directories ("${PROJECT_SOURCE_DIR}/src/helics")
include_directories ("${PROJECT_SOURCE_DIR}/src/helics/cpp98")

add_executable(pi_sender_cpp pi_sender.cpp)
add_executable(pi_receiver_cpp pi_receiver.cpp)

target_link_libraries(pi_sender_cpp helicsCpp98 )
target_link_libraries(pi_receiver_cpp helicsCpp98 )

add_executable(pi_sender2_cpp pi_sender2.cpp)
add_executable(pi_receiver2_cpp pi_receiver2.cpp)

target_link_libraries(pi_sender2_cpp helicsCpp98 )
target_link_libraries(pi_receiver2_cpp helicsCpp98 )

add_executable(nonlings_fed1_cpp nonlings_fed1.cpp)
add_executable(nonlings_fed2_cpp nonlings_fed2.cpp)

target_link_libraries(nonlings_fed1_cpp helicsCpp98 )
target_link_libraries(nonlings_fed2_cpp helicsCpp98 )

if (!MSVC)
target_compile_options(pi_sender_cpp PRIVATE -std=c++98 -pedantic -Werror)
target_compile_options(pi_receiver_cpp PRIVATE -std=c++98 -pedantic -Werror)

target_compile_options(pi_sender2_cpp PRIVATE -std=c++98 -pedantic -Werror)
target_compile_options(pi_receiver2_cpp PRIVATE -std=c++98 -pedantic -Werror)

target_compile_options(nonlings_fed1_cpp PRIVATE -std=c++98 -pedantic -Werror)
target_compile_options(nonlings_fed2_cpp PRIVATE -std=c++98 -pedantic -Werror)
endif()

set_target_properties (pi_sender_cpp PROPERTIES
    FOLDER examples)

set_target_properties (pi_sender2_cpp PROPERTIES
    FOLDER examples)

set_target_properties (pi_receiver_cpp PROPERTIES
    FOLDER examples)

set_target_properties (pi_receiver2_cpp PROPERTIES
    FOLDER examples)

set_target_properties (nonlings_fed1_cpp PROPERTIES
    FOLDER examples)

set_target_properties (nonlings_fed2_cpp PROPERTIES
    FOLDER examples)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET pi_sender_cpp POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:pi_sender_cpp>/")                 # <--this is out-file path
endforeach(keyfile)

add_custom_command(TARGET pi_sender_cpp POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
        "$<TARGET_FILE_DIR:pi_sender_cpp>/")                 # <--this is out-file path

