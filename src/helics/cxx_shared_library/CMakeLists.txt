# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(
    helics_shared_public_headers
    ../application_api/CombinationFederate.hpp
    ../application_api/Publications.hpp
    ../application_api/Subscriptions.hpp
    ../application_api/Endpoints.hpp
    ../application_api/Filters.hpp
    ../application_api/Federate.hpp
    ../application_api/helicsTypes.hpp
    ../application_api/data_view.hpp
    ../application_api/MessageFederate.hpp
    ../application_api/MessageOperators.hpp
    ../application_api/ValueConverter.hpp
    ../application_api/ValueConverter_impl.hpp
    ../application_api/ValueFederate.hpp
    ../application_api/HelicsPrimaryTypes.hpp
    ../application_api/queryFunctions.hpp
    ../application_api/FederateInfo.hpp
    ../application_api/Inputs.hpp
	../application_api/BrokerApp.hpp
	../application_api/CoreApp.hpp
	../application_api/timeOperations.hpp
	../application_api/typeOperations.hpp
)

set(
    helics_shared_private_headers
    ../application_api/MessageFederateManager.hpp
    ../application_api/ValueFederateManager.hpp
    ../application_api/AsyncFedCallInfo.hpp
    ../application_api/FilterOperations.hpp
    ../application_api/FilterFederateManager.hpp
)

set(conv_headers ../application_api.hpp ../ValueFederates.hpp ../MessageFederates.hpp)
set(basic_headers ../helics_enums.h)

set(
    helics_shared_sources
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
    ../application_api/FilterOperations.cpp
    ../application_api/FilterFederateManager.cpp
    ../application_api/Endpoints.cpp
    ../application_api/helicsTypes.cpp
    ../application_api/queryFunctions.cpp
    ../application_api/FederateInfo.cpp
    ../application_api/Inputs.cpp
	../application_api/BrokerApp.cpp
	../application_api/CoreApp.cpp
	../application_api/timeOperations.cpp
	../application_api/typeOperations.cpp
)

   
   add_library(helics-shared SHARED ${helics_shared_sources} ${conv_headers}
                                   ${helics_shared_public_headers}
                                   ${helics_shared_private_headers})

target_link_libraries(helics-shared PRIVATE helics_core)
target_compile_definitions(helics-shared PRIVATE helics_shared_EXPORTS)
target_compile_definitions(helics-shared PUBLIC HELICS_SHARED_LIBRARY)

if(MSYS AND USE_LIBCXX)
    target_compile_definitions(helics-shared PUBLIC "-DSTX_NO_STD_OPTIONAL=1")
endif()

    add_library(HELICS::helics-shared ALIAS helics-shared)

    set_target_properties(
        helics-shared
        PROPERTIES VERSION ${HELICS_VERSION} SOVERSION ${HELICS_VERSION_MAJOR}
    )

	set_target_properties(
   helics-shared
    PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
)

file(
     COPY ${helics_shared_public_headers}
     DESTINATION ${PROJECT_BINARY_DIR}/include/helics/application_api
)

file(
     COPY ${conv_headers} ${basic_headers}
     DESTINATION ${PROJECT_BINARY_DIR}/include/helics
)

set(core_include_files 
	../core/helics-time.hpp
	../core/core-data.hpp
	../core/core-types.hpp
	../core/federate_id.hpp
	../core/CoreFederateInfo.hpp
	../core/Core.hpp
	../core/helics_definitions.hpp 
	../core/helicsCLI11.hpp
	../core/helicsVersion.hpp
	)

file(
     COPY ${core_include_files}
     DESTINATION ${PROJECT_BINARY_DIR}/include/helics/core
)

set(utilities_include_files 
	${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/timeRepresentation.hpp)

file(
     COPY ${utilities_include_files}
     DESTINATION ${PROJECT_BINARY_DIR}/include/helics/utilities
)
		
file(
     COPY ${PROJECT_SOURCE_DIR}/ThirdParty/helics/external
     DESTINATION ${PROJECT_BINARY_DIR}/include/helics
)

install(
                DIRECTORY ${PROJECT_SOURCE_DIR}/ThirdParty/cereal
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional
                COMPONENT cereal
            )
            
target_include_directories(helics-shared 
		INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
		 $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
		 $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional>
		 PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		)



    install(
        TARGETS helics-shared ${HELICS_EXPORT_COMMAND}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    if(WIN32)
        install(
            FILES $<TARGET_LINKER_FILE:helics-shared>
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT libs
        )
        install(
            FILES $<TARGET_FILE:helics-shared>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        )
    else()
        install(
            TARGETS helics-shared DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT
            RUNTIME
        )
    endif()

	if(NOT HELICS_BINARY_ONLY_INSTALL)

    if(MSVC AND NOT EMBEDDED_DEBUG_INFO)
        install(
            FILES $<TARGET_PDB_FILE:helicsSharedLib>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL
            COMPONENT libs
        )
    endif()
    install(DIRECTORY
        ${PROJECT_BINARY_DIR}/include/helics
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT headers
    )

endif()

