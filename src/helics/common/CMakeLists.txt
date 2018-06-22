##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

set(common_headers
AirLock.hpp
simpleQueue.hpp
BlockingQueue.hpp
BlockingPriorityQueue.hpp
timeRepresentation.hpp
delayedDestructor.hpp
DelayedObjects.hpp
searchableObjectHolder.hpp
argParser.h
JsonProcessingFunctions.hpp
stringToCmdLine.h
AsioServiceManager.h
logger.h
stringOps.h
MapTraits.hpp
GuardedTypes.hpp
generic_string_ops.hpp
charMapper.h
base64.h
MappedVector.hpp
DualMappedVector.hpp
MappedPointerVector.hpp
DualMappedPointerVector.hpp
TripWire.hpp
)

set(common_sources
stringToCmdLine.cpp
AsioServiceManager.cpp
JsonProcessingFunctions.cpp
logger.cpp
stringOps.cpp
charMapper.cpp
base64.cpp
argParser.cpp
TripWire.cpp
)

set(zmq_headers
	#zmqReactor.h
	zmqContextManager.h
	zmqHelper.h
	#zmqSocketDescriptor.h
	#zmqProxyHub.h
	cppzmq/zmq.hpp
	cppzmq/zmq_addon.hpp
	)

set(zmq_sources
	#zmqReactor.cpp
	zmqContextManager.cpp
	#zmqSocketDescriptor.cpp
	zmqHelper.cpp
	#zmqProxyHub.cpp
)

#headers that are part of the public interface
set (helics_public_common
timeRepresentation.hpp
argParser.h
JsonProcessingFunctions.hpp
stringToCmdLine.h
AsioServiceManager.h
logger.h
stringOps.h
MapTraits.hpp
GuardedTypes.hpp
generic_string_ops.hpp
)

if (HELICS_HAVE_ZEROMQ)
   list(APPEND common_headers ${zmq_headers})
   list(APPEND common_sources ${zmq_sources})
endif (HELICS_HAVE_ZEROMQ)

list(APPEND common_sources ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp.cpp)

add_library(helics_common OBJECT ${common_sources} ${common_headers})
set_target_properties(helics_common PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(helics_common PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(helics_common SYSTEM PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>)
target_compile_definitions(helics_common PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_DEFINITIONS>)

target_compile_options(helics_common PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_OPTIONS>)

if (HELICS_HAVE_ZEROMQ)
	target_include_directories(helics_common SYSTEM PRIVATE $<TARGET_PROPERTY:${ZeroMQ_DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>)
endif (HELICS_HAVE_ZEROMQ)

if (APPLE)
	target_compile_definitions(helics_common PUBLIC "-DSTX_NO_STD_ANY=1")
endif(APPLE)

install(FILES ${helics_public_common} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/common COMPONENT headers)

