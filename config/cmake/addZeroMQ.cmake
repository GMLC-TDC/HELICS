#file to include ZMQ
OPTION(USE_SYSTEM_ZEROMQ_ONLY "only search for system zeromq libraries, bypass autobuild option" OFF)
if (USE_SYSTEM_ZEROMQ_ONLY)
	find_package(ZeroMQ)
else()
OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZeroMQ_INSTALL_PATH PATH
  "path to the zmq libraries" "${AUTOBUILD_INSTALL_PATH}")

set(ZMQ_CMAKE_SUFFIXES 
	cmake/ZeroMQ 
	cmake
	CMake/ZeroMQ
	lib/cmake)

if (WIN32 AND NOT MSYS)
find_package(ZeroMQ QUIET
	HINTS 
		${ZeroMQ_INSTALL_PATH}
		${AUTOBUILD_INSTALL_PATH}
	PATH_SUFFIXES ${ZMQ_CMAKE_SUFFIXES}
	)
else()
find_package(ZeroMQ QUIET
	HINTS 
		${ZeroMQ_INSTALL_PATH}
		${AUTOBUILD_INSTALL_PATH}
	PATH_SUFFIXES ${ZMQ_CMAKE_SUFFIXES}
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PACKAGE_REGISTRY
	NO_CMAKE_SYSTEM_PATH
	NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
	)
endif()

if (NOT ZeroMQ_FOUND)
	#message(STATUS "initialZMQ not found")
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
			if (WIN32 AND NOT MSYS)
			OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" ON)
			else()
				OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" OFF)
			endif()
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

endif() # USE_SYSTEM_ZEROMQ_ONLY