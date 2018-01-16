# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End



#check for clang 3.4 and the fact that CMAKE_CXX_STANDARD doesn't work yet for that compiler

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
	set(VERSION_OPTION -std=c++1y)
  else ()
	if (ENABLE_CXX_17)
		set(VERSION_OPTION -std=c++1z)
	endif(ENABLE_CXX_17)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  # c++14 becomes default in GCC 6.1
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.1)
    set(VERSION_OPTION -std=c++1y)
  else ()
    if (ENABLE_CXX_17)
		set(VERSION_OPTION -std=c++1z)
	endif(ENABLE_CXX_17)
  endif()
endif()

#boost libraries don't compile under /std:c++latest flag 1.66 might solve this issue
if (MSVC)
if (ENABLE_CXX_17)
set(VERSION_OPTION /std:c++latest)
endif(ENABLE_CXX_17)
endif()
