##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

#-----------------------------------------------------------------------------
# player recorder using Boost
#-----------------------------------------------------------------------------


set(recplayer_test_sources
player_recorder_tests.cpp
multi_player_tests.cpp
multi_tests.cpp
player_tests.cpp
recorder_tests.cpp
exeTestHelper.cpp
exeTestHelper.h
)


add_executable(player-recorder-tests ${recplayer_test_sources})

set_target_properties (player-recorder-tests PROPERTIES FOLDER tests)


include_directories(SYSTEM ${Boost_INCLUDE_DIR})

target_link_libraries(player-recorder-tests helics-static)

target_link_libraries(player-recorder-tests ${Boost_LIBRARIES_test})



include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)

add_test(NAME player-recorder-tests COMMAND player-recorder-tests --log_level=warning --report_level=detailed)


add_definitions(-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")


if (BUILD_PLAYER)
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_player>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
endif()

if (BUILD_RECORDER)
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_recorder>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
endif()

if (BUILD_BROKER)
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_broker>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")
endif()

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
endforeach(keyfile)
