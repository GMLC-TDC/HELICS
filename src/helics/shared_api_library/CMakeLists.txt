# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

include(GenerateExportHeader)

add_library(helicsSharedLib SHARED ${helicsShared_sources} ${helicsShared_headers})

add_library(HELICS::helicsSharedLib ALIAS helicsSharedLib)
generate_export_header(helicsSharedLib BASE_NAME helics)

target_include_directories(
    helicsSharedLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
              $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(
    helicsSharedLib PROPERTIES VERSION ${HELICS_VERSION} SOVERSION ${HELICS_VERSION_MAJOR}
)

set_target_properties(
    helicsSharedLib PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden
)
target_link_libraries(helicsSharedLib PRIVATE helics_application_api)
target_link_libraries(helicsSharedLib PRIVATE compile_flags_target)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
    target_compile_options(helicsSharedLib PUBLIC -Wno-documentation)
endif()

if(UNIX OR MINGW)
    if(NOT APPLE)
        target_link_libraries(
            helicsSharedLib PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.txt
        )
    endif(NOT APPLE)
endif()

set_target_properties(helicsSharedLib PROPERTIES FOLDER interfaces)

install(
    TARGETS helicsSharedLib ${HELICS_EXPORT_COMMAND}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(CMAKE_VERSION VERSION_GREATER 3.6)
    set(HELICS_EXCLUSION_COMMAND EXCLUDE_FROM_ALL)
else()
    set(HELICS_EXCLUSION_COMMAND)
endif()

if(WIN32)
    install(
        FILES $<TARGET_LINKER_FILE:helicsSharedLib>
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libs
        ${HELICS_EXCLUSION_COMMAND}
    )
    install(
        FILES $<TARGET_FILE:helicsSharedLib>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
        ${HELICS_EXCLUSION_COMMAND}
    )
else()
    install(
        TARGETS helicsSharedLib
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT RUNTIME
        ${HELICS_EXCLUSION_COMMAND}
    )
endif()

if(MSVC AND NOT HELICS_EMBEDDED_DEBUG_INFO)
    install(
        FILES $<TARGET_PDB_FILE:helicsSharedLib>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        OPTIONAL
        COMPONENT libs
    )
endif()

if(NOT HELICS_BINARY_ONLY_INSTALL)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics_export.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library COMPONENT headers
    )

    install(FILES ${helicsShared_headers}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/shared_api_library COMPONENT headers
    )

    install(FILES ../chelics.h ../helics98.hpp ../helics_enums.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics COMPONENT headers
    )
endif()
