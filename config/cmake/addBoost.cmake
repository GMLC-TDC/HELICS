##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

SHOW_VARIABLE(BOOST_INSTALL_PATH PATH "Boost root directory" "${BOOST_INSTALL_PATH}")

if(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)
  set(Boost_USE_MULTITHREADED OFF)   # Needed if MT libraries not built
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
else(UNIX)
  if(MSYS)
	option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  else(MSYS)
  #this will be MSYS or stand alone Mingw
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
  endif(MSYS)
endif(UNIX)

mark_as_advanced(USE_BOOST_STATIC_LIBS)

if (USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_STATIC ON)
endif ()

mark_as_advanced(USE_BOOST_STATIC_LIBS)

if (MSVC)

set (boost_versions
boost_1_68_0
boost_1_67_0
boost_1_66_0
boost_1_65_1
boost_1_65_0
boost_1_64_0
boost_1_63_0
boost_1_62_0
boost_1_61_0
)

set(poss_prefixes
C:
C:/boost
C:/local
C:/local/boost
C:/Libraries
D:
D:/boost
D:/local
D:/local/boost
)

# create an empty list
list(APPEND boost_paths "")
mark_as_advanced(BOOST_INSTALL_PATH)
foreach( dir ${poss_prefixes})
	foreach( boostver ${boost_versions})
		if (IS_DIRECTORY ${dir}/${boostver})
			if (EXISTS ${dir}/${boostver}/boost/version.hpp)
				list(APPEND boost_paths ${dir}/${boostver})
			endif()
		endif()
	endforeach()
endforeach()

find_path(BOOST_TEST_PATH
			NAMES 			boost/version.hpp
			HINTS	ENV BOOST_INSTALL_PATH
			PATHS		${BOOST_INSTALL_PATH}
						${boost_paths}
		)

		if (BOOST_TEST_PATH)
			set(BOOST_ROOT ${BOOST_TEST_PATH})
		endif(BOOST_TEST_PATH)
else(MSVC)
	if (NOT BOOST_ROOT)
		if (BOOST_INSTALL_PATH)
			set(BOOST_ROOT "${BOOST_INSTALL_PATH}")
		elseif ($ENV{BOOST_INSTALL_PATH})
			set(BOOST_ROOT "$ENV{BOOST_INSTALL_PATH}")
		else()
			set(BOOST_ROOT "$ENV{BOOST_ROOT}")
		endif()
	endif()
endif(MSVC)

HIDE_VARIABLE(BOOST_TEST_PATH)

if (NOT BOOST_REQUIRED_LIBRARIES)
	set(BOOST_REQUIRED_LIBRARIES program_options unit_test_framework filesystem system date_time timer chrono)
endif()

# Minimum version of Boost required for building HELICS
set(BOOST_MINIMUM_VERSION 1.58)
set(Boost_USE_STATIC_LIBS   ${USE_BOOST_STATIC_LIBS})
find_package(Boost ${BOOST_MINIMUM_VERSION} COMPONENTS ${BOOST_REQUIRED_LIBRARIES} REQUIRED)

# Minimum version of Boost required for building test suite
if (Boost_VERSION LESS 106100)
  set(BOOST_VERSION_LEVEL 0)
elseif (Boost_VERSION GREATER 106599)
	#in 1.166 there were some changes to asio and inclusion of beast that will enable other components
	set(BOOST_VERSION_LEVEL 2)
else()
	set(BOOST_VERSION_LEVEL 1)
endif()


#message(STATUS "Using Boost include files : ${Boost_INCLUDE_DIR}")
#message(STATUS "Using Boost libraries in : ${Boost_LIBRARY_DIRS}")
#message(STATUS "Using Boost libraries : ${Boost_LIBRARIES}")
set(modifier,"")
foreach(loop_var ${Boost_LIBRARIES})
	if (${loop_var} MATCHES "debug")
		list(INSERT modifier 0 ${loop_var})
	elseif(${loop_var} MATCHES "optimized")
		list(INSERT modifier 0 ${loop_var})
	else()
		#message("Boost_LIBRARIES ${loop_var}")
		if(${loop_var} MATCHES "unit_test")
			list(APPEND Boost_LIBRARIES_test ${modifier} ${loop_var})
		else()
			list(APPEND Boost_LIBRARIES_core ${modifier} ${loop_var})
		endif()
		if (${modifier} MATCHES "debug")
			if(${loop_var} MATCHES "unit_test")
				list(APPEND Boost_LIBRARIES_test_debug ${loop_var})
			else()
				list(APPEND Boost_LIBRARIES_core_debug ${loop_var})
			endif()
		else()
			if(${loop_var} MATCHES "unit_test")
				list(APPEND Boost_LIBRARIES_test_release ${loop_var})
			else()
				list(APPEND Boost_LIBRARIES_core_release ${loop_var})
			endif()
		endif()
		list(LENGTH modifier modifier_size)
		if (modifier_size GREATER 0)
			list(REMOVE_AT modifier -1)
		endif()
	endif()
