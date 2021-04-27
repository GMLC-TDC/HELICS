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

set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "")

# get the SPDLOG library
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
