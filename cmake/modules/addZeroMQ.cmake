#file to include ZMQ 

OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZeroMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${PROJECT_BINARY_DIR}/libs")

SHOW_VARIABLE(ZeroMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${PROJECT_BINARY_DIR}/libs")

set(ZeroMQ_FIND_QUIETLY ON)
if (EXISTS ${PROJECT_BINARY_DIR}/libs/cmake/ZeroMQ/ZeroMQConfig.cmake)
	include(${PROJECT_BINARY_DIR}/libs/cmake/ZeroMQ/ZeroMQConfig.cmake)
	set(ZeroMQ_FOUND 1)
else()
	if (ZMQ_USE_STATIC_LIBRARY)
		include(buildlibZMQ)
		build_libzmq()
		set(ZeroMQ_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
		include(${PROJECT_BINARY_DIR}/libs/cmake/ZeroMQ/ZeroMQConfig.cmake)
		set(ZeroMQ_FOUND 1)
	else()
		find_package(ZeroMQ)
		if (NOT ZeroMQ_INCLUDE_DIR)
			OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" ON)
			IF (AUTOBUILD_ZMQ)
				include(buildlibZMQ)
				build_libzmq()
				set(ZeroMQ_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
				include(${PROJECT_BINARY_DIR}/libs/cmake/ZeroMQ/ZeroMQConfig.cmake)
				set(ZeroMQ_FOUND 1)
			ENDIF(AUTOBUILD_ZMQ)
		endif(NOT ZeroMQ_INCLUDE_DIR)
	endif()
endif()

	
	
