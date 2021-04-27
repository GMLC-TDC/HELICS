# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Generate SWIG wrapper (for Octave)
message(STATUS "Building Octave interface")

find_package(Octave REQUIRED)
if(NOT MKOCTFILE_EXECUTABLE)
    show_variable(
        OCTAVE_INSTALL_LOCATION PATH "installation location for Octave"
        "${OCTAVE_INSTALL_LOCATION}"
    )
endif()

# Compile OCT file add_custom_target("helicsOCT" ALL)
file(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

# custom command for building the wrap file
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
    COMMAND
        "${SWIG_EXECUTABLE}" "-octave" "-c++" -o "helicsOCTAVE_wrap.cxx"
        "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
        ${CMAKE_CURRENT_SOURCE_DIR}/helicsOctave.i
    DEPENDS ${CMAKE_SOURCE_DIR}/interfaces/helics.i ${CMAKE_CURRENT_SOURCE_DIR}/helicsOctave.i
            ${CMAKE_CURRENT_SOURCE_DIR}/octave_maps.i ${SHARED_LIB_HEADERS}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
    COMMAND ${CMAKE_COMMAND} -D VOID_SIZE=${CMAKE_SIZEOF_VOID_P} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m
    COMMAND
        ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_LINKER_FILE:helicsSharedLib> -D
        BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp -D
        LIBRARY_INCLUDE_LOCATION=${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/ -D
        OTHER_INCLUDE_LOCATION=${PROJECT_BINARY_DIR}/src/helics/shared_api_library/ -D
        SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/generateOCTcreationScript.cmake
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helics.oct
    COMMAND ${OCTAVE_EXECUTABLE} "-q" "\"${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m\""
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m helicsSharedLib
            ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
)

add_custom_target(helicsOCT ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helics.oct)

set_target_properties(helicsOCT PROPERTIES FOLDER interfaces)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.oct DESTINATION octave COMPONENT octave)
install_key_files_with_comp(octave)
