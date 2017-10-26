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

link_directories(${external_link_directories})

include_directories(SYSTEM ${Boost_INCLUDE_DIR})

target_link_libraries(player-recorder-tests helics)

target_link_libraries(player-recorder-tests ${external_library_list})

target_link_libraries(player-recorder-tests${Boost_LIBRARIES_test})
link_directories(${external_link_directories})

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)

add_test(NAME player-recorder-tests COMMAND player-recorder-tests)


target_link_libraries(player-recorder-tests Threads::Threads)
IF (UNIX AND NOT APPLE)
target_link_libraries(player-recorder-tests rt)
ENDIF()

add_definitions(-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")



add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_player>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
		
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_recorder>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
		
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_broker>"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")    

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET player-recorder-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:player-recorder-tests>/")                 # <--this is out-file path
endforeach(keyfile)