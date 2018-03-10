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
)

set(helicsShared_sources
helicsExport.cpp
FederateExport.cpp
ValueFederateExport.cpp
MessageFederateExport.cpp
MessageFiltersExport.cpp
internal/api_objects.h
)


add_library(helicsSharedLib SHARED ${helicsShared_sources} ${helicsShared_headers})

target_link_libraries(helicsSharedLib helics-static)

if (HELICS_HAVE_ZEROMQ)
   target_include_directories (helicsSharedLib PRIVATE "${ZeroMQ_INCLUDE_DIR}")
endif(HELICS_HAVE_ZEROMQ)

INSTALL(TARGETS helicsSharedLib EXPORT helics-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

INSTALL(FILES ${helicsShared_headers} DESTINATION include/helics/shared_api_library)


if (WIN32 AND BUILD_PYTHON)
	INSTALL(TARGETS helicsSharedLib DESTINATION ${CMAKE_INSTALL_PREFIX}/python/)
endif(WIN32 AND BUILD_PYTHON)

