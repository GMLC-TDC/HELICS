##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
set(SRC_FILES
    CoreFactory.cpp
	BrokerFactory.cpp
	BrokerBase.cpp
	CommonCore.cpp
	FederateState.cpp
	SubscriptionInfo.cpp
	PublicationInfo.cpp
	InterfaceInfo.cpp
	#FilterInfo.cpp
	EndpointInfo.cpp
	ActionMessage.cpp
	CoreBroker.cpp
	core-types.cpp
	core-data.cpp
	TimeCoordinator.cpp
	ForwardingTimeCoordinator.cpp
	TimeDependencies.cpp
	CommsInterface.cpp
	CommsBroker.cpp
	NetworkBrokerData.cpp
	helics-time.cpp
	HandleManager.cpp
	HandlePointerManager.cpp
)

set(TESTCORE_SOURCE_FILES
	TestBroker.cpp
	TestCore.cpp
)

set (IPC_SOURCE_FILES
	ipc/IpcCore.cpp
	ipc/IpcBroker.cpp
	ipc/IpcComms.cpp
	ipc/IpcQueueHelper.cpp
)

set (MPI_SOURCE_FILES
	mpi/MpiCore.cpp
	mpi/MpiBroker.cpp
	mpi/MpiComms.cpp
    mpi/MpiService.cpp
)

set (ZMQ_SOURCE_FILES
	zmq/ZmqCore.cpp
	zmq/ZmqBroker.cpp
	zmq/ZmqComms.cpp
	zmq/ZmqRequestSets.cpp
)

set (UDP_SOURCE_FILES
	udp/UdpCore.cpp
	udp/UdpBroker.cpp
	udp/UdpComms.cpp
)

set (TCP_SOURCE_FILES
	tcp/TcpCore.cpp
	tcp/TcpBroker.cpp
	tcp/TcpComms.cpp
	tcp/TcpHelperClasses.cpp
)

set (PUBLIC_INCLUDE_FILES
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
../flag-definitions.h
)

set(INCLUDE_FILES
	BrokerBase.hpp
	TimeDependencies.hpp
	TimeCoordinator.hpp
	ForwardingTimeCoordinator.hpp
	loggingHelper.hpp
	CommsBroker.hpp
	CommsBroker_impl.hpp
	CoreBroker.hpp
	InterfaceInfo.hpp
	ActionMessageDefintions.hpp
	ActionMessage.hpp
	CommonCore.hpp
	CommsInterface.hpp
	FederateState.hpp
	SubscriptionInfo.hpp
	PublicationInfo.hpp
	EndpointInfo.hpp
	FilterInfo.hpp
	fmt_wrapper.h
	BasicHandleInfo.hpp
	FilterFunctions.hpp
	NetworkBrokerData.hpp
	HandleManager.hpp
	HandlePointerManager.hpp
)

set(TESTCORE_HEADER_FILES
 TestCore.h
 TestBroker.h
)

set(IPC_HEADER_FILES
	ipc/IpcCore.h
	ipc/IpcBroker.h
	ipc/IpcComms.h
	ipc/IpcQueueHelper.h
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
    mpi/MpiService.h
	)

set(UDP_HEADER_FILES
	udp/UdpCore.h
	udp/UdpBroker.h
	udp/UdpComms.h
	)

set(TCP_HEADER_FILES
	tcp/TcpCore.h
	tcp/TcpBroker.h
	tcp/TcpComms.h
	tcp/TcpHelperClasses.h
	)

	list(APPEND SRC_FILES ${TESTCORE_SOURCE_FILES} ${UDP_SOURCE_FILES} ${IPC_SOURCE_FILES})
	list(APPEND INCLUDE_FILES ${TESTCORE_HEADER_FILES} ${UDP_HEADER_FILES} ${IPC_HEADER_FILES})

	if (NOT DISABLE_TCP_CORE)
		list(APPEND SRC_FILES ${TCP_SOURCE_FILES})
		list(APPEND INCLUDE_FILES ${TCP_HEADER_FILES} )
	endif()


if (HELICS_HAVE_ZEROMQ)
   list(APPEND SRC_FILES ${ZMQ_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${ZMQ_HEADER_FILES})
endif (HELICS_HAVE_ZEROMQ)

if (HELICS_HAVE_MPI)
   list(APPEND SRC_FILES ${MPI_SOURCE_FILES})
   list(APPEND INCLUDE_FILES ${MPI_HEADER_FILES})
endif (HELICS_HAVE_MPI)

add_library(helics_core OBJECT ${SRC_FILES} ${INCLUDE_FILES} ${PUBLIC_INCLUDE_FILES})

target_include_directories(helics_core PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(helics_core SYSTEM PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>)
target_compile_definitions(helics_core PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_DEFINITIONS>)
target_compile_options(helics_core PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_OPTIONS>)

target_include_directories(helics_core SYSTEM PRIVATE $<TARGET_PROPERTY:fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(helics_core PRIVATE $<TARGET_PROPERTY:fmt-header-only,INTERFACE_COMPILE_DEFINITIONS>)

if (APPLE)
	target_compile_definitions(helics_core PUBLIC "-DSTX_NO_STD_ANY=1")
endif(APPLE)

if (HELICS_HAVE_MPI)
	source_group("mpi" FILES ${MPI_SOURCE_FILES} ${MPI_HEADER_FILES})
endif (HELICS_HAVE_MPI)

if (HELICS_HAVE_ZEROMQ)
	target_include_directories(helics_core SYSTEM PRIVATE $<TARGET_PROPERTY:${ZeroMQ_DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>)
	source_group("zmq" FILES ${ZMQ_SOURCE_FILES} ${ZMQ_HEADER_FILES})
endif(HELICS_HAVE_ZEROMQ)

source_group("udp" FILES ${UDP_SOURCE_FILES} ${UDP_HEADER_FILES})
source_group("ipc" FILES ${IPC_SOURCE_FILES} ${IPC_HEADER_FILES})
source_group("test" FILES ${TESTCORE_SOURCE_FILES} ${TESTCORE_HEADER_FILES})

if (NOT DISABLE_TCP_CORE)
   source_group("tcp" FILES ${TCP_SOURCE_FILES} ${TCP_HEADER_FILES})
endif()

if (HELICS_HAVE_MPI)
   target_include_directories(helics_core PRIVATE "${MPI_C_INCLUDE_DIR}" "${MPI_C_HEADER_DIR}"  "${MPI_C_ADDITIONAL_INCLUDE_DIRS}")
endif(HELICS_HAVE_MPI)

install (FILES ${PUBLIC_INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/core COMPONENT headers)
