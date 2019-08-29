
#so units cpp exports to the correct target export
set(UNITS_LIBRARY_EXPORT_COMMAND ${HELICS_EXPORT_COMMAND} CACHE INTERNAL "")

if (MSVC)
  option(HELICS_UNITS_OBJLIB "use the units objlib for linking object files instead of the normal target" OFF)
else(MSVC)
  option(HELICS_UNITS_OBJLIB "use the units objlib for linking object files instead of the normal target" ON)
endif(MSVC)

mark_as_advanced(HELICS_UNITS_OBJLIB)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``11``, ``14``, and ``17``.
endif()


if (CMAKE_INSTALL_INCLUDEDIR)
    set(OLD_CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
    set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional)
endif()

if (HELICS_UNITS_OBJLIB)
	set(BUILD_UNITS_OBJECT_LIBRARY  ON CACHE INTERNAL "")
	set(BUILD_UNITS_STATIC_LIBRARY OFF CACHE INTERNAL "")
	set(BUILD_UNITS_SHARED_LIBRARY OFF CACHE INTERNAL "")
else()
	set(BUILD_UNITS_OBJECT_LIBRARY  OFF CACHE INTERNAL "")
	set(BUILD_UNITS_STATIC_LIBRARY ON CACHE INTERNAL "")
	set(BUILD_UNITS_SHARED_LIBRARY OFF CACHE INTERNAL "")
endif()
add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/units" "${PROJECT_BINARY_DIR}/ThirdParty/units")

if (OLD_CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR ${OLD_CMAKE_INSTALL_INCLUDEDIR})
endif()
if (HELICS_UNITS_OBJLIB)
	set_target_properties(units-object PROPERTIES FOLDER Extern)
else()
	set_target_properties(units-static PROPERTIES FOLDER Extern)
	add_library(HELICS::units ALIAS units-static)
endif()

HIDE_VARIABLE(BUILD_UNITS_FUZZ_TARGETS)
