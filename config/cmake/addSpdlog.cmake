# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# -----------------------------------------------------------------------------
# create the spdlog target
# -----------------------------------------------------------------------------

if(NOT TARGET spdlog::spdlog)
    option(${PROJECT_NAME}_USE_EXTERNAL_SPDLOG "Use external copy of spdlog" OFF)
    mark_as_advanced(${PROJECT_NAME}_USE_EXTERNAL_SPDLOG)

    if(${PROJECT_NAME}_USE_EXTERNAL_SPDLOG)
        # find spdlog somewhere on the system NOTE/TODO: there may be odd corner cases when external
        # spdlog depends on an external fmt, but we tell HELICS to use the vendored copy of fmt
        # NOTE2: static spdlog should be built with CMAKE_POSITION_INDEPENDENT_CODE
        find_package(spdlog REQUIRED)
    else()
        # the spdlog install internally stores whether it was built with this on our vendored copy
        # of spdlog should never use its own fmt
        set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")

        # use the vendored SPDLOG library
        add_subdirectory(ThirdParty/spdlog)

        set_target_properties(spdlog PROPERTIES FOLDER Extern)
        hide_variable(SPDLOG_BUILD_ALL)
        hide_variable(SPDLOG_BUILD_BENCH)
        hide_variable(SPDLOG_BUILD_EXAMPLE)
        hide_variable(SPDLOG_BUILD_EXAMPLE_HO)
        hide_variable(SPDLOG_BUILD_SHARED)
        hide_variable(SPDLOG_BUILD_TESTS)
        hide_variable(SPDLOG_BUILD_TESTS_HO)
        hide_variable(SPDLOG_BUILD_WARNINGS)
        hide_variable(SPDLOG_ENABLE_PCH)
        hide_variable(SPDLOG_NO_ATOMIC_LEVELS)
        hide_variable(SPDLOG_NO_EXCEPTIONS)
        hide_variable(SPDLOG_NO_THREAD_ID)
        hide_variable(SPDLOG_NO_TLS)
        hide_variable(SPDLOG_PREVENT_CHILD_FD)
        hide_variable(SPDLOG_SANITIZE_ADDRESS)
        hide_variable(SPDLOG_TIDY)
        hide_variable(SPDLOG_WCHAR_FILENAMES)
        hide_variable(SPDLOG_WCHAR_SUPPORT)
        hide_variable(SPDLOG_DISABLE_DEFAULT_LOGGER)
        hide_variable(SPDLOG_FMT_EXTERNAL_HO)
        hide_variable(SPDLOG_INSTALL)
        hide_variable(SPDLOG_CLOCK_COARSE)
        hide_variable(SPDLOG_USE_STD_FORMAT)
    endif()
endif()
