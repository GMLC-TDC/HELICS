cmake_minimum_required (VERSION 3.4)
project (HELICS)

#-----------------------------------------------------------------------------
# HELICS Version number
#-----------------------------------------------------------------------------
set (HELICS_VERSION_MAJOR 1)
set (HELICS_VERSION_MINOR 0)
set (HELICS_VERSION_PATCH 0)
set (HELICS_VERSION_BUILD alpha)
set (HELICS_DATE "01-18-18")

OPTION(BUILD_HELICS_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_TESTS)
enable_testing ()
endif(BUILD_HELICS_TESTS)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

OPTION(BUILD_HELICS_EXAMPLES "Enable the test Executables to be built" ON)

OPTION(BUILD_PYTHON "Build Python extension" OFF)
OPTION(GENERATE_PYTHON "Generate Python extension" ON)

OPTION(BUILD_MATLAB "Build MATLAB extension" OFF)

OPTION(BUILD_C_SHARED_LIB "Build the Shared Libraries with a C interface" ON)
IF(NOT MSVC)
OPTION(BUILD_CXX_SHARED_LIB "Build a Shared Libraries of the CXX interface gcc/clang only" OFF)
ENDIF()

if (BUILD_PYTHON OR BUILD_C_SHARED_LIB OR BUILD_CXX_SHARED_LIB)
set(BUILD_SHARED_LIBS ON)
else()
OPTION(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF)
endif()

OPTION(BUILD_JAVA "Build Java extension" OFF)
if (BUILD_PYTHON OR BUILD_C_SHARED_LIB)
set(BUILT_C_SHARED_LIB ON)
endif()

IF(BUILD_SHARED_LIBS OR USE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()




#-----------------------------------------------------------------------------
# General project wide configuration
#-----------------------------------------------------------------------------

if (NOT CMAKE_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX d)
endif()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/config/cmake/")

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty")
include_directories ("${PROJECT_BINARY_DIR}/libs/include")

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hiding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
    SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)




# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()

# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID}")
IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
  #this options produces lots of warning but is useful for checking ever once in a while with Clang, GCC warning notices with this aren't as useful
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)

   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  option (USE_LIBCXX "Use LibC++ vs as opposed to the default" OFF)
  IF(USE_LIBCXX)
      add_compile_options(-stdlib=libc++)
      link_libraries("-stdlib=libc++")
     ENDIF(USE_LIBCXX)
ELSE(UNIX)
  IF(MINGW)
  add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
  #this options produces lots of warning but is useful for checking ever once in a while with Clang, GCC warning notices with this aren't as useful
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)

  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MINGW)
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(/MP)
  add_compile_options(-W4  /EHsc /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484 /wd4702 /wd4996 )
  add_compile_options(/sdl)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)
ENDIF(MSVC)
  ENDIF(MINGW)
ENDIF(UNIX)

IF (BUILD_MATLAB OR USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
ENDIF ()

# -------------------------------------------------------------
# Check and set latest CXX Standard supported by compiler
# -------------------------------------------------------------
OPTION(ENABLE_CXX_17 "set to on to enable C++17 compilation features" OFF)
include(CheckLatestCXXStandardOption)
IF (VERSION_OPTION)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
ELSE ()
	set(CMAKE_CXX_STANDARD 14)
ENDIF ()

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${PROJECT_BINARY_DIR}/libs/include/helics/)
include(configGenerator)

option (HELICS_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (HELICS_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_GENERATE_DOXYGEN_DOC)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)


# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

OPTION(ZeroMQ_ENABLE "Enable ZeroMQ networking library" ON)
# If ZeroMQ library is enabled try to locate it and link against it

IF (ZeroMQ_ENABLE)

include(addZeroMQ)

if (ZeroMQ_FOUND)
   set(HELICS_HAVE_ZEROMQ TRUE)
   include_directories ("${ZeroMQ_INCLUDE_DIR}")
 endif(ZeroMQ_FOUND)
endif(ZeroMQ_ENABLE)

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

OPTION(MPI_ENABLE "Enable MPI networking library" OFF)
IF (MPI_ENABLE)
find_package(MPI)
set(MPI_C_FOUND FALSE)
if (${MPI_C_FOUND})
   set(HELICS_HAVE_MPI TRUE)
   include_directories("${MPI_C_INCLUDE_PATH}")
endif (${MPI_C_FOUND})
ENDIF(MPI_ENABLE)


# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
include(addBoost)

add_library(helics-static STATIC
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
)
target_link_libraries(helics-static Threads::Threads)
IF (UNIX AND NOT APPLE)
target_link_libraries(helics-static rt)
ENDIF()

IF(MINGW)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	if (BOOST_VERSION_LEVEL GREATER 1)
	#set(DISABLE_TCP_CORE TRUE)
		set(DISABLE_TCP_CORE FALSE)
		target_link_libraries(helics-static wsock32 ws2_32)
	else()
		set(DISABLE_TCP_CORE TRUE)
		message(status, "disabling TCP core |${DISABLE_TCP_CORE}|")
		target_link_libraries(helics-static ws2_32)
	endif()

	else()
		target_link_libraries(helics-static wsock32 ws2_32)
	endif()

ENDIF(MINGW)

