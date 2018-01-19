##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

set(common_headers
simpleQueue.hpp
BlockingQueue.hpp
BlockingPriorityQueue.hpp
timeRepresentation.hpp
delayedDestructor.hpp
searchableObjectHolder.hpp
stringToCmdLine.h
AsioServiceManager.h
logger.h
stringOps.h
charMapper.h
generic_string_ops.hpp
base64.h
argParser.h
MappedVector.hpp
MappedPointerVector.hpp
DualMappedVector.hpp

)

set(common_sources
stringToCmdLine.cpp
AsioServiceManager.cpp
logger.cpp
stringOps.cpp
charMapper.cpp
base64.cpp
argParser.cpp
)

set(zmq_headers
	zmqReactor.h
	zmqContextManager.h
	zmqHelper.h
	zmqSocketDescriptor.h
	zmqProxyHub.h
	cppzmq/zmq.hpp
	cppzmq/zmq_addon.hpp
	)

set(zmq_sources
	zmqReactor.cpp
	zmqContextManager.cpp
	zmqSocketDescriptor.cpp
	zmqHelper.cpp
	zmqProxyHub.cpp
)

set (helics_public_common
timeRepresentation.hpp
)

if (${ZeroMQ_FOUND})
   list(APPEND common_headers ${zmq_headers})
   list(APPEND common_sources ${zmq_sources})
endif (${ZeroMQ_FOUND})

list(APPEND common_sources ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp.cpp)

add_library(helics_common OBJECT ${common_sources} ${common_headers})
set_target_properties(helics_common PROPERTIES LINKER_LANGUAGE CXX)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})

install(FILES ${helics_public_common} DESTINATION include/helics/common)

