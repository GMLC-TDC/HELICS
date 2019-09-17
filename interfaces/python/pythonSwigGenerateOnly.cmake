#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#File used to generate the python interface and overwrite the existing files if requested

file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    COMMAND
      "${SWIG_EXECUTABLE}" "-python" "-py3" -o "helicsPython.c"
      "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsPython.i
    DEPENDS
      ../helics.i
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsPython.i
      ${SHARED_LIB_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/python_maps.i
  )

  if(HELICS_OVERWRITE_INTERFACE_FILES)
    add_custom_target(
      pyfile_overwrite ALL
      COMMAND
        ${CMAKE_COMMAND} -D TARGET_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/overwritePythonFiles.cmake
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    )
  else(HELICS_OVERWRITE_INTERFACE_FILES)
  #extra target for generation only and no overwrite
   add_custom_target(
      python_create ALL
      COMMAND
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    )
endif(HELICS_OVERWRITE_INTERFACE_FILES)
