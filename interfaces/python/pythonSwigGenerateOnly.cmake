# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#File used to generate the python interface and overwrite the existing files if requested

file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

if(SWIG_VERSION VERSION_GREATER "4.0.0")
  set(SWIG_DOXYGEN_FLAG "-doxygen")
endif()

  get_filename_component(helics.i_INCLUDE_DIR "${HELICS_SWIG_helics.i_FILE}" DIRECTORY)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    COMMAND
      "${SWIG_EXECUTABLE}" "-python" "-py3" -o "helicsPython.c" "${SWIG_DOXYGEN_FLAG}"
      "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
      "-I${helics.i_INCLUDE_DIR}"
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsPython3.i
    DEPENDS
      ${HELICS_SWIG_helics.i_FILE}
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsPython3.i
      ${SHARED_LIB_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/python_maps3.i
  )

  if(HELICS_OVERWRITE_INTERFACE_FILES)
    add_custom_target(
      pyfile_overwrite ALL
      COMMAND
        ${CMAKE_COMMAND} -D TARGET_DIR=${CMAKE_CURRENT_SOURCE_DIR}/interface -P
        ${CMAKE_CURRENT_SOURCE_DIR}/overwritePythonFiles.cmake
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    )
    set_target_properties(pyfile_overwrite PROPERTIES FOLDER interfaces)
  else(HELICS_OVERWRITE_INTERFACE_FILES)
  #extra target for generation only and no overwrite
   add_custom_target(
      python_create ALL
      COMMAND
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsPython.c
    )
    set_target_properties(python_create PROPERTIES FOLDER interfaces)
endif(HELICS_OVERWRITE_INTERFACE_FILES)
