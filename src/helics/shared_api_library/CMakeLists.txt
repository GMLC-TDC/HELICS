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

target_link_libraries(helicsSharedLib PRIVATE helics-static)

if (HELICS_HAVE_ZEROMQ)
   target_include_directories (helicsSharedLib PRIVATE "${ZeroMQ_INCLUDE_DIR}")
endif(HELICS_HAVE_ZEROMQ)

#this is redundant but I need it once for the targets and once for cpack
# cant use COMPONENT and distinct directories together
INSTALL(TARGETS helicsSharedLib EXPORT helics-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

INSTALL(FILES $<TARGET_LINKER_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)
INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)

INSTALL(FILES ${helicsShared_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library COMPONENT headers)