endforeach(loop_var)

############################################################
# Add boost targets to use
#####################################################

if (${Boost_USE_STATIC_LIBS})
	add_library(Boostlibs::core STATIC IMPORTED)
	add_library(Boostlibs::test STATIC IMPORTED)
else()
	add_library(Boostlibs::core UNKNOWN IMPORTED)
	add_library(Boostlibs::test UNKNOWN IMPORTED)
endif()

list(LENGTH Boost_LIBRARIES_core_debug core_debug_size)
list(LENGTH Boost_LIBRARIES_core_release core_release_size)

math(EXPR rng "${core_release_size} - 1")

if (core_debug_size EQUAL 0)
	list(GET Boost_LIBRARIES_core_release 0 first_lib)
	set_target_properties(Boostlibs::core PROPERTIES IMPORTED_LOCATION ${first_lib})

    foreach(item RANGE 1 ${rng})
		list(GET Boost_LIBRARIES_core_release ${item} next_lib)
		string(RANDOM LENGTH 7 rand_name)
		if (${Boost_USE_STATIC_LIBS})
			add_library(Boostlibs::${rand_name} STATIC IMPORTED)
		else()
			add_library(Boostlibs::${rand_name} UNKNOWN IMPORTED)
		endif()
		set_target_properties(Boostlibs::${rand_name} PROPERTIES IMPORTED_LOCATION ${next_lib})
		list(APPEND boost_core_deps Boostlibs::${rand_name})
	endforeach()
else()
	list(GET Boost_LIBRARIES_core_release 0 first_lib_r)
	list(GET Boost_LIBRARIES_core_debug 0 first_lib_d)
	set_target_properties(Boostlibs::core PROPERTIES IMPORTED_LOCATION_DEBUG ${first_lib_d} IMPORTED_LOCATION_RELEASE ${first_lib_r})

	foreach(item RANGE 1 ${rng})
		list(GET Boost_LIBRARIES_core_release ${item} next_lib_r)
		list(GET Boost_LIBRARIES_core_debug ${item} next_lib_d)
		string(RANDOM LENGTH 7 rand_name)
		if (${Boost_USE_STATIC_LIBS})
			add_library(Boostlibs::${rand_name} STATIC IMPORTED)
		else()
			add_library(Boostlibs::${rand_name} UNKNOWN IMPORTED)
		endif()
		set_target_properties(Boostlibs::${rand_name} PROPERTIES IMPORTED_LOCATION_DEBUG ${next_lib_d} IMPORTED_LOCATION_RELEASE ${next_lib_r})
		list(APPEND boost_core_deps Boostlibs::${rand_name})
	endforeach()
endif()

set_target_properties(Boostlibs::core Boostlibs::test PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
set_target_properties(Boostlibs::core PROPERTIES INTERFACE_LINK_LIBRARIES "${boost_core_deps}")

if (Boost_LIBRARIES_test_debug)
	set_target_properties(Boostlibs::test PROPERTIES IMPORTED_LOCATION_DEBUG "${Boost_LIBRARIES_test_debug}" IMPORTED_LOCATION_RELEASE "${Boost_LIBRARIES_test_release}")
else()
	set_target_properties(Boostlibs::test PROPERTIES IMPORTED_LOCATION "${Boost_LIBRARIES_test_release}")
endif()

#message(STATUS "Using Boost core debug libraries : ${Boost_LIBRARIES_core_debug}")
#message(STATUS "Using Boost core release libraries : ${Boost_LIBRARIES_core_release}")

