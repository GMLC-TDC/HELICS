#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
set(
  SRC_FILES
    CoreFactory.cpp
	BrokerFactory.cpp
	BrokerBase.cpp
	CommonCore.cpp
	FederateState.cpp
	PublicationInfo.cpp
	NamedInputInfo.cpp
	InterfaceInfo.cpp
    FilterInfo.cpp
	EndpointInfo.cpp
	ActionMessage.cpp
	CoreBroker.cpp
	core-types.cpp
	core-data.cpp
	TimeCoordinator.cpp
	NetworkCommsInterface.cpp
	ForwardingTimeCoordinator.cpp
	TimeDependencies.cpp
	CommsInterface.cpp
	CommsBroker.cpp
	NetworkBrokerData.cpp
	helics-time.cpp
	HandleManager.cpp
	FilterCoordinator.cpp
	UnknownHandleManager.cpp
	MessageTimer.cpp
	JsonMapBuilder.cpp
	federate_id.cpp
	TimeoutMonitor.cpp

)

set(TESTCORE_SOURCE_FILES test/TestBroker.cpp test/TestCore.cpp test/TestComms.cpp)

set(
  IPC_SOURCE_FILES
	ipc/IpcCore.cpp
	ipc/IpcBroker.cpp
	ipc/IpcComms.cpp
	ipc/IpcQueueHelper.cpp
	#ipc/IpcBlockingPriorityQueue.cpp
	#ipc/IpcBlockingPriorityQueueImpl.cpp
)

set(
  MPI_SOURCE_FILES
	mpi/MpiCore.cpp
	mpi/MpiBroker.cpp
	mpi/MpiComms.cpp
    mpi/MpiService.cpp
)

set(
  ZMQ_SOURCE_FILES
	zmq/ZmqCore.cpp
	zmq/ZmqBroker.cpp
	zmq/ZmqComms.cpp
	zmq/ZmqCommsSS.cpp
	zmq/ZmqRequestSets.cpp
)

set(UDP_SOURCE_FILES udp/UdpCore.cpp udp/UdpBroker.cpp udp/UdpComms.cpp)

set(
  TCP_SOURCE_FILES
	tcp/TcpCore.cpp
	tcp/TcpBroker.cpp
	tcp/TcpComms.cpp
	tcp/TcpCommsSS.cpp
	tcp/TcpHelperClasses.cpp
	tcp/TcpCommsCommon.cpp
)

set(
  PUBLIC_INCLUDE_FILES
  Core.hpp
  Broker.hpp
  CoreFactory.hpp
  BrokerFactory.hpp
  core-exceptions.hpp
  core-types.hpp
  core-data.hpp
  helics-time.hpp
  CoreFederateInfo.hpp
  helicsVersion.hpp
  federate_id.hpp
  helics_definitions.hpp
  helicsCLI11.hpp
)

set(
  INCLUDE_FILES
	BrokerBase.hpp
	TimeDependencies.hpp
	TimeCoordinator.hpp
	ForwardingTimeCoordinator.hpp
	loggingHelper.hpp
	CommsBroker.hpp
	CommsBroker_impl.hpp
	TimeoutMonitor.h
	CoreBroker.hpp
	InterfaceInfo.hpp
	ActionMessageDefintions.hpp
	ActionMessage.hpp
	CommonCore.hpp
	CommsInterface.hpp
	NetworkCommsInterface.hpp
	FederateState.hpp
	PublicationInfo.hpp
	NamedInputInfo.hpp
	EndpointInfo.hpp
	flagOperations.hpp
	BasicHandleInfo.hpp
	FilterInfo.hpp
	FilterCoordinator.hpp
	NetworkBrokerData.hpp
	HandleManager.hpp
	UnknownHandleManager.hpp
	queryHelpers.hpp
	MessageTimer.hpp
	JsonMapBuilder.hpp
	NetworkBroker.hpp
	NetworkCore.hpp
	NetworkBroker_impl.hpp
	NetworkCore_impl.hpp
	fileConnections.hpp
	../helics_enums.h
)

set(TESTCORE_HEADER_FILES test/TestCore.h test/TestBroker.h test/TestComms.h)

set(IPC_HEADER_FILES ipc/IpcCore.h ipc/IpcBroker.h ipc/IpcComms.h ipc/IpcQueueHelper.h
#ipc/IpcBlockingPriorityQueue.hpp
#ipc/IpcBlockingPriorityQueueImpl.hpp
)

set(ZMQ_HEADER_FILES zmq/ZmqCore.h zmq/ZmqBroker.h zmq/ZmqComms.h zmq/ZmqCommsSS.h zmq/ZmqRequestSets.h)

set(MPI_HEADER_FILES mpi/MpiCore.h mpi/MpiBroker.h mpi/MpiComms.h mpi/MpiService.h)

set(UDP_HEADER_FILES udp/UdpCore.h udp/UdpBroker.h udp/UdpComms.h)

set(
  TCP_HEADER_FILES
	tcp/TcpCore.h
	tcp/TcpBroker.h
	tcp/TcpComms.h
	tcp/TcpCommsSS.h
	tcp/TcpHelperClasses.h
	tcp/TcpCommsCommon.h
)

if(ENABLE_TEST_CORE)
list(
  APPEND
    SRC_FILES
    ${TESTCORE_SOURCE_FILES}
)
list(
  APPEND
    INCLUDE_FILES
    ${TESTCORE_HEADER_FILES}
)
endif()

if(ENABLE_UDP_CORE)
  list(APPEND SRC_FILES ${UDP_SOURCE_FILES})
  list(APPEND INCLUDE_FILES ${UDP_HEADER_FILES})
endif()

if(ENABLE_IPC_CORE)
  list(APPEND SRC_FILES ${IPC_SOURCE_FILES})
  list(APPEND INCLUDE_FILES ${IPC_HEADER_FILES})
endif()

if(ENABLE_TCP_CORE)
  list(APPEND SRC_FILES ${TCP_SOURCE_FILES})
  list(APPEND INCLUDE_FILES ${TCP_HEADER_FILES})
endif()

if(ENABLE_ZMQ_CORE)
   list(APPEND SRC_FILES ${ZMQ_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${ZMQ_HEADER_FILES})
endif(ENABLE_ZMQ_CORE)

if(ENABLE_MPI_CORE)
   list(APPEND SRC_FILES ${MPI_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${MPI_HEADER_FILES})
endif(ENABLE_MPI_CORE)

add_library(helics_core OBJECT ${SRC_FILES} ${INCLUDE_FILES} ${PUBLIC_INCLUDE_FILES})

target_include_directories(
  helics_core
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_INCLUDE_DIRECTORIES>
)
target_include_directories(
  helics_core SYSTEM
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
)
target_compile_definitions(
  helics_core PRIVATE
  $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_definitions(
  helics_core PRIVATE BOOST_DATE_TIME_NO_LIB
)
target_compile_options(
  helics_core
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_OPTIONS>
)

if (TARGET Boost::boost)
target_include_directories(
  helics_core SYSTEM
  PRIVATE $<TARGET_PROPERTY:Boost::boost,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
)
endif()

target_include_directories(
  helics_core SYSTEM
  PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(
  helics_core PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_COMPILE_DEFINITIONS>
)

if(ENABLE_MPI_CORE)
	source_group("mpi" FILES ${MPI_SOURCE_FILES} ${MPI_HEADER_FILES})
endif(ENABLE_MPI_CORE)

if(ENABLE_ZMQ_CORE)
  target_include_directories(
    helics_core SYSTEM
    PRIVATE $<TARGET_PROPERTY:${ZeroMQ_DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>
  )
	source_group("zmq" FILES ${ZMQ_SOURCE_FILES} ${ZMQ_HEADER_FILES})
endif(ENABLE_ZMQ_CORE)

if(ENABLE_UDP_CORE)
source_group("udp" FILES ${UDP_SOURCE_FILES} ${UDP_HEADER_FILES})
endif()

if (ENABLE_IPC_CORE)
source_group("ipc" FILES ${IPC_SOURCE_FILES} ${IPC_HEADER_FILES})
endif()

if (ENABLE_TEST_CORE)
source_group("test" FILES ${TESTCORE_SOURCE_FILES} ${TESTCORE_HEADER_FILES})
endif()

if(ENABLE_TCP_CORE)
   source_group("tcp" FILES ${TCP_SOURCE_FILES} ${TCP_HEADER_FILES})
endif()

if(ENABLE_MPI_CORE)
  target_include_directories(
    helics_core
    PRIVATE
      "${MPI_C_INCLUDE_DIR}" "${MPI_C_HEADER_DIR}" "${MPI_C_ADDITIONAL_INCLUDE_DIRS}"
  )
endif(ENABLE_MPI_CORE)

install(
  FILES ${PUBLIC_INCLUDE_FILES}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/core
  COMPONENT headers
)

install(
  FILES ../helics_enums.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
  COMPONENT headers
)
