# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4)
project(helicsPYTHON)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
    message(STATUS "Standalone Python interface build")

    if(CMAKE_VERSION VERSION_LESS 3.17)
        cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
    else()
        cmake_policy(VERSION 3.17)
    endif()

    # Setup options
    option(HELICS_ENABLE_SWIG "Use SWIG to generate the Python interface files" ON)
    # option(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY "Only generate the SWIG interface files" OFF)
    if(NOT ${CMAKE_VERSION} VERSION_LESS 3.12)
        option(HELICS_USE_NEW_PYTHON_FIND "Use the FindPython module added in CMake 3.12" OFF)
    endif()

    option(BUILD_PYTHON2_INTERFACE "Build the interface for Python2; requires SWIG" OFF)

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

# Give the Python3 interface preferential treatment
set(_py_ver 3)
if(BUILD_PYTHON_INTERFACE AND BUILD_PYTHON2_INTERFACE)
    message(WARNING "Python 2 interface will not be built since Python 3 is active")
elseif(BUILD_PYTHON2_INTERFACE)
    set(_py_ver 2)
    # Require swig and don't allow only generating interface files for Python 2
    if(NOT HELICS_ENABLE_SWIG)
        message(SEND_ERROR "Python 2 build requires swig")
        return()
    elseif(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)
        message(SEND_ERROR "Python 2 interface does not support only generating interface files")
        return()
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
else()
    set(USE_SWIG_ADD_LIBRARY OFF)
endif()

# ##################################################################################################
# Run SWIG only (no build)
# ##################################################################################################
# Swig doesn't need Python to generate the wrapper files
if(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY AND SWIG_EXECUTABLE)
    if(NOT hasParent)
        message(
            FATAL_ERROR
                "`HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY`  can only be used when building from the root HELICS CMakeLists.txt file"
        )
    endif()
    include(${CMAKE_CURRENT_SOURCE_DIR}/pythonSwigGenerateOnly.cmake)
    return()
endif()

# ##################################################################################################
# Find Python libraries
# ##################################################################################################
if(HELICS_USE_NEW_PYTHON_FIND)
    find_package(Python${_py_ver} COMPONENTS Interpreter Development)
    set(HELICS_Python_EXECUTABLE "${Python${_py_ver}_EXECUTABLE}")
    set(HELICS_Python_INCLUDE_DIRS "${Python${_py_ver}_INCLUDE_DIRS}")
    set(HELICS_Python_LIBRARIES "${Python${_py_ver}_LIBRARIES}")
