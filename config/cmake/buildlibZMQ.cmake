##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
# This function is used to force a build on a dependant project at cmake configuration phase.
# 
function (build_libzmq)

set(trigger_build_dir ${CMAKE_BINARY_DIR}/autobuild/force_libzmq)



	include(escape_string)
	
	escape_string(cxx_compiler_string ${CMAKE_CXX_COMPILER})
	escape_string(c_compiler_string ${CMAKE_C_COMPILER})
	escape_string(linker_string ${CMAKE_LINKER})
	message(STATUS "cxx compiler ${cxx_compiler_string}")
	message(STATUS "linker ${linker_string}")
	
	set(extra_cxx_flags ${CMAKE_CXX_FLAGS})
	if (UNIX)
	if (USE_LIBCXX)
		set(extra_cxx_flags "${extra_cxx_flags} -stdlib=libc++")
	endif(USE_LIBCXX)
	else(UNIX)
	endif()
	
	if (ZMQ_USE_STATIC_LIBRARY)
	   set(zmq_static_build ON)
	   set(zmq_shared_build OFF)
	   set(extra_cxx_flags "${extra_cxx_flags} -fPIC")
	else()
        set(zmq_static_build OFF)
	   set(zmq_shared_build ON)
	endif()
	
	if (VERSION_OPTION)
		set(extra_cxx_flags "${extra_cxx_flags} ${VERSION_OPTION}")
	endif()
	
	message(STATUS "flags ${extra_cxx_flags}")
	
    #mktemp dir in build tree
    file(MAKE_DIRECTORY ${trigger_build_dir} ${trigger_build_dir}/build)

    #generate false dependency project
    set(CMAKE_LIST_CONTENT "
    cmake_minimum_required(VERSION 3.4)
    include(ExternalProject)
ExternalProject_Add(libzmq
	SOURCE_DIR ${PROJECT_BINARY_DIR}/Download/libzmq
    GIT_REPOSITORY  https://github.com/zeromq/libzmq.git
	GIT_TAG v4.2.3
    DOWNLOAD_COMMAND " " 
    UPDATE_COMMAND " " 
	BINARY_DIR ${PROJECT_BINARY_DIR}/ThirdParty/libzmq
	 
    CMAKE_ARGS 
        -DCMAKE_CXX_COMPILER=${cxx_compiler_string}
        -DCMAKE_C_COMPILER=${c_compiler_string}
        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/libs
        -DCMAKE_BUILD_TYPE=\$\{CMAKE_BUILD_TYPE\}
		-DZMQ_BUILD_TESTS=OFF
		-DBUILD_STATIC=${zmq_static_build}
		-DBUILD_SHARED=${zmq_shared_build}
		\"-DCMAKE_CXX_FLAGS=${extra_cxx_flags}\"
		\"-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}\"
		-DENABLE_CPACK=OFF
		-DLIBZMQ_PEDANTIC=OFF
		-DZEROMQ_CMAKECONFIG_INSTALL_DIR=${PROJECT_BINARY_DIR}/libs/cmake/ZeroMQ
		
	INSTALL_DIR ${PROJECT_BINARY_DIR}/libs
	)")


    file(WRITE ${trigger_build_dir}/CMakeLists.txt "${CMAKE_LIST_CONTENT}")

	message(STATUS "Configuring ZeroMQ Autobuild for release logging to ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_config_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND}  -D CMAKE_CXX_COMPILER=${cxx_compiler_string} -D CMAKE_C_COMPILER=${c_compiler_string}
	    -D CMAKE_LINKER=${linker_string} -D CMAKE_BUILD_TYPE=Release -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_config_release.log
        )
		
	message(STATUS "Building ZeroMQ release build logging to ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_build_release.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Release
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_build_release.log
        )
		
if (NOT BUILD_RELEASE_ONLY)
	message(STATUS "Configuring ZeroMQ Autobuild for debug logging to ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_config_debug.log")
	execute_process(COMMAND ${CMAKE_COMMAND}  -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER}
	    -D CMAKE_BUILD_TYPE=Debug -G ${CMAKE_GENERATOR} .. 
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_config_debug.log
        )
		
	message(STATUS "Building ZeroMQ debug build logging to ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_build_debug.log")
    execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug
        WORKING_DIRECTORY ${trigger_build_dir}/build
		OUTPUT_FILE ${PROJECT_BINARY_DIR}/logs/zmq_autobuild_build_debug.log
        )
endif()	




endfunction()
