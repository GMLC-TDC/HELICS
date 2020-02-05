# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------

set(FMT_SILENT ON)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``14``, and ``17``.
endif()

set(SUPPORTS_VARIADIC_TEMPLATES ON)
set(SUPPORTS_USER_DEFINED_LITERALS ON)
set(FMT_HAS_VARIANT OFF)

# get the FMT header only library
add_subdirectory(ThirdParty/fmtlib)

set_target_properties(fmt PROPERTIES FOLDER Extern)
hide_variable(FMT_DOC)
hide_variable(FMT_INSTALL)
hide_variable(FMT_PEDANTIC)
hide_variable(FMT_TEST)
hide_variable(FMT_WERROR)
hide_variable(FMT_FUZZ)
hide_variable(FMT_CUDA_TEST)
