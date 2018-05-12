#file to include ZMQ

OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZeroMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${PROJECT_BINARY_DIR}/libs")

SHOW_VARIABLE(ZeroMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${PROJECT_BINARY_DIR}/libs")

set(ZeroMQ_FIND_QUIETLY ON)

find_package(ZeroMQ
	HINTS ${PROJECT_BINARY_DIR}/libs/
	PATH_SUFFIXES 
		cmake/ZeroMQ 
		cmake
		CMake/ZeroMQ)

if (NOT ZeroMQ_FOUND)
	if (ZMQ_USE_STATIC_LIBRARY OR AUTOBUILD_ZMQ)
		include(buildlibZMQ)
		build_libzmq()
		find_package(ZeroMQ
			HINTS ${PROJECT_BINARY_DIR}/libs/
			PATH_SUFFIXES 
				cmake/ZeroMQ 
				cmake
				CMake/ZeroMQ)
	else()
		find_package(ZeroMQ)
		if (NOT ZeroMQ_FOUND)
			OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" ON)
			IF (AUTOBUILD_ZMQ)
				include(buildlibZMQ)
				build_libzmq()
				find_package(ZeroMQ
					HINTS ${PROJECT_BINARY_DIR}/libs/
					PATH_SUFFIXES 
					cmake/ZeroMQ 
					cmake
					CMake/ZeroMQ)
			ENDIF(AUTOBUILD_ZMQ)
		endif()
	endif()
endif()

