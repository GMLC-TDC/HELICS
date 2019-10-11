# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(EXAMPLES_WARNINGS_AS_ERROR)
  # the examples had better compile without warnings otherwise they should not be
  # allowed
  if(WIN32)
    if(MSVC)
      add_compile_options(/WX)
    else(MSVC)
      add_compile_options(-Werror)
    endif(MSVC)
  else(WIN32)
    add_compile_options(-Werror)
  endif(WIN32)
endif()
if (HELICS_BUILD_APP_LIBRARY)
  add_subdirectory(comboFederates1)
endif()

if(NOT HELICS_DISABLE_C_SHARED_LIB)
  add_subdirectory(CppInterface)
endif()
