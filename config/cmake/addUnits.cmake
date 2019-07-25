
#so units cpp exports to the correct target export
set(UNITS_INSTALL_EXPORT EXPORT helics-targets CACHE INTERNAL "")

if (MSVC)
  option(HELICS_UNITS_OBJLIB OFF "use the units objlib for linking object files instead of the normal target") 
else(MSVC)
 # for Everything but MSVC turn this on to not conflict with system jsoncpp if any
  option(HELICS_UNITS_OBJLIB ON "use the units objlib for linking object files instead of the normal target") 
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
	set(UNITS_LIBRARY_TYPE OBJECT)
endif(HELICS_UNITS_OBJLIB)

add_subdirectory("${HELICS_SOURCE_DIR}/ThirdParty/units" "${PROJECT_BINARY_DIR}/ThirdParty/units")

if (OLD_CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR ${OLD_CMAKE_INSTALL_INCLUDEDIR})
endif()

set_target_properties(unitslib PROPERTIES FOLDER Extern)

