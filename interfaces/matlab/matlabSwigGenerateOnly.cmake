# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#File used to generate the matlab interface and overwrite the existing files if requested

file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

  get_filename_component(helics.i_INCLUDE_DIR "${HELICS_SWIG_helics.i_FILE}" DIRECTORY)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    COMMAND
      "${SWIG_EXECUTABLE}" "-matlab" "-c++" -o "helicsMEX.cpp"
      "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
      "-I${helics.i_INCLUDE_DIR}"
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
    DEPENDS
      ../helics.i
      ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
      ${SHARED_LIB_HEADERS}
      ${CMAKE_CURRENT_SOURCE_DIR}/matlab_maps.i
  )

  if(HELICS_OVERWRITE_INTERFACE_FILES)
    add_custom_target(
      mfile_overwrite ALL
      COMMAND
        ${CMAKE_COMMAND} -D TARGET_DIR=${CMAKE_CURRENT_SOURCE_DIR}/interface -P
        ${CMAKE_CURRENT_SOURCE_DIR}/overwriteMatlabFiles.cmake
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
    set_target_properties( mfile_overwrite PROPERTIES FOLDER interfaces)
  else(HELICS_OVERWRITE_INTERFACE_FILES)
  #extra target for generation only and no overwrite
   add_custom_target(
      matlab_create ALL
      COMMAND
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
    set_target_properties(matlab_create PROPERTIES FOLDER interfaces)
endif(HELICS_OVERWRITE_INTERFACE_FILES)
