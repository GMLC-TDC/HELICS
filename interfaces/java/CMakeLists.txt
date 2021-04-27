# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4)
project(helicsJAVA)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
    message(STATUS "Standalone Java interface build")

    if(CMAKE_VERSION VERSION_LESS 3.17)
        cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
    else()
        cmake_policy(VERSION 3.17)
    endif()

    # Setup options
    option(HELICS_ENABLE_SWIG "Use SWIG to generate the Java interface files" ON)

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
# Swig doesn't need Java to generate the wrapper files
if(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY AND SWIG_EXECUTABLE)
    if(NOT hasParent)
        message(
            FATAL_ERROR
                "`HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY`  can only be used when building from the root HELICS CMakeLists.txt file"
        )
    endif()
    include(${CMAKE_CURRENT_SOURCE_DIR}/javaSwigGenerateOnly.cmake)
    return()
endif()

# ##################################################################################################
# Find Java libraries
# ##################################################################################################
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

if(NOT USE_MODERNIZED_SWIG)
    include_directories(${JAVA_INCLUDE_PATH})
    include_directories(${JAVA_INCLUDE_PATH2})
    include_directories(${JNI_INCLUDE_DIRS})
    # "Using old SWIG include_directories")
endif()

# ##################################################################################################
# Build the Java interface
# ##################################################################################################
if(HELICS_ENABLE_SWIG AND SWIG_EXECUTABLE)
    set_property(SOURCE helicsJava.i PROPERTY SWIG_MODULE_NAME helics)
    set(CMAKE_SWIG_FLAGS "-package;com.java.helics")

    if(SWIG_VERSION VERSION_GREATER 4.0.0)
        set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} "-doxygen")
    endif()

    if(USE_SWIG_ADD_LIBRARY)
        swig_add_library(
            helicsJava
            TYPE MODULE
            LANGUAGE java
            SOURCES helicsJava.i
        )
    else()
        swig_add_module(helicsJava java helicsJava.i)
    endif()

    swig_link_libraries(helicsJava HELICS::helicsSharedLib)
    swig_link_libraries(helicsJava ${JAVA_LIBRARIES})

    if(USE_MODERNIZED_SWIG)
        target_include_directories(${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE ${JAVA_INCLUDE_PATH})
        target_include_directories(
            ${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE ${JAVA_INCLUDE_PATH2}
        )
        target_include_directories(${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE ${JNI_INCLUDE_DIRS})
        message(STATUS "Using new swig target_include_directories")
    endif()

    if(MSVC)
        target_compile_options(${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE "/wd4100")
    else()
        target_compile_options(
            ${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE -Wno-deprecated-declarations
        )
        if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            target_compile_options(
                ${SWIG_MODULE_helicsJava_REAL_NAME} PRIVATE -Wno-long-long -Wno-strict-aliasing
            )
        endif()
    endif()

    set_target_properties(
        ${SWIG_MODULE_helicsJava_REAL_NAME} PROPERTIES FOLDER interfaces DEBUG_POSTFIX ""
    )
    add_library(helics::java ALIAS ${SWIG_MODULE_helicsJava_REAL_NAME})
    message(STATUS "java target is ${SWIG_MODULE_helicsJava_REAL_NAME}")
    set(HELICS_JAVA_TARGET ${SWIG_MODULE_helicsJava_REAL_NAME})
else()
    message(STATUS "Building without swig")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    add_library(helicsJava SHARED interface/helicsJava.c)
    target_link_libraries(helicsJava HELICS::helicsSharedLib ${JAVA_LIBRARIES})
    target_include_directories(helicsJava PRIVATE ${JAVA_INCLUDE_PATH})
    target_include_directories(helicsJava PRIVATE ${JAVA_INCLUDE_PATH2})
    target_include_directories(helicsJava PRIVATE ${JNI_INCLUDE_DIRS})

    if(MSVC)
        target_compile_options(helicsJava PRIVATE "/wd4100")
    else()
        target_compile_options(helicsJava PRIVATE -Wno-deprecated-declarations)
        if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            target_compile_options(helicsJava PRIVATE -Wno-long-long -Wno-strict-aliasing)
        endif()
    endif()

    set_target_properties(helicsJava PROPERTIES FOLDER interfaces DEBUG_POSTFIX "")

    add_library(helics::java ALIAS helicsJava)
    set(HELICS_JAVA_TARGET helicsJava)
endif()

# ##################################################################################################
# Make JAR file
# ##################################################################################################
configure_file(MakeJarCMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/jarproject/CMakeLists.txt @ONLY)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar/)

# -D ADDITIONAL_JAR_FILES=$<TARGET_FILE:helicsSharedLib>;$<TARGET_FILE:helicsJava>
add_custom_command(
    TARGET ${HELICS_JAVA_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_FILE:${HELICS_JAVA_TARGET}> -P
            ${CMAKE_CURRENT_SOURCE_DIR}/addLoadLibraryCommand.cmake
)

if(CMAKE_GENERATOR_PLATFORM)
    add_custom_command(
        TARGET ${HELICS_JAVA_TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ..
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar"
        VERBATIM
    )
else()
    add_custom_command(
        TARGET ${HELICS_JAVA_TARGET}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -G ${CMAKE_GENERATOR} ..
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar"
        VERBATIM
    )
endif()

add_custom_command(
    TARGET ${HELICS_JAVA_TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} ARGS --build ./ --target helics
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar"
    COMMENT "Building jar file"
    VERBATIM
)

# ##################################################################################################
# Install steps
# ##################################################################################################
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar/helics-${HELICS_VERSION}.jar
        DESTINATION java COMPONENT java
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/jarproject/buildjar/helics.jar
    DESTINATION java
    COMPONENT java
    OPTIONAL
)
install(TARGETS ${HELICS_JAVA_TARGET} DESTINATION java COMPONENT java)

# Copy/install commands for shared libraries are only needed on Windows, which doesn't support
# rpaths
if(WIN32)
    if(COMMAND copy_key_files_to_target_location)
        copy_key_files_to_target_location(${HELICS_JAVA_TARGET})
    endif()

    if(COMMAND copy_shared_target)
        copy_shared_target(${HELICS_JAVA_TARGET} helicsSharedLib)
    endif()

    if(COMMAND install_key_files_with_comp)
        install_key_files_with_comp(java)
    else()
        install(FILES "$<TARGET_FILE:HELICS::helicsSharedLib>" DESTINATION java COMPONENT java)
    endif()
endif()
