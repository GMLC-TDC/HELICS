# LLNS Copyright Start
# Copyright (c) 2017, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End

include(CheckCXXCompilerFlag)
message(STATUS "CMAKE_CXX_STANDARD: --${CMAKE_CXX_STANDARD}")
if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
message(STATUS "CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

if(MSVC)
    if (CMAKE_CXX_STANDARD EQUAL 14)
		set(CXX_STANDARD_FLAG /std:c++14)
    else()
        set(CXX_STANDARD_FLAG /std:c++latest)
    endif()
else()

if (CMAKE_CXX_STANDARD EQUAL 20)
  check_cxx_compiler_flag(-std=c++20 has_std_20_flag)
  if (has_std_20_flag)
    set(CXX_STANDARD_FLAG -std=c++20)
  else()
	 check_cxx_compiler_flag(-std=c++2a has_std_2a_flag)
	 if (has_std_2a_flag)
       set(CXX_STANDARD_FLAG -std=c++2a)
     endif ()
  endif()
elseif (CMAKE_CXX_STANDARD EQUAL 17)
  check_cxx_compiler_flag(-std=c++17 has_std_17_flag)
  if (has_std_17_flag)
    set(CXX_STANDARD_FLAG -std=c++17)
  else()
	 check_cxx_compiler_flag(-std=c++1z has_std_1z_flag)
	 if (has_std_1z_flag)
       set(CXX_STANDARD_FLAG -std=c++1z)
     endif ()
  endif()
elseif (CMAKE_CXX_STANDARD EQUAL 14)
  check_cxx_compiler_flag(-std=c++14 has_std_14_flag)
  if (has_std_14_flag)
    set(CXX_STANDARD_FLAG -std=c++14)
  else()
	 check_cxx_compiler_flag(-std=c++1y has_std_1y_flag)
	 if (has_std_1y_flag)
       set(CXX_STANDARD_FLAG -std=c++1y)
     endif ()
  endif()
else()
  message(FATAL_ERROR "HELICS requires C++14 or Higher")
endif ()

endif()
#set(CMAKE_REQUIRED_FLAGS ${CXX_STANDARD_FLAG})

# boost libraries don't compile under /std:c++latest flag 1.66 might solve this
# issue
