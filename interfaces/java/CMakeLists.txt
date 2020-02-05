# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)
    include(${CMAKE_CURRENT_SOURCE_DIR}/javaSwigGenerateOnly.cmake)
else()
    find_package(Java REQUIRED)
    find_package(JNI REQUIRED)
    include(UseJava)

    include_directories(${JAVA_INCLUDE_PATH})
    include_directories(${JAVA_INCLUDE_PATH2})
    include_directories(${JNI_INCLUDE_DIRS})

    if(HELICS_ENABLE_SWIG AND SWIG_EXECUTABLE)
        # Enable generation using swig

        set_property(SOURCE helicsJava.i PROPERTY SWIG_MODULE_NAME JNIhelics)

        set(CMAKE_SWIG_FLAGS "-package;com.java.helics")

        if(SWIG_VERSION VERSION_GREATER "4.0.0")
            set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} "-doxygen")
        endif()

        if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
            swig_add_library(
                JNIhelics
                TYPE
                MODULE
                LANGUAGE
                java
                SOURCES
                helicsJava.i
            )
        else()
            swig_add_module(JNIhelics java helicsJava.i)
        endif()

        swig_link_libraries(JNIhelics helicsSharedLib)
        swig_link_libraries(JNIhelics ${JAVA_LIBRARIES})

        set_target_properties(JNIhelics PROPERTIES FOLDER interfaces)
        # get rid of some annoying warnings coming from swig generated code
        if(NOT MSVC)
            target_compile_options(JNIhelics PRIVATE "-Wno-strict-aliasing")
        endif()

    else()

        file(
            COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
        if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            # Update if necessary
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long")
        endif()
        add_library(JNIhelics SHARED interface/helicsJava.c)
        target_link_libraries(JNIhelics helicsSharedLib ${JAVA_LIBRARIES})
        set_target_properties(JNIhelics PROPERTIES FOLDER interfaces)
    endif() # HELICS_ENABLE_SWIG

    configure_file(MakeJarCMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
                   @ONLY)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/buildjar/)

    # -D ADDITIONAL_JAR_FILES=$<TARGET_FILE:helicsSharedLib>;$<TARGET_FILE:JNIhelics>
    add_custom_command(
        TARGET "JNIhelics" POST_BUILD
        COMMAND
            ${CMAKE_COMMAND}
            -D
            LIBRARY_FILE=$<TARGET_FILE:JNIhelics>
            -P
            ${CMAKE_CURRENT_SOURCE_DIR}/addLoadLibraryCommand.cmake
    )

if (CMAKE_GENERATOR_PLATFORM)
    add_custom_command(
        TARGET "JNIhelics" POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -G ${CMAKE_GENERATOR} -A ${CMAKE_GENERATOR_PLATFORM} ..
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/buildjar"
        VERBATIM
    )
else()
    add_custom_command(
        TARGET "JNIhelics" POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -G ${CMAKE_GENERATOR} ..
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/buildjar"
        VERBATIM
    )
endif()
    

    add_custom_command(
        TARGET "JNIhelics" POST_BUILD
        COMMAND
            ${CMAKE_COMMAND}
            ARGS
            --build
            .
            --target
            helics
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/buildjar/"
        COMMENT "Building jar file"
        VERBATIM
    )

    copy_key_files_to_target_location(JNIhelics)
    copy_shared_target(JNIhelics helicsSharedLib)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/buildjar/helics-${HELICS_VERSION}.jar
        DESTINATION java
        COMPONENT java
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/buildjar/helics.jar
        DESTINATION java
        COMPONENT java
        OPTIONAL
    )

    install_key_files_with_comp(java)
    install(
        TARGETS
            JNIhelics
            DESTINATION
            java
            COMPONENT
            java
    )

endif()