else()
    if(_py_ver EQUAL 3)
        set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
    elseif(_py_ver EQUAL 2)
        set(Python_ADDITIONAL_VERSIONS 2.7)
    endif()
    if(NOT PYTHON_EXECUTABLE)
        find_package(PythonInterp ${_py_ver} REQUIRED)
    endif()
    if(PYTHONINTERP_FOUND)
        set(Python_ADDITIONAL_VERSIONS "${PYTHON_VERSION_STRING}")
    endif()
    find_package(PythonLibs ${_py_ver} REQUIRED)
    set(HELICS_Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
    set(HELICS_Python_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}")
    set(HELICS_Python_LIBRARIES "${PYTHON_LIBRARIES}")
endif()

# ##################################################################################################
# Build the Python interface
# ##################################################################################################
if(HELICS_ENABLE_SWIG AND SWIG_EXECUTABLE)
    # https://stackoverflow.com/a/3818084/5451769
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # Update if necessary
        set(CMAKE_C_FLAGS
            "${CMAKE_C_FLAGS} -Wno-overlength-strings -Wno-ignored-attributes -Wno-deprecated-declarations"
        )
    endif()

    if(NOT USE_MODERNIZED_SWIG)
        include_directories(${HELICS_Python_INCLUDE_DIRS})
        message(STATUS "Using old SWIG include_directories")
    endif()

    if(_py_ver EQUAL 3)
        set(CMAKE_SWIG_FLAGS "-py3")
    endif()

    if(SWIG_VERSION VERSION_GREATER "4.0.0")
        set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} "-doxygen")
    endif()

    if(USE_SWIG_ADD_LIBRARY)
        swig_add_library(
            helics
            TYPE MODULE
            LANGUAGE python
            SOURCES helicsPython${_py_ver}.i
        )
    else()
        swig_add_module(helics python helicsPython${_py_ver}.i)
        message(STATUS "Using old swig_add_module")
    endif()

    swig_link_libraries(helics HELICS::helicsSharedLib)

    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Ie%20m6Ot%20jBCQAJ
        # https://github.com/GMLC-TDC/HELICS/commit/633b218f9351aa452d4#commitcomment-27055932
        set_target_properties(
            ${SWIG_MODULE_helics_REAL_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
        )
    else()
        swig_link_libraries(helics ${HELICS_Python_LIBRARIES})
    endif()

    if(USE_MODERNIZED_SWIG)
        target_include_directories(
            ${SWIG_MODULE_helics_REAL_NAME} PRIVATE ${HELICS_Python_INCLUDE_DIRS}
        )
        message(STATUS "Using new swig target_include_directories")
    endif()

    if(MSVC)
        target_compile_options(${SWIG_MODULE_helics_REAL_NAME} PRIVATE "/wd4100")
    else(MSVC)
        target_compile_options(
            ${SWIG_MODULE_helics_REAL_NAME} PRIVATE -Wno-long-long -Wno-deprecated-declarations
        )
    endif(MSVC)

    message(STATUS "swig target is ${SWIG_MODULE_helics_REAL_NAME}")

    set_target_properties(
        ${SWIG_MODULE_helics_REAL_NAME} PROPERTIES FOLDER interfaces DEBUG_POSTFIX ""
    )

    if(CMAKE_PROJECT_NAME STREQUAL "HELICS")
        set_target_properties(
            ${SWIG_MODULE_helics_REAL_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                       ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_target_properties(
            ${SWIG_MODULE_helics_REAL_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                                       ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
    install(TARGETS ${SWIG_MODULE_helics_REAL_NAME} DESTINATION python COMPONENT python)

    set(HELICS_PYTHON_TARGET_NAME ${SWIG_MODULE_helics_REAL_NAME})
else()
    message(STATUS "Building without swig")

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    if(HELICS_USE_NEW_PYTHON_FIND)
        python3_add_library(helicsPYTHON MODULE interface/helicsPython.c)
    else()
        # Remove in HELICS 3
        include(${CMAKE_CURRENT_SOURCE_DIR}/addHelicsPy3Lib.cmake)
        helics_python3_add_library(helicsPYTHON interface/helicsPython.c)
    endif()

    # Set the output library name to _helics
    set_target_properties(helicsPYTHON PROPERTIES PREFIX "_" OUTPUT_NAME "helics" DEBUG_POSTFIX "")

    target_link_libraries(helicsPYTHON PUBLIC HELICS::helicsSharedLib)
    set_target_properties(helicsPYTHON PROPERTIES FOLDER interfaces)
    if(CMAKE_PROJECT_NAME STREQUAL "HELICS")
        set_target_properties(
            helicsPYTHON PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_target_properties(
            helicsPYTHON PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()
    if(NOT MSVC)
        target_compile_options(
            helicsPYTHON PRIVATE -Wno-long-long -Wno-strict-aliasing -Wno-deprecated-declarations
        )
    endif()
    install(TARGETS helicsPYTHON DESTINATION python COMPONENT python)

    set(HELICS_PYTHON_TARGET_NAME helicsPYTHON)

    # TODO: check for when setup.py is used
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.py DESTINATION python COMPONENT python)

# ##################################################################################################
# Install steps
# ##################################################################################################
# Copy/install commands for shared libraries are only needed on Windows, which doesn't support
# rpaths. Standalone interface builds for wheels will need the files to be copied to a different
# location anyway, so not copying the files for now isn't a big loss
if(WIN32)
    # Realistically for bundling into a Python wheel, the dll should be given a unique name to avoid
    # conflicts with other packages using a different version of the dll with the same name
    add_custom_command(
        TARGET ${HELICS_PYTHON_TARGET_NAME}
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
            "$<TARGET_FILE:HELICS::helicsSharedLib>" # <--this is in- file
            "$<TARGET_FILE_DIR:${HELICS_PYTHON_TARGET_NAME}>/" # <--this is out- file path
    )
    if(COMMAND copy_key_files_to_target_location)
        copy_key_files_to_target_location(${HELICS_PYTHON_TARGET_NAME})
    endif()
    if(COMMAND install_key_files_with_comp)
        install_key_files_with_comp(python)
    else()
        install(FILES "$<TARGET_FILE:HELICS::helicsSharedLib>" DESTINATION python COMPONENT python)
    endif()
endif()
