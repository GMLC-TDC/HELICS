# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helics_shared_sources
    ../application_api/CombinationFederate.cpp
    ../application_api/Federate.cpp
    ../application_api/MessageFederate.cpp
    ../application_api/MessageFederateManager.cpp
    ../application_api/MessageOperators.cpp
    ../application_api/ValueFederate.cpp
    ../application_api/ValueFederateManager.cpp
    ../application_api/helicsPrimaryTypes.cpp
    ../application_api/Publications.cpp
    ../application_api/Filters.cpp
    ../application_api/Translator.cpp
    ../application_api/FilterOperations.cpp
    ../application_api/TranslatorOperations.cpp
    ../application_api/ConnectorFederateManager.cpp
    ../application_api/PotentialInterfacesManager.cpp
    ../application_api/Endpoints.cpp
    ../application_api/helicsTypes.cpp
    ../application_api/queryFunctions.cpp
    ../application_api/FederateInfo.cpp
    ../application_api/Inputs.cpp
    ../application_api/BrokerApp.cpp
    ../application_api/CoreApp.cpp
    ../application_api/timeOperations.cpp
    ../application_api/typeOperations.cpp
    ../application_api/ValueConverter.cpp
)

set(HELICS_LIBRARY_SOURCE_DIR ${HELICS_SOURCE_DIR}/src/helics)

include(${CMAKE_CURRENT_SOURCE_DIR}/cxx_file_list.cmake)

add_library(
    helicscpp SHARED ${helics_shared_sources} ${conv_headers} ${helics_shared_public_headers}
                     ${helics_shared_private_headers}
)

target_link_libraries(helicscpp PRIVATE HELICS::network fmt::fmt)
target_compile_definitions(helicscpp PRIVATE helicscpp_EXPORTS PUBLIC HELICS_SHARED_LIBRARY)

target_compile_options(helicscpp PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/wd4251 /wd4275>)

add_library(HELICS::helicscpp ALIAS helicscpp)

set_target_properties(
    helicscpp PROPERTIES VERSION ${HELICS_VERSION} SOVERSION ${HELICS_VERSION_MAJOR}
)

set_target_properties(
    helicscpp PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden
                         VISIBILITY_INLINES_HIDDEN ON
)

file(MAKE_DIRECTORY ${HELICS_BINARY_DIR}/include/helics_cxx)

add_custom_command(
    TARGET helicscpp
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -DHELICS_CXX_HEADER_FILE_LOC=${HELICS_BINARY_DIR}/include/helics_cxx
        -DHELICS_LIBRARY_SOURCE_DIR=${HELICS_LIBRARY_SOURCE_DIR}
        -DHELICS_SOURCE_DIR=${HELICS_SOURCE_DIR} -DHELICS_BINARY_DIR=${HELICS_BINARY_DIR}
        -DHELICS_BUILD_APP_LIBRARY=${HELICS_BUILD_APP_LIBRARY} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/collect_cxx_shared_headers.cmake
)

target_include_directories(
    helicscpp
    INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/helics_cxx>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
              $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

install(
    TARGETS helicscpp ${HELICS_EXPORT_COMMAND}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(WIN32)
    install(FILES $<TARGET_LINKER_FILE:helicscpp> DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT libs
    )
    install(FILES $<TARGET_FILE:helicscpp> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
else()
    install(TARGETS helicscpp DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT RUNTIME)
endif()

if(NOT HELICS_BINARY_ONLY_INSTALL)

    if(MSVC AND NOT HELICS_EMBEDDED_DEBUG_INFO)
        install(
            FILES $<TARGET_PDB_FILE:helicscpp>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL
            COMPONENT libs
        )
    endif()
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/helics_cxx/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT headers
    )

endif()
