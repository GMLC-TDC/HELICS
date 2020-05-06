# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# This is a compatibility file for pre-CMake 3.12 versions using the old python find method
function(helics_python3_add_library HELICS_LIBNAME)
    add_library(${HELICS_LIBNAME} MODULE ${ARGN})

    # Add path to Python include directory
    target_include_directories(${HELICS_LIBNAME} PUBLIC ${HELICS_Python_INCLUDE_DIRS})

    # Get list of libraries to link with for Python module
    if(NOT MSVC)
        execute_process(
            COMMAND
                ${HELICS_Python_EXECUTABLE} "-c"
                "from distutils import sysconfig; print(sysconfig.get_config_var('BLDSHARED').split(' ', 1)[1])"
            OUTPUT_VARIABLE pymodule_libs
        )

        # Clean-up leading and trailing whitespace
        string(STRIP ${pymodule_libs} pymodule_libs)
    else()
        set(pymodule_libs "")
    endif()

    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        # https://groups.google.com/a/continuum.io/d/msg/anaconda/057P4uNWyCU/Ie
        # m6Ot%20jBCQAJ
        set_target_properties(
            ${HELICS_LIBNAME}
            PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
        )
    else()
        target_link_libraries(
            ${HELICS_LIBNAME}
            PUBLIC ${pymodule_libs} ${HELICS_Python_LIBRARIES}
        )
    endif()

    if(NOT MSVC)
        # Get list of compiler flags for compiling Python module
        execute_process(
            COMMAND
                ${HELICS_Python_EXECUTABLE} "-c"
                "from distutils import sysconfig; print(sysconfig.get_config_var('CFLAGS').split(' ', 1)[1])"
            OUTPUT_VARIABLE pymodule_includes
        )

        # Clean-up leading and trailing whitespace, convert into a CMake
        # ;-separated list
        string(STRIP ${pymodule_includes} pymodule_includes)
        string(REPLACE " " ";" pymodule_includes "${pymodule_includes}")
        target_compile_options(${HELICS_LIBNAME} PUBLIC ${pymodule_includes})

        set_target_properties(${HELICS_LIBNAME} PROPERTIES FOLDER interfaces)
    endif()

    # This will get the correct extension for a Python module
    # On Windows it is always .pyd
    # On other platforms it will include the right platform tag
    # - Old Python versions will just return .so
    # - New Python versions will return a tag such as .cpython-37m-x86_64-linux-gnu.so
    execute_process(
        COMMAND
            ${HELICS_Python_EXECUTABLE} "-c"
            "import sysconfig; ext_suffix=sysconfig.get_config_var('EXT_SUFFIX'); print(ext_suffix if ext_suffix is not None else sysconfig.get_config_var('SO'))"
        RESULT_VARIABLE pymodule_rc
        OUTPUT_VARIABLE pymodule_extsuffix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if("${pymodule_rc}" STREQUAL "0")
        set_property(TARGET ${HELICS_LIBNAME} PROPERTY SUFFIX "${pymodule_extsuffix}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Ensure Windows uses .pyd
        set_property(TARGET ${HELICS_LIBNAME} PROPERTY SUFFIX ".pyd")
    endif()
endfunction()
