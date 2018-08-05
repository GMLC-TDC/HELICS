#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

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

add_subdirectory(comboFederates1)

if(BUILD_C_SHARED_LIB)
  add_subdirectory(CppInterface)
endif(BUILD_C_SHARED_LIB)
