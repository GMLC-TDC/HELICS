##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
set(public_application_api_headers
CombinationFederate.hpp
Publications.hpp
Subscriptions.hpp
Endpoints.hpp
Filters.hpp
Federate.hpp
helicsTypes.hpp
data_view.hpp
MessageFederate.hpp
MessageOperators.hpp
ValueConverter.hpp
ValueConverter_impl.hpp
ValueFederate.hpp
HelicsPrimaryTypes.hpp
queryFunctions.hpp
)

set(private_application_api_headers
MessageFederateManager.hpp
ValueFederateManager.hpp
AsyncFedCallInfo.hpp
FilterOperations.hpp
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
Subscriptions.cpp
Publications.cpp
Filters.cpp
FilterOperations.cpp
Endpoints.cpp
helicsTypes.cpp
queryFunctions.cpp
FederateInfo.cpp
)

add_library(application_api OBJECT ${application_api_sources} ${public_application_api_headers} ${private_application_api_headers})

install(FILES ${public_application_api_headers} DESTINATION include/helics/application_api)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

link_directories(${external_link_directories})

