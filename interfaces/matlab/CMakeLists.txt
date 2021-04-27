# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4)
project(helicsMATLAB)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
    message(STATUS "Standalone MATLAB interface build")

    if(CMAKE_VERSION VERSION_LESS 3.17)
        cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
    else()
        cmake_policy(VERSION 3.17)
    endif()

    # Setup options
    option(HELICS_ENABLE_SWIG "Use SWIG to generate the MATLAB interface files" ON)

    include(GNUInstallDirs)

    # Setup rpath
    if(NOT DEFINED CMAKE_MACOSX_RPATH)
        set(CMAKE_MACOSX_RPATH ON)
    endif()

    if(NOT DEFINED CMAKE_INSTALL_RPATH_USE_LINK_PATH)

        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    endif()

    # Add the local directory to the rpath
    if(NOT APPLE)
        list(APPEND CMAKE_INSTALL_RPATH $ORIGIN)
        list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    else()
        list(APPEND CMAKE_INSTALL_RPATH "@loader_path")
        list(APPEND CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
        list(APPEND CMAKE_INSTALL_RPATH "@executable_path")
        list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}")
    endif()
endif()

# ##################################################################################################
# Find required dependencies
# ##################################################################################################
# Find HELICS
if(NOT hasParent OR NOT TARGET HELICS::helicsSharedLib)
    find_package(HELICS 2 REQUIRED)
endif()

if(HELICS_ENABLE_SWIG OR HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)
    # Find SWIG
    if(NOT DEFINED SWIG_FOUND OR SWIG_VERSION VERSION_LESS 3)
        find_package(SWIG 3 REQUIRED)
        include(${SWIG_USE_FILE})
    endif()

    # check to make sure the swig executable supports matlab
    execute_process(COMMAND ${SWIG_EXECUTABLE} -help OUTPUT_VARIABLE SWIG_HELP_OUTPUT)
    string(FIND "${SWIG_HELP_OUTPUT}" "-matlab" MATLAB_HELP_FOUND)
    if(${MATLAB_HELP_FOUND} LESS 0)
        set(MATLAB_SWIG_NOT_AVAILABLE 1)
        message(WARNING " SWIG VERSION does not support Matlab, reverting to build only")
    endif()

    # Find helics.i swig interface file
    if(NOT HELICS_SWIG_helics.i_FILE)
        foreach(TMPDIR IN LISTS HELICS_SWIG_INCLUDE_DIRS)
            if(NOT HELICS_SWIG_helics.i_FILE)
                if(EXISTS "${TMPDIR}/helics.i")
                    # cmake-format: off
                    set(HELICS_SWIG_helics.i_FILE "${TMPDIR}/helics.i" CACHE FILEPATH "Path to the helics.i file used by swig" FORCE)
                    # cmake-format: on
                endif()
            endif()
        endforeach()
        if(NOT HELICS_SWIG_helics.i_FILE)
            message(
                FATAL_ERROR "Unable to find helics.i file, required for swig to generate interfaces"
            )
        endif()
    endif()
endif()

# CMake UseSWIG doesn't properly propagate include dirs when using target_link_libraries. This is
# also the only way that works with CMake < 3.12
include_directories(${HELICS_SWIG_INCLUDE_DIRS})

# ##################################################################################################
# UseSWIG feature support
# ##################################################################################################
# This can be removed in HELICS 3 if the minimum CMake version is >= 3.12
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
    set(USE_MODERNIZED_SWIG ON)
else()
    set(USE_MODERNIZED_SWIG OFF)
endif()

# This can be removed in HELICS 3 if the minimum CMake version is >= 3.8
if(NOT CMAKE_VERSION VERSION_LESS 3.8)
    set(USE_SWIG_ADD_LIBRARY ON)
endif()

# ##################################################################################################
# Run SWIG only (no build)
# ##################################################################################################
# Swig doesn't need MATLAB to generate the wrapper files
if(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY AND SWIG_EXECUTABLE)
    # cmake-format: off
    if(NOT hasParent)
        message(FATAL_ERROR "`HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY` can only be used when building from the root HELICS CMakeLists.txt file")
    endif()
    if(MATLAB_SWIG_NOT_AVAILABLE)
        message(FATAL_ERROR "`HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY` needs a version of swig that supports matlab")
    endif()
    # cmake-format: on
    include(${CMAKE_CURRENT_SOURCE_DIR}/matlabSwigGenerateOnly.cmake)
    return()
endif()

# ##################################################################################################
# Find MATLAB libraries
# ##################################################################################################
find_package(Matlab COMPONENTS MAIN_PROGRAM)

# ##################################################################################################
# Build the Java interface
# ##################################################################################################
message(STATUS "Building MATLAB")
get_target_property(
    TMP_HELICS_SHARED_LIB_INCLUDE_DIRS HELICS::helicsSharedLib INTERFACE_INCLUDE_DIRECTORIES
)
if(NOT HELICS_ENABLE_SWIG OR NOT SWIG_EXECUTABLE OR MATLAB_SWIG_NOT_AVAILABLE)
    message(STATUS "Building without swig")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        COMMAND "${CMAKE_COMMAND}" -E copy ${CMAKE_CURRENT_SOURCE_DIR}/interface/helicsMEX.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/interface/helicsMEX.cpp
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/interface/+helics DESTINATION matlab
            COMPONENT matlab
    )

    file(GLOB MATLAB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.m)
    file(GLOB MATLAB_GENERATED_FILES ${CMAKE_CURRENT_SOURCE_DIR}/interface/*.m)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${MATLAB_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/interface/+helics DESTINATION matlab
            COMPONENT matlab
    )
    install(FILES ${MATLAB_FILES} DESTINATION matlab COMPONENT matlab)
    install(FILES ${MATLAB_GENERATED_FILES} DESTINATION matlab COMPONENT matlab)
else()
    message(STATUS "Building with swig")
    file(GLOB SHARED_LIB_HEADERS ${TMP_HELICS_SHARED_LIB_INCLUDE_DIRS}/*.h)

    # custom command for building the wrap file
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        COMMAND "${SWIG_EXECUTABLE}" "-matlab" "-c++" -o "helicsMEX.cpp"
                "-I${TMP_HELICS_SHARED_LIB_INCLUDE_DIRS}" ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
        DEPENDS ${HELICS_SWIG_helics.i_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
                ${SHARED_LIB_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/matlab_maps.i
    )

    # add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp COMMAND ${CMAKE_COMMAND}
    # -P ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake DEPENDS
    # ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp )

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+helics DESTINATION matlab COMPONENT matlab)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m
                  ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m DESTINATION matlab COMPONENT matlab
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pisender.m ${CMAKE_CURRENT_SOURCE_DIR}/pireceiver.m
                  ${CMAKE_CURRENT_SOURCE_DIR}/GetFullPath.m DESTINATION matlab COMPONENT matlab
    )
endif()

if(hasParent)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m
        COMMAND
            ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_LINKER_FILE:HELICS::helicsSharedLib> -D
            BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp -D
            LIBRARY_INCLUDE_LOCATION=${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/ -D
            OTHER_INCLUDE_LOCATION=${CMAKE_BINARY_DIR}/src/helics/shared_api_library/ -D
            SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/generateMEXcreationScript.cmake
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
else()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m
        COMMAND
            ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_LINKER_FILE:HELICS::helicsSharedLib> -D
            BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp -D
            LIBRARY_INCLUDE_LOCATION=${TMP_HELICS_SHARED_LIB_INCLUDE_DIRS} -D
            SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
            ${CMAKE_CURRENT_SOURCE_DIR}/generateMEXcreationScript.cmake
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
endif()

# Compile the generated files using matlab
if(WIN32)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
        COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nosplash -minimize -r
                "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m HELICS::helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
elseif(APPLE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
        COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r
                "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m HELICS::helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
else()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
        COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r mkhelicsMEXFile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m HELICS::helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )
endif()

add_custom_target(
    helicsMEX ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
                          HELICS::helicsSharedLib
)
set_target_properties(helicsMEX PROPERTIES FOLDER interfaces)

file(GLOB HSHARED_LIB_SOURCES ${TMP_HELICS_SHARED_LIB_INCLUDE_DIRS}/*.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION} DESTINATION matlab
        COMPONENT matlab
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp DESTINATION matlab COMPONENT matlab)

if(WIN32)
    install(FILES $<TARGET_LINKER_FILE:HELICS::helicsSharedLib> DESTINATION matlab COMPONENT matlab)
    if(hasParent)
        install(FILES ${CMAKE_BINARY_DIR}/src/helics/shared_api_library/helics_export.h
                DESTINATION matlab/headers COMPONENT matlab
        )
    else()
        install(FILES ${TMP_HELICS_SHARED_LIB_INCLUDE_DIRS}/helics_export.h
                DESTINATION matlab/headers COMPONENT matlab
        )
    endif()
    install(FILES helicsSharedLib.h libhelicsSharedLib.h DESTINATION matlab COMPONENT matlab)
endif()

if(COMMAND install_key_files_with_comp)
    install_key_files_with_comp(matlab)
endif()
install(FILES ${HSHARED_LIB_SOURCES} DESTINATION matlab/headers COMPONENT matlab)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generatehelicsMEXFile.m DESTINATION matlab
        COMPONENT matlab
)

file(GLOB EXTRA_MATLAB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/extra/*.m)

message(STATUS "Extra ${EXTRA_MATLAB_FILES}")
install(FILES ${EXTRA_MATLAB_FILES} DESTINATION matlab/+helics COMPONENT matlab)
