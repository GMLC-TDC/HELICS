#file to include ZMQ

OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZeroMQ_INSTALL_PATH PATH
  "path to the zmq libraries" "${AUTOBUILD_INSTALL_PATH}")

set(ZMQ_CMAKE_SUFFIXES 
	cmake/ZeroMQ 
	cmake
	CMake/ZeroMQ
	lib/cmake)
	
find_package(ZeroMQ QUIET
	HINTS 
		${ZeroMQ_INSTALL_PATH}
		${AUTOBUILD_INSTALL_PATH}
	PATH_SUFFIXES ${ZMQ_CMAKE_SUFFIXES}
	)

if (NOT ZeroMQ_FOUND)
	message(STATUS "initialZMQ not found")
	if (ZMQ_USE_STATIC_LIBRARY OR AUTOBUILD_ZMQ)
		include(buildlibZMQ)
		build_libzmq()
		find_package(ZeroMQ
			HINTS 
				${ZeroMQ_INSTALL_PATH}
				${AUTOBUILD_INSTALL_PATH}
			PATH_SUFFIXES ${ZMQ_CMAKE_SUFFIXES}
			)
	else()
		set(ZeroMQ_FIND_QUIETLY ON)
		find_package(ZeroMQ)
		if (NOT ZeroMQ_FOUND)
			OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" ON)
			IF (AUTOBUILD_ZMQ)
				include(buildlibZMQ)
				build_libzmq()
				find_package(ZeroMQ
					HINTS 
						${ZeroMQ_INSTALL_PATH}
						${AUTOBUILD_INSTALL_PATH}
					PATH_SUFFIXES ${ZMQ_CMAKE_SUFFIXES}
				)
			ENDIF(AUTOBUILD_ZMQ)
		endif()
	endif()
endif()