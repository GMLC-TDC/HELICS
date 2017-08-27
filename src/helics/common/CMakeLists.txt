##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

set(common_headers
simpleQueue.hpp
BlockingQueue.hpp
timeRepresentation.hpp
blocking_queue.h
stringToCmdLine.h
)

set(common_sources
stringToCmdLine.cpp
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
)

if (${ZMQ_FOUND})
   list(APPEND common_headers ${zmq_headers})
   list(APPEND common_sources ${zmq_sources})
endif (${ZMQ_FOUND})

add_library(helics_common OBJECT ${common_sources} ${common_headers})
set_target_properties(helics_common PROPERTIES LINKER_LANGUAGE CXX)

install (FILES ${common_headers} DESTINATION include/helics/common)
