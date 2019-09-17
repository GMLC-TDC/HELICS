#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#File used to generate the matlab interface and overwrite the existing files if requested

file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsJava.c
    COMMAND
      "${SWIG_EXECUTABLE}" "-java" "-package" "com.java.helics" -o "helicsJava.c"
      "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsJava.i
    DEPENDS
      ../helics.i
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsJava.i
      ${SHARED_LIB_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/java_maps.i
  )

  if(HELICS_OVERWRITE_INTERFACE_FILES)
    add_custom_target(
      javafile_overwrite ALL
      COMMAND
        ${CMAKE_COMMAND} -D TARGET_DIR=${CMAKE_CURRENT_SOURCE_DIR}/interface -P
        ${CMAKE_CURRENT_SOURCE_DIR}/overwriteJavaFiles.cmake
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsJava.c
    )
  else(HELICS_OVERWRITE_INTERFACE_FILES)
  #extra target for generation only and no overwrite so the dependency actually gets evaluated by cmake
   add_custom_target(
      java_create ALL
      COMMAND
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsJava.c
    )
endif(HELICS_OVERWRITE_INTERFACE_FILES)
