##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(SRC_FILES
    CoreFactory.cpp
	BrokerFactory.cpp
	BrokerBase.cpp
    TestCore.cpp
	CommonCore.cpp
	FederateState.cpp
	SubscriptionInfo.cpp
	FilterInfo.cpp
	EndpointInfo.cpp
	ActionMessage.cpp
	CoreBroker.cpp
	TestBroker.cpp
	core-data.cpp
	ipc/IpcCore.cpp
	ipc/IpcBroker.cpp
	ipc/IpcComms.cpp
	ipc/IpcQueueHelper.cpp
	TimeCoordinator.cpp
	TimeDependencies.cpp
	CommsInterface.cpp
	CommsBroker.cpp
	argParser.cpp
)

set (MPI_SOURCE_FILES
	mpi/MpiCore.cpp
	mpi/MpiBroker.cpp
	mpi/MpiComms.cpp
)

set (ZMQ_SOURCE_FILES
	zmq/ZmqCore.cpp
	zmq/ZmqBroker.cpp
	zmq/ZmqComms.cpp
	zmq/ZmqRequestSets.cpp
)

set(INCLUDE_FILES
    core-data.h
	BrokerBase.h
    CoreFactory.h
	BrokerFactory.h
	TimeDependencies.h
	core-exceptions.h
	TimeCoordinator.h
	loggingHelper.hpp
	delayedObjects.hpp
	CommsBroker.hpp
	CommsBroker_impl.hpp
    core.h
    core-types.h
    helics-time.h
    TestCore.h
	CoreBroker.h
	ActionMessageDefintions.h
	ActionMessage.h
	CommonCore.h
	ipc/IpcCore.h
	ipc/IpcBroker.h
	ipc/IpcComms.h
	ipc/IpcQueueHelper.h
    
	CommsInterface.h
	FederateState.h
	SubscriptionInfo.h
	PublicationInfo.h
	EndpointInfo.h
	FilterInfo.h
	BasicHandleInfo.h
	FilterFunctions.h
	TestBroker.h
	argParser.h
)

set(ZMQ_HEADER_FILES
	zmq/ZmqCore.h
	zmq/ZmqBroker.h
	zmq/ZmqComms.h
	zmq/ZmqRequestSets.h
)

set(MPI_HEADER_FILES
	mpi/MpiCore.h
	mpi/MpiBroker.h
	mpi/MpiComms.h
	)

if (${ZMQ_FOUND})
   list(APPEND SRC_FILES ${ZMQ_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${ZMQ_HEADER_FILES})
endif (${ZMQ_FOUND})

if (${MPI_C_FOUND})
   list(APPEND SRC_FILES ${MPI_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${MPI_HEADER_FILES})
endif (${MPI_C_FOUND})

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
add_library(helics_core OBJECT ${SRC_FILES} ${INCLUDE_FILES})


include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)

OPTION(BUILD_BROKER "Build a helics Broker" ON)
if (BUILD_BROKER)
link_directories(${external_link_directories})
add_executable(helics_broker helics-broker.cc)
   target_link_libraries(helics_broker helics)
   target_link_libraries(helics_broker ${external_library_list})
   
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


