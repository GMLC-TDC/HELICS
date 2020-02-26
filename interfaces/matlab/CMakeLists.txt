# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Generate SWIG wrapper for MATLAB
if(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)
    include(${CMAKE_CURRENT_SOURCE_DIR}/matlabSwigGenerateOnly.cmake)
else()
    find_package(Matlab COMPONENTS MAIN_PROGRAM)

    if(HELICS_ENABLE_SWIG AND SWIG_EXECUTABLE)
        # check to make sure the swig executable supports matlab
        execute_process(
            COMMAND ${SWIG_EXECUTABLE} -help
            OUTPUT_VARIABLE SWIG_HELP_OUTPUT
        )
        string(FIND "${SWIG_HELP_OUTPUT}" "-matlab" MATLAB_HELP_FOUND)
        if(${MATLAB_HELP_FOUND} LESS 0)
            set(MATLAB_SWIG_NOT_AVAILABLE 1)
            message(
                WARNING
                    " SWIG VERSION does not support Matlab, reverting to build only"
            )
        endif()
    endif()

    message(STATUS "Building MATLAB")
    if(NOT SWIG_EXECUTABLE OR MATLAB_SWIG_NOT_AVAILABLE)

        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
            COMMAND
                "${CMAKE_COMMAND}"
                -E
                copy
                ${CMAKE_CURRENT_SOURCE_DIR}/interface/helicsMEX.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/interface/helicsMEX.cpp
        )
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/interface/+helics
            DESTINATION matlab
            COMPONENT matlab
        )

        file(GLOB MATLAB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.m)
        file(GLOB MATLAB_GENERATED_FILES ${CMAKE_CURRENT_SOURCE_DIR}/interface/*.m)
        file(
            COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
        file(
            COPY ${MATLAB_FILES}
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
        )
        install(
            DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/interface/+helics
            DESTINATION matlab
            COMPONENT matlab
        )
        install(FILES ${MATLAB_FILES} DESTINATION matlab COMPONENT matlab)
        install(FILES ${MATLAB_GENERATED_FILES} DESTINATION matlab COMPONENT matlab)
    else()

        file(GLOB SHARED_LIB_HEADERS
             ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

        # custom command for building the wrap file
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
            COMMAND
                "${SWIG_EXECUTABLE}"
                "-matlab"
                "-c++"
                -o
                "helicsMEX.cpp"
                "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
                ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
            DEPENDS
                ../helics.i ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
                ${SHARED_LIB_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/matlab_maps.i
        )

        # add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp COMMAND
        # ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake
        # DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp )

        install(
            DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+helics
            DESTINATION matlab
            COMPONENT matlab
        )
        install(
            FILES
                ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m
                ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m
                ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m
            DESTINATION matlab
            COMPONENT matlab
        )
        install(
            FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/pisender.m
                ${CMAKE_CURRENT_SOURCE_DIR}/pireceiver.m
                ${CMAKE_CURRENT_SOURCE_DIR}/GetFullPath.m
            DESTINATION matlab
            COMPONENT matlab
        )
    endif()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m
        COMMAND
            ${CMAKE_COMMAND}
            -D
            LIBRARY_FILE=$<TARGET_LINKER_FILE:helicsSharedLib>
            -D
            BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
            -D
            LIBRARY_INCLUDE_LOCATION=${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/
            -D
            OTHER_INCLUDE_LOCATION=${PROJECT_BINARY_DIR}/src/helics/shared_api_library/
            -D
            SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P
            ${CMAKE_CURRENT_SOURCE_DIR}/generateMEXcreationScript.cmake
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    )

    if(WIN32)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
            COMMAND
                ${Matlab_MAIN_PROGRAM}
                -nojvm
                -nosplash
                -minimize
                -r
                "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
            DEPENDS
                ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        )
    elseif(APPLE)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
            COMMAND
                ${Matlab_MAIN_PROGRAM}
                -nojvm
                -nodisplay
                -nosplash
                -r
                "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
            DEPENDS
                ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        )
    else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
            COMMAND
                ${Matlab_MAIN_PROGRAM}
                -nojvm
                -nodisplay
                -nosplash
                -r
                mkhelicsMEXFile
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
                ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m helicsSharedLib
                ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        )
    endif()

    add_custom_target(
        helicsMEX ALL
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
            helicsSharedLib
    )
    set_target_properties(helicsMEX PROPERTIES FOLDER interfaces)

    file(GLOB HSHARED_LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
        DESTINATION matlab
        COMPONENT matlab
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        DESTINATION matlab
        COMPONENT matlab
    )

    if(WIN32)
        install(
            FILES $<TARGET_LINKER_FILE:helicsSharedLib>
            DESTINATION matlab
            COMPONENT matlab
        )
        install(
            FILES ${PROJECT_BINARY_DIR}/src/helics/shared_api_library/helics_export.h
            DESTINATION matlab/headers
            COMPONENT matlab
        )
        install(
            FILES helicsSharedLib.h libhelicsSharedLib.h
            DESTINATION matlab
            COMPONENT matlab
        )
    endif()

    install_key_files_with_comp(matlab)
    install(FILES ${HSHARED_LIB_SOURCES} DESTINATION matlab/headers COMPONENT matlab)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/generatehelicsMEXFile.m
        DESTINATION matlab
        COMPONENT matlab
    )

    file(GLOB EXTRA_MATLAB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/extra/*.m)

    message(STATUS "Extra ${EXTRA_MATLAB_FILES}")
    install(FILES ${EXTRA_MATLAB_FILES} DESTINATION matlab/+helics COMPONENT matlab)

endif()
