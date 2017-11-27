cmake_minimum_required (VERSION 3.4)
project (HELICS)

#-----------------------------------------------------------------------------
# HELICS Version number
#-----------------------------------------------------------------------------
set (HELICS_VERSION_MAJOR 0)
set (HELICS_VERSION_MINOR 9)
set (HELICS_VERSION_PATCH 0)

set (HELICS_DATE "11-30-17")

OPTION(BUILD_HELICS_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_TESTS)
enable_testing ()
endif(BUILD_HELICS_TESTS)

OPTION(BUILD_HELICS_EXAMPLES "Enable the test Executables to be built" OFF)

OPTION(BUILD_PYTHON "Build Python extension" OFF)

OPTION(BUILD_C_SHARED_LIB "Build the Shared Libraries with a C interface" OFF)
IF(NOT MSVC)
OPTION(BUILD_CXX_SHARED_LIB "Build a Shared Libraries of the CXX interface gcc/clang only" OFF)
ENDIF()

if (BUILD_PYTHON OR BUILD_C_SHARED_LIB OR BUILD_CXX_SHARED_LIB)
set(BUILD_SHARED_LIBS ON)
else()
OPTION(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF)
endif()

IF(BUILD_SHARED_LIBS OR USE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()

#-----------------------------------------------------------------------------
# General project wide configuration
#-----------------------------------------------------------------------------
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

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
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MINGW)
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(/bigobj /MP)
  add_compile_options(-W4  /EHsc /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)
ENDIF(MSVC)
  ENDIF(MINGW)
ENDIF(UNIX)

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
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_GENERATE_DOXYGEN_DOC)


# -------------------------------------------------------------
# Enable ZMQ
# -------------------------------------------------------------

OPTION(ZMQ_ENABLE "Enable ZMQ networking library" ON)
# If ZMQ library is enabled try to locate it and link against it

IF (ZMQ_ENABLE)

OPTION(ZMQ_USE_STATIC_LIBRARY
  "use the ZMQ static library" OFF)

SHOW_VARIABLE(ZMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${ZMQ_LIBRARY_PATH}")

SHOW_VARIABLE(ZMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${ZMQ_INCLUDE_PATH}")

  set(ZMQ_FIND_QUIETLY ON)
find_package(ZMQ)

if (NOT ZMQ_FOUND)
  OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" OFF)
  IF (AUTOBUILD_ZMQ)
  include(buildlibZMQ)
  build_libzmq()
  set(ZMQ_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
  find_package(ZMQ)
  ENDIF(AUTOBUILD_ZMQ)

endif(NOT ZMQ_FOUND)
if (ZMQ_FOUND)
   set(HELICS_HAVE_ZEROMQ TRUE)
   include_directories ("${ZMQ_INCLUDE_DIR}")
 endif(ZMQ_FOUND)
endif(ZMQ_ENABLE)

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
	target_link_libraries(helics-static ws2_32)
ENDIF(MINGW)

if (${ZMQ_FOUND})
	if (ZMQ_USE_STATIC_LIBRARY)
		set(ZMQ_DEPENDENCY ${ZMQ_STATIC_LIBRARY})
	else()
		set(ZMQ_DEPENDENCY ${ZMQ_LIBRARY})
	endif()
	target_link_libraries(helics-static ${ZMQ_DEPENDENCY})
else()
   if (ZMQ_ENABLE)
   message(WARNING "ZMQ not found")
   endif()
endif ()

if (${MPI_C_FOUND})
   target_link_libraries(helics-static ${MPI_C_LIBRARIES})
endif (${MPI_C_FOUND})



link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(helics-static ${Boost_LIBRARIES_core})


include(GNUInstallDirs)




OPTION(BUILD_PLAYER "Build a helics player" ON)
OPTION(BUILD_RECORDER "Build a helics recorder" ON)

# we aren't ready for this for the whole library yet
IF(BUILD_SHARED_LIBS AND NOT BUILD_CXX_SHARED_LIB)
    IF(UNIX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
		add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=hidden>)
    ELSE(UNIX)
        IF(MINGW)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
			add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=hidden>)
        ENDIF(MINGW)
    ENDIF(UNIX)
ENDIF()

IF(BUILD_CXX_SHARED_LIB)
	if (BUILD_C_SHARED_LIB OR BUILD_PYTHON)
		warning("Building the CXX shared library and C shared library in the same build is not advisable due to conflicting symbol visibility guidelines and is unlikely to work very well")
	endif()

	add_library(helics-shared SHARED helics-static)
endif()

FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")

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
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)

	set(helics_static_file "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
	
IF(BUILD_CXX_SHARED_LIB)
install (TARGETS helics-shared EXPORT helics-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)
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

set(helics_dependencies ${Boost_Libraries_Core})

IF (UNIX AND NOT APPLE)
	list(APPEND helics_dependencies rt)
ENDIF()

IF(MINGW)
	list(APPEND helics_dependencies ws2_32)
ENDIF(MINGW)

if (ZMQ_FOUND)
	list(APPEND helics_dependencies ${ZMQ_DEPENDENCY})
endif(ZMQ_FOUND)



#-----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
#-----------------------------------------------------------------------------
configure_file (
  "helics-config.h.in"
  "${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h"
  )
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/compiler-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics)
  install(DIRECTORY ThirdParty/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	  FILES_MATCHING PATTERN "helics_includes/*.h*")




include(CMakePackageConfigHelpers)


configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${HELICS_VERSION_MAJOR}.${HELICS_VERSION_MINOR}.${HELICS_VERSION_PATCH}
                                 COMPATIBILITY AnyNewerVersion)

configure_file(${PROJECT_NAME}Import.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
			   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake
              DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})