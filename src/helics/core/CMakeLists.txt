##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(SRC_FILES
    core-factory.cc
    test-core.cc
    mpi/mpi-core.cc
    zmq/zmq-core.cc
    zmq/zmq-helper.cc
)
set(INCLUDE_FILES
    core-c.h
    core-data.h
    core-factory.h
    core.h
    core-types.h
    helics-time.h
    test-core.h
    mpi/mpi-core.h
    zmq/zmq-core.h
    zmq/zmq-fedstate.h
    zmq/zmq-handle.h
    zmq/zmq-helper.h
)

add_library(helics_core OBJECT ${SRC_FILES} ${INCLUDE_FILES})


include_directories(${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)

if (${ZMQ_FOUND})
OPTION(BUILD_BROKER "Build a helics Broker" ON)
if (BUILD_BROKER)
   add_executable(helics_broker zmq/zmq-broker.cc)
   target_link_libraries(helics_broker helics)

   set(HELICS_BROKER_LOC ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "build folder location of the broker")
   #message(STATUS "bloc ${CMAKE_CURRENT_BINARY_DIR}")
   INSTALL(TARGETS helics_broker DESTINATION bin)
   
   foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET helics_broker POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:helics_broker>/")                 # <--this is out-file path
endforeach(keyfile)
endif()

endif (${ZMQ_FOUND})

