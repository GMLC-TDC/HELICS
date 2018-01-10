##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################


set(travis_test_headers
testFixtures.h
)

set(travis_test_sources
travis-tests.cpp
test-travis-value-federate.cpp
test-travis-message-federate.cpp
test-travis-filters.cpp
testFixtures.cpp
)


include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(travis-tests ${travis_test_sources} ${travis_test_headers})
target_link_libraries(travis-tests helics-static)

target_link_libraries(travis-tests ${Boost_LIBRARIES_test})

add_test(NAME travis-tests COMMAND travis-tests --log_level=success --report_level=detailed)

set_target_properties (travis-tests PROPERTIES FOLDER tests)


foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET travis-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:travis-tests>/")                 # <--this is out-file path
endforeach(keyfile)

