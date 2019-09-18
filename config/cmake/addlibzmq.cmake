#
# Copyright (c) 2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# This file is used to add libzmq to a project
#

if(ZMQ_USE_STATIC_LIBRARY)
        set(zmq_static_build ON)
        set(zmq_shared_build OFF)
    else()
        set(zmq_static_build OFF)
        set(zmq_shared_build ON)
    endif()
	
if (MINGW)
set(HELICS_LIBZMQ_VERSION v4.3.1)
else()
set(HELICS_LIBZMQ_VERSION v4.3.2)
endif()

string(TOLOWER "libzmq" lcName)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
include(FetchContent)

FetchContent_Declare(
  libzmq
  GIT_REPOSITORY https://github.com/zeromq/libzmq.git
  GIT_TAG        ${HELICS_LIBZMQ_VERSION}
)

FetchContent_GetProperties(libzmq)

if(NOT ${lcName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(libzmq)

  # this section to be removed at the next release of ZMQ for now we need to download the file in master as the one in the release doesn't work
	file(RENAME ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old)
  file(DOWNLOAD https://raw.githubusercontent.com/zeromq/libzmq/master/builds/cmake/ZeroMQConfig.cmake.in ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in)
  
endif()
else() #cmake <3.11

# create the directory first
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/_deps)

include(GitUtils)
git_clone(
             PROJECT_NAME                    ${lcName}
             GIT_URL                         https://github.com/zeromq/libzmq.git
             GIT_TAG                         ${HELICS_LIBZMQ_VERSION}
			 DIRECTORY                       ${PROJECT_BINARY_DIR}/_deps
       )
	   
set(${lcName}_BINARY_DIR ${PROJECT_BINARY_DIR}/_deps/${lcName}-build)

if (NOT EXISTS ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old)
	file(RENAME ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in.old)
  file(DOWNLOAD https://raw.githubusercontent.com/zeromq/libzmq/master/builds/cmake/ZeroMQConfig.cmake.in ${${lcName}_SOURCE_DIR}/builds/cmake/ZeroMQConfig.cmake.in)
endif()

endif()

  # Set custom variables, policies, etc.
  # ...

  set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURVE OFF CACHE BOOL "" FORCE)
  set(ENABLE_DRAFTS OFF CACHE BOOL "" FORCE)
  set(WITH_DOCS OFF CACHE BOOL "" FORCE)
  set(ZMQ_LOCAL_BUILD ON CACHE BOOL "" FORCE)
  set(LIBZMQ_PEDANTIC OFF CACHE BOOL "" FORCE)
  set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
  set(ENABLE_CPACK OFF CACHE BOOL "" FORCE)
  set(BUILD_STATIC ${zmq_static_build} CACHE BOOL "" FORCE)
  set(BUILD_SHARED ${zmq_shared_build} CACHE BOOL "" FORCE)
  message(STATUS "zmq install lib dir ${CMAKE_INSTALL_LIBDIR}")
  
  set(ZEROMQ_CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/ZeroMQ CACHE BOOL "" FORCE)
  # Bring the populated content into the build
  add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})
  set(COMPILER_SUPPORTS_CXX11 ON)
  set(ZeroMQ_FOUND TRUE)

  set_target_properties(clang-format clang-format-check clang-format-diff PROPERTIES FOLDER "Extern/zmq_clang_format")

  if(ZMQ_USE_STATIC_LIBRARY)
  set_target_properties(libzmq-static
    PROPERTIES FOLDER "Extern")
	if ( !MSVC)
		target_compile_options(libzmq-static PRIVATE "-fPIC")
	endif()
    else()
        set_target_properties(libzmq
    PROPERTIES FOLDER "Extern")
    endif()

# hide a bunch of local variables and options
  HIDE_VARIABLE(LIBZMQ_PEDANTIC)
  HIDE_VARIABLE(LIBZMQ_WERROR)
  HIDE_VARIABLE(ZMQ_BUILD_TESTS)
  HIDE_VARIABLE(WITH_LIBSODIUM)
  HIDE_VARIABLE(WITH_MILITANT)
  HIDE_VARIABLE(WITH_OPENPGM)
  HIDE_VARIABLE(WITH_VMCI)
  HIDE_VARIABLE(WITH_PERF_TOOL)
  HIDE_VARIABLE(WITH_DOCS)
  HIDE_VARIABLE(ZEROMQ_CMAKECONFIG_INSTALL_DIR)
  HIDE_VARIABLE(POLLER)
   HIDE_VARIABLE(API_POLLER)
   HIDE_VARIABLE(ENABLE_CURVE)
 HIDE_VARIABLE(ENABLE_DRAFTS)
  HIDE_VARIABLE(ENABLE_ANALYSIS)
  HIDE_VARIABLE(ENABLE_ASAN)
  HIDE_VARIABLE(ENABLE_RADIX_TREE)
  HIDE_VARIABLE(ENABLE_EVENTFD)
  HIDE_VARIABLE(ZMQ_CV_IMPL)
  HIDE_VARIABLE(BUILD_TESTS)
  

  HIDE_VARIABLE(ZMQ_WIN32_WINNT)
  
if(ZMQ_USE_STATIC_LIBRARY)
  set(zmq_target_output "libzmq-static")
else()
  set(zmq_target_output "libzmq")
endif()


get_target_property(ZMQ_PUBLIC_HEADER_TARGETS ${zmq_target_output} PUBLIC_HEADER)

if (ZMQ_PUBLIC_HEADER_TARGETS)

set(NEW_ZMQ_PUBLIC_HEADERS)
foreach( SOURCE_FILE ${ZMQ_PUBLIC_HEADER_TARGETS} )
    list( APPEND NEW_ZMQ_PUBLIC_HEADERS ${${lcName}_SOURCE_DIR}/${SOURCE_FILE} )
  ENDFOREACH()
set_target_properties(${zmq_target_output} PROPERTIES PUBLIC_HEADER "${NEW_ZMQ_PUBLIC_HEADERS}")

if (NOT HELICS_BINARY_ONLY_INSTALL)
install(FILES ${NEW_ZMQ_PUBLIC_HEADERS} 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
	)
endif()
endif()

if(NOT CMAKE_VERSION VERSION_LESS 3.13)
message(STATUS "installing ${zmq_target_output} as HELICS EXPORT")
install(TARGETS ${zmq_target_output}
    ${HELICS_EXPORT_COMMAND}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT libs)
	
endif()

install(
    FILES $<TARGET_FILE:${zmq_target_output}>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT libs
  )
  
  install(
    FILES $<TARGET_LINKER_FILE:${zmq_target_output}>
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libs
  )
  
if(MSVC AND NOT EMBEDDED_DEBUG_INFO AND NOT HELICS_BINARY_ONLY_INSTALL)
  install(
    FILES $<TARGET_PDB_FILE:${zmq_target_output}>
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    OPTIONAL COMPONENT libs
  )
endif()