if (${ZeroMQ_FOUND})
	if (ZMQ_USE_STATIC_LIBRARY)
		set(ZeroMQ_DEPENDENCY ${ZeroMQ_STATIC_LIBRARY})
		ADD_DEFINITIONS(-DZMQ_STATIC)
	else()
		set(ZeroMQ_DEPENDENCY ${ZeroMQ_LIBRARY})
	endif()
	message(STATUS "zmq dep ${ZeroMQ_DEPENDENCY}")
	target_link_libraries(helics-static ${ZeroMQ_DEPENDENCY})
else()
   if (ZeroMQ_ENABLE)
   message(WARNING "ZeroMQ not found")
   endif()
endif ()

if (${MPI_C_FOUND})
   target_link_libraries(helics-static ${MPI_C_LIBRARIES})
endif (${MPI_C_FOUND})



link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(helics-static ${Boost_LIBRARIES_core})


include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/bin" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (NOT USE_BOOST_STATIC_LIBS)
list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
endif()

OPTION(BUILD_PLAYER "Build a helics player" ON)
OPTION(BUILD_RECORDER "Build a helics recorder" ON)

# we aren't ready for this for the whole library yet
IF(BUILD_SHARED_LIBS AND NOT BUILD_CXX_SHARED_LIB)
    IF(UNIX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=default>)
		add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=default>)
    ELSE(UNIX)
        IF(MINGW)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=default>)
			add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=default>)
        ENDIF(MINGW)
    ENDIF(UNIX)
ENDIF()

IF(BUILD_CXX_SHARED_LIB)
	if (BUILD_C_SHARED_LIB OR BUILD_PYTHON)
		warning("Building the CXX shared library and C shared library in the same build is not advisable due to conflicting symbol visibility guidelines and is unlikely to work very well")
	endif()

	add_library(helics-shared SHARED src/empty.cpp)
	target_link_libraries(helics-shared helics-static)
	if (WIN32)
		IF(MINGW)
			set_target_properties(helics-shared PROPERTIES
			LINK_FLAGS "-Wl,--whole-archive")
		ELSE(MINGW)
			set_target_properties(helics-shared PROPERTIES
			LINK_FLAGS "/WHOLEARCHIVE")
		endif()

	elseif (APPLE)
		set_target_properties(helics-shared PROPERTIES
        LINK_FLAGS "-Wl,-all_load")
	else ()
		set_target_properties(helics-shared PROPERTIES
        LINK_FLAGS "-Wl,--whole-archive")
	endif ()

endif()

if (WIN32)
FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")
else()
set(KEY_LIBRARY_FILES "")
endif()

OPTION(DISABLE_LOGGING "disable all normal, debug, and trace logging in HELICS" OFF)
if (NOT DISABLE_LOGGING)
OPTION(DISABLE_TRACE_LOGGING "disable trace logging" OFF)
OPTION(DISABLE_DEBUG_LOGGING "disable debug logging" OFF)
endif()


#-----------------------------------------------------------------------------
# CMAKE Subdirectories
#-----------------------------------------------------------------------------

add_subdirectory (src)

if (BUILD_HELICS_TESTS)
if (MSVC)
SET(ENABLE_TEST_TIMEOUTS OFF)
ELSE(MSVC)
OPTION(ENABLE_TEST_TIMEOUTS "Enable timeouts on test" ON)
ENDIF(MSVC)
add_subdirectory (tests)


#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
include (CTest)

endif(BUILD_HELICS_TESTS)

if (BUILD_HELICS_EXAMPLES)
add_subdirectory (examples)

endif(BUILD_HELICS_EXAMPLES)

INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin)
# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

add_subdirectory(swig)


set(HELICS_CMAKECONFIG_INSTALL_DIR "cmake/${PROJECT_NAME}" CACHE STRING "install path for HELICSConfig.cmake")

install (TARGETS helics-static EXPORT helics-targets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

	set(helics_static_file "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_STATIC_LIBRARY_SUFFIX}")

IF(BUILD_CXX_SHARED_LIB)
install (TARGETS helics-shared EXPORT helics-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	set(helics_cxx_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_SHARED_LIBRARY_SUFFIX}")
ELSE (BUILD_CXX_SHARED_LIB)
	set(helics_cxx_shared_file )
ENDIF(BUILD_CXX_SHARED_LIB)

IF(BUILD_C_SHARED_LIB OR BUILD_PYTHON)
	set(helics_c_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_SHARED_LIBRARY_SUFFIX}")
ELSE()
	set(helics_c_shared_file )
ENDIF()


install (EXPORT helics-targets DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})

set(helics_dependencies ${Boost_LIBRARIES_core})

IF (UNIX AND NOT APPLE)
	list(APPEND helics_dependencies rt)
ENDIF()

IF(MINGW)
	list(APPEND helics_dependencies wsock32 ws2_32)
ENDIF(MINGW)

if (ZeroMQ_FOUND)
	list(APPEND helics_dependencies ${ZeroMQ_DEPENDENCY})
endif(ZeroMQ_FOUND)



#-----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
#-----------------------------------------------------------------------------
configure_file (
  "config/helics-config.h.in"
  "${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h"
  )
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/compiler-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)
  install(DIRECTORY ThirdParty/cereal DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY ThirdParty/helics_includes DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})



include(CMakePackageConfigHelpers)


configure_package_config_file(config/${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${HELICS_VERSION_MAJOR}.${HELICS_VERSION_MINOR}.${HELICS_VERSION_PATCH}.${HELICS_VERSION_BUILD}
                                 COMPATIBILITY AnyNewerVersion)

configure_file(config/${PROJECT_NAME}Import.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
			   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake
              DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})
