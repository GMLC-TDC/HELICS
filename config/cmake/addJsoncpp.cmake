
set(JSONCPP_WITH_TESTS OFF CACHE INTERNAL "")
set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE INTERNAL "")
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE INTERNAL "")
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE INTERNAL "")
#so json cpp exports to the correct target export
set(INSTALL_EXPORT ${HELICS_EXPORT_COMMAND} CACHE INTERNAL "")

set(JSONCPP_DISABLE_CCACHE ON CACHE INTERNAL "")

if (MSVC)
  option(JSONCPP_OBJLIB OFF "use jsoncpp objlib for linking object files instead of the normal target") 
else(MSVC)
 # for Everything but MSVC turn this on to not conflict with system jsoncpp if any
  option(JSONCPP_OBJLIB ON "use jsoncpp objlib for linking object files instead of the normal target") 
endif(MSVC)

mark_as_advanced(JSONCPP_OBJLIB)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()

if (BUILD_SHARED_LIBS)
   set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
   set(BUILD_SHARED_LIBS OFF)
endif()

if (CMAKE_INSTALL_INCLUDEDIR)
    set(OLD_CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional)
endif()

#these are internal variables used in JSONCPP that we know to be true based on the requirements in HELICS for newer compilers than JSONCPP supports
set(HAVE_CLOCALE ON)
set(HAVE_LOCALECONV ON)
set(COMPILER_HAS_DEPRECATED ON)
set(HAVE_STDINT_H ON)
set(HAVE_DECIMAL_POINT ON)
add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/jsoncpp" "${PROJECT_BINARY_DIR}/ThirdParty/jsoncpp")

if (OLD_CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR ${OLD_CMAKE_INSTALL_INCLUDEDIR})
endif()

if (OLD_BUILD_SHARED_LIBS)
   set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
endif()

mark_as_advanced(
JSONCPP_WITH_TESTS
JSONCPP_WITH_CMAKE_PACKAGE
JSONCPP_WITH_PKGCONFIG_SUPPORT
JSONCPP_WITH_POST_BUILD_UNITTEST
JSONCPP_USE_SECURE_MEMORY
JSONCPP_WITH_STRICT_ISO
JSONCPP_WITH_WARNING_AS_ERROR
)
