# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------

option(${PROJECT_NAME}_USE_EXTERNAL_FMT "Use external copy of {fmt}" OFF)
mark_as_advanced(${PROJECT_NAME}_USE_EXTERNAL_FMT)

if(${PROJECT_NAME}_USE_EXTERNAL_FMT)
    # Note: static fmt should be built with CMAKE_POSITION_INDEPENDENT_CODE
    find_package(fmt REQUIRED)
else()
    set(FMT_SILENT ON)

    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17) # Supported values are ``14``, and ``17``.
    endif()

    set(SUPPORTS_VARIADIC_TEMPLATES ON)
    set(SUPPORTS_USER_DEFINED_LITERALS ON)
    set(FMT_HAS_VARIANT ON)
    set(type STRING CACHE INTERNAL "")

    if (CYGWIN)
        set(FMT_OS
            OFF
            CACHE INTERNAL ""
        )
    else()
        set(FMT_OS
            ON
            CACHE INTERNAL ""
        )
    endif()
    # add the vendored FMT header only library
    add_subdirectory(ThirdParty/fmtlib)

    set_target_properties(fmt PROPERTIES FOLDER Extern)
    hide_variable(FMT_DOC)
    hide_variable(FMT_INSTALL)
    hide_variable(FMT_PEDANTIC)
    hide_variable(FMT_TEST)
    hide_variable(FMT_WERROR)
    hide_variable(FMT_FUZZ)
    hide_variable(FMT_CUDA_TEST)
    hide_variable(FMT_DEBUG_POSTFIX)
    hide_variable(FMT_INC_DIR)
    hide_variable(FMT_MODULE)
    hide_variable(FMT_SYSTEM_HEADERS)
endif()
