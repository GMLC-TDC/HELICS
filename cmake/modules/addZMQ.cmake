#file to include ZMQ 

OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${ZMQ_LIBRARY_PATH}")

SHOW_VARIABLE(ZMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${ZMQ_INCLUDE_PATH}")

set(ZMQ_FIND_QUIETLY ON)
	if (EXISTS ${ZMQ_LIBRARY_PATH}/cmake/ZeroMQ/ZeroMQConfig.cmake)
		include(${ZMQ_LIBRARY_PATH}/cmake/ZeroMQ/ZeroMQConfig.cmake)
	else()
		find_package(ZMQ)
	endif()

	if (NOT ZMQ_FOUND)
		OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" ON)
		IF (AUTOBUILD_ZMQ)
			include(buildlibZMQ)
			build_libzmq()
			set(ZMQ_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
			include(${ZMQ_LIBRARY_PATH}/cmake/ZeroMQ/ZeroMQConfig.cmake)
		ENDIF(AUTOBUILD_ZMQ)

	endif(NOT ZMQ_FOUND)
	
