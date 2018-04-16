##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
IF(UNIX)
  add_compile_options(-fvisibility=hidden)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-fvisibility=hidden)
  ENDIF(MINGW)
ENDIF(UNIX)

set(helicsShared_headers
helics.h
api-data.h
ValueFederate.h
MessageFederate.h
MessageFilters.h
helicsCallbacks.h
)

set(helicsShared_sources
helicsExport.cpp
FederateExport.cpp
ValueFederateExport.cpp
MessageFederateExport.cpp
MessageFiltersExport.cpp
helicsCallbacks.cpp
internal/api_objects.h
)


add_library(helicsSharedLib SHARED ${helicsShared_sources} ${helicsShared_headers})

target_link_libraries(helicsSharedLib PRIVATE helics-static)
if (UNIX OR MINGW)
	if (NOT APPLE)
		target_link_libraries(helicsSharedLib PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.txt)
	endif(NOT APPLE)
endif()

if (HELICS_HAVE_ZEROMQ)
   target_include_directories (helicsSharedLib PRIVATE "${ZeroMQ_INCLUDE_DIR}")
endif(HELICS_HAVE_ZEROMQ)

set_target_properties (helicsSharedLib PROPERTIES FOLDER interfaces)

INSTALL(TARGETS helicsSharedLib EXPORT helics-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.6)
	INSTALL(FILES $<TARGET_LINKER_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs EXCLUDE_FROM_ALL)
	INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime EXCLUDE_FROM_ALL)
ELSE()
	INSTALL(FILES $<TARGET_LINKER_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs )
	INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
ENDIF()

IF (MSVC)
install(FILES $<TARGET_PDB_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL COMPONENT libs)
ENDIF(MSVC)
INSTALL(FILES ${helicsShared_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library COMPONENT headers)

