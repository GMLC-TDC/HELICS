##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(public_application_api_headers
CombinationFederate.h
Publications.hpp
Subscriptions.hpp
Endpoints.hpp
Filters.hpp
Federate.h
helicsTypes.hpp
Message.h
MessageFederate.h
MessageOperators.h
ValueConverter.hpp
ValueConverter_impl.hpp
ValueFederate.h
HelicsPrimaryTypes.h
queryFunctions.h
)

set(private_application_api_headers
MessageFederateManager.h
ValueFederateManager.h
asyncFedCallInfo.h
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
helicsTypes.cpp
queryFunctions.cpp
)

add_library(application_api OBJECT ${application_api_sources} ${public_application_api_headers} ${private_application_api_headers})

install(FILES ${public_application_api_headers} DESTINATION include/helics/application_api)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

link_directories(${external_link_directories})

