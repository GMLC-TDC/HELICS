# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(application_api_headers
    CombinationFederate.hpp
    CallbackFederate.hpp
    Publications.hpp
    Subscriptions.hpp
    Endpoints.hpp
    Filters.hpp
    Translator.hpp
    Federate.hpp
    helicsTypes.hpp
    data_view.hpp
    MessageFederate.hpp
    MessageOperators.hpp
    ValueConverter.hpp
    ValueFederate.hpp
    HelicsPrimaryTypes.hpp
    queryFunctions.hpp
    FederateInfo.hpp
    Inputs.hpp
    BrokerApp.hpp
    CoreApp.hpp
)

set(conv_headers ../application_api.hpp ../ValueFederates.hpp ../MessageFederates.hpp)

set(private_application_api_headers
    MessageFederateManager.hpp
    ValueFederateManager.hpp
    AsyncFedCallInfo.hpp
    FilterOperations.hpp
    ConnectorFederateManager.hpp
    TranslatorOperations.hpp
    PotentialInterfacesManager.hpp
)

set(application_api_sources
    CombinationFederate.cpp
    Federate.cpp
    MessageFederate.cpp
    MessageFederateManager.cpp
    MessageOperators.cpp
    ValueFederate.cpp
    ValueConverter.cpp
    ValueFederateManager.cpp
    helicsPrimaryTypes.cpp
    Publications.cpp
    Filters.cpp
    FilterOperations.cpp
    ConnectorFederateManager.cpp
    PotentialInterfacesManager.cpp
    Translator.cpp
    TranslatorOperations.cpp
    Endpoints.cpp
    helicsTypes.cpp
    queryFunctions.cpp
    FederateInfo.cpp
    CallbackFederate.cpp
    Inputs.cpp
    BrokerApp.cpp
    CoreApp.cpp
)

add_library(
    helics_application_api STATIC ${application_api_sources} ${conv_headers}
                                  ${application_api_headers} ${private_application_api_headers}
)

target_link_libraries(helics_application_api PUBLIC HELICS::network)
target_link_libraries(helics_application_api PRIVATE fmt::fmt)
target_compile_definitions(helics_application_api PUBLIC HELICS_CXX_STATIC_DEFINE)
if(MSYS AND USE_LIBCXX)
    target_compile_definitions(helics_application_api PUBLIC "-DSTX_NO_STD_OPTIONAL=1")
endif()

add_library(HELICS::application_api ALIAS helics_application_api)
