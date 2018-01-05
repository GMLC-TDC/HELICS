##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

#-----------------------------------------------------------------------------
# player recorder using Boost
#-----------------------------------------------------------------------------


set(special_federate_test_sources
player_recorder_tests.cpp
multi_player_tests.cpp
multi_tests.cpp
player_tests.cpp
recorder_tests.cpp
exeTestHelper.cpp
exeTestHelper.h
)


add_executable(special-federate-tests ${special_federate_test_sources})


include_directories(SYSTEM ${Boost_INCLUDE_DIR})

target_link_libraries(special-federate-tests special-federates-static helics-static)

target_link_libraries(special-federate-tests ${Boost_LIBRARIES_test})



include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_test(NAME special-federate-tests COMMAND special-federate-tests --log_level=warning --report_level=short)


add_definitions(-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")


if (BUILD_PLAYER)
add_custom_command(TARGET special-federate-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_player>"      # <--this is in-file
        "$<TARGET_FILE_DIR:special-federate-tests>/")                 # <--this is out-file path
endif()

if (BUILD_RECORDER)
add_custom_command(TARGET special-federate-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_recorder>"      # <--this is in-file
        "$<TARGET_FILE_DIR:special-federate-tests>/")                 # <--this is out-file path
endif()

if (BUILD_BROKER)
add_custom_command(TARGET special-federate-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helics_broker>"      # <--this is in-file
        "$<TARGET_FILE_DIR:special-federate-tests>/")
endif()

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET special-federate-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:special-federate-tests>/")                 # <--this is out-file path
endforeach(keyfile)
