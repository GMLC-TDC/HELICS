# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#File used to generate the Java interface and overwrite the existing files if requested

file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

if(SWIG_VERSION VERSION_GREATER "4.0.0")
  set(SWIG_DOXYGEN_FLAG "-doxygen")
endif()

  get_filename_component(helics.i_INCLUDE_DIR "${HELICS_SWIG_helics.i_FILE}" DIRECTORY)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsJava.c
    COMMAND
      "${SWIG_EXECUTABLE}" "-java" "-package" "com.java.helics" -o "helicsJava.c" "${SWIG_DOXYGEN_FLAG}"
      "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
      "-I${helics.i_INCLUDE_DIR}"
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsJava.i
    DEPENDS
      ${HELICS_SWIG_helics.i_FILE}
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
    set_target_properties(javafile_overwrite PROPERTIES FOLDER interfaces)
  else(HELICS_OVERWRITE_INTERFACE_FILES)
  #extra target for generation only and no overwrite so the dependency actually gets evaluated by cmake
   add_custom_target(
      java_create ALL
      COMMAND
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsJava.c
    )
    set_target_properties(java_create PROPERTIES FOLDER interfaces)
endif(HELICS_OVERWRITE_INTERFACE_FILES)
