#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

set(
  common_headers
  JsonProcessingFunctions.hpp
  JsonBuilder.hpp
  TomlProcessingFunctions.hpp
  AsioContextManager.h
  logger.h
  loggerCore.hpp
  GuardedTypes.hpp
  fmt_format.h
  fmt_ostream.h
  addTargets.hpp
)

set(
  common_sources
  AsioContextManager.cpp
  JsonProcessingFunctions.cpp
  JsonBuilder.cpp
  TomlProcessingFunctions.cpp
  logger.cpp
  loggerCore.cpp
)

set(
  zmq_headers
  zmqContextManager.h
  zmqHelper.h
  ${HELICS_SOURCE_DIR}/ThirdParty/cppzmq/zmq.hpp
  ${HELICS_SOURCE_DIR}/ThirdParty/cppzmq/zmq_addon.hpp
)

set(
  zmq_sources
  # zmqReactor.cpp
  zmqContextManager.cpp
  # zmqSocketDescriptor.cpp
  zmqHelper.cpp
  # zmqProxyHub.cpp
)

# headers that are part of the public interface
set(
  helics_public_common
  JsonProcessingFunctions.hpp
  AsioContextManager.h
  logger.h
  GuardedTypes.hpp
)

if(ENABLE_ZMQ_CORE)
  list(APPEND common_headers ${zmq_headers})
  list(APPEND common_sources ${zmq_sources})
endif(ENABLE_ZMQ_CORE)

add_library(helics_common OBJECT ${common_sources} ${common_headers})
set_target_properties(helics_common PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(
  helics_common
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_INCLUDE_DIRECTORIES>
)
target_include_directories(
  helics_common SYSTEM
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
)
target_compile_definitions(
  helics_common PRIVATE
  $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_DEFINITIONS>
)

if (TARGET Boost::boost)
target_include_directories(
  helics_common SYSTEM
  PRIVATE $<TARGET_PROPERTY:Boost::boost,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
)
endif()

target_compile_options(
  helics_common
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_OPTIONS>
)

target_compile_options(
  helics_common
  PRIVATE $<TARGET_PROPERTY:compile_flags_target,INTERFACE_COMPILE_OPTIONS>
)


if(ENABLE_ZMQ_CORE)
  target_include_directories(
    helics_common SYSTEM
    PRIVATE $<TARGET_PROPERTY:${ZeroMQ_DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>
  )
endif(ENABLE_ZMQ_CORE)

if(APPLE)
  target_compile_definitions(helics_common PUBLIC "-DSTX_NO_STD_ANY=1")
endif(APPLE)

install(
  FILES ${helics_public_common}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/common
  COMPONENT headers
)
