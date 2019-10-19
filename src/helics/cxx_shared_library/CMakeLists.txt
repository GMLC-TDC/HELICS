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

set(
    helics_shared_sources
    ../application_api/CombinationFederate.cpp
    ../application_api/Federate.cpp
    ../application_api/MessageFederate.cpp
    ../application_api/MessageFederateManager.cpp
    ../application_api/MessageOperators.cpp
    ../application_api/ValueFederate.cpp
    ../application_api/ValueConverter.cpp
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
)

   
   add_library(helics-shared SHARED ${helics_shared_sources} ${conv_headers}
                                   ${helics_shared_public_headers}
                                   ${helics_shared_private_headers})

target_link_libraries(helics-shared PRIVATE helics_core)
target_compile_definitions(helics-shared PRIVATE helics_shared_EXPORTS)

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


if(NOT HELICS_BINARY_ONLY_INSTALL)
    # Install the header if the C++ shared library is built
    if(HELICS_INSTALL_CPP14_HEADERS)
        install(
            FILES helics.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
            COMPONENT headers
        )
    endif()

endif()


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

