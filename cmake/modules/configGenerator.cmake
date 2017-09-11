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
  set(HAVE_VARIABLE_TEMPLATES ON)
  set(HAVE_EXP_STRING_VIEW ON)
  set(HAVE_EXP_OPTIONAL ON)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
	set(VERSION_OPTION -std=c++1y)
  endif()
  #clang doesn't work with C++17 if GCC is installed so we need to disable this for the time being 
  #if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.9)
#	set(HAVE_CLAMP ON)
#	set(HAVE_HYPOT3 ON)
#	set(HAVE_CONSTEXPR_IF ON)
#	set(VERSION_OPTION -std=c++1z)
 # endif()
 # if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0)
#	set(HAVE_VARIANT ON)
#	set(HAVE_STRING_VIEW ON)
#	set(HAVE_CONSTEXPR_IF ON)
#	set(HAVE_FALLTHROUGH ON)
#	set(HAVE_OPTIONAL ON)
#	set(HAVE_UNUSED ON)
#	message(STATUS "gt 4.0 ${CMAKE_CXX_COMPILER_VERSION}")
#  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(HAVE_EXP_STRING_VIEW ON)
  set(HAVE_EXP_OPTIONAL ON)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
	set(HAVE_VARIABLE_TEMPLATES ON)
  endif()
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
	set(HAVE_VARIANT ON)
	#set(HAVE_STRING_VIEW ON)
	set(HAVE_CONSTEXPR_IF ON)
	set(HAVE_FALLTHROUGH ON)
	set(HAVE_UNUSED ON)
	set(HAVE_CLAMP ON)
	set(HAVE_HYPOT3 ON)
	set(HAVE_OPTIONAL ON)
	set(VERSION_OPTION -std=c++1z)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
#intel doesn't really have anything beyond the minimum
  if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 17.0)
  
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  message(STATUS "win_comp_version ${CMAKE_CXX_COMPILER_VERSION}")
  set(HAVE_VARIABLE_TEMPLATES ON)
endif()

if (NOT NO_CONFIG_GENERATION)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/libs/include/config.h)
endif(NOT NO_CONFIG_GENERATION)