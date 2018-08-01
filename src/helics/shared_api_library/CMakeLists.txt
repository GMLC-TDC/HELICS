#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

set(helicsShared_headers
    helics.h
    api-data.h
    ValueFederate.h
    MessageFederate.h
    MessageFilters.h
    helicsCallbacks.h)

set(helicsShared_sources
    helicsExport.cpp
    FederateExport.cpp
    ValueFederateExport.cpp
    MessageFederateExport.cpp
    MessageFiltersExport.cpp
    helicsCallbacks.cpp
    internal/api_objects.h)

include(GenerateExportHeader)

add_library(helicsSharedLib SHARED ${helicsShared_sources} ${helicsShared_headers})

add_library(HELICS::helicsSharedLib ALIAS helicsSharedLib)
generate_export_header(helicsSharedLib BASE_NAME helics)

target_include_directories(helicsSharedLib
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set_target_properties(helicsSharedLib
                      PROPERTIES CXX_VISIBILITY_PRESET
                                 hidden
                                 C_VISIBILITY_PRESET
                                 hidden)
target_link_libraries(helicsSharedLib PRIVATE helics-static)
if(UNIX OR MINGW)
  if(NOT APPLE)
    target_link_libraries(
      helicsSharedLib
      PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.txt)
  endif(NOT APPLE)
endif()

set_target_properties(helicsSharedLib PROPERTIES FOLDER interfaces)

install(TARGETS helicsSharedLib
        EXPORT helics-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.6)
  install(FILES $<TARGET_LINKER_FILE:helicsSharedLib>
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT libs EXCLUDE_FROM_ALL)
  install(FILES $<TARGET_FILE:helicsSharedLib>
          DESTINATION ${CMAKE_INSTALL_BINDIR}
          COMPONENT
          runtime EXCLUDE_FROM_ALL)
else()
  install(FILES $<TARGET_LINKER_FILE:helicsSharedLib>
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT libs)
  install(FILES $<TARGET_FILE:helicsSharedLib>
          DESTINATION ${CMAKE_INSTALL_BINDIR}
          COMPONENT
          runtime)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics_export.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library
        COMPONENT headers)

if(MSVC AND NOT EMBEDDED_DEBUG_INFO)
  install(FILES $<TARGET_PDB_FILE:helicsSharedLib>
          DESTINATION ${CMAKE_INSTALL_BINDIR}
          OPTIONAL COMPONENT libs)
endif()
install(FILES ${helicsShared_headers}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library
        COMPONENT headers)
