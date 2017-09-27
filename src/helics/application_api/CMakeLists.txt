##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(application_api_headers
application_api.h
CombinationFederate.h
Publications.hpp
Subscriptions.hpp
Endpoints.hpp
Filters.hpp
Federate.h
helicsTypes.hpp
Message.h
MessageFederate.h
MessageFederateManager.h
MessageFilterFederate.h
MessageFilterFederateManager.h
MessageOperators.h
ValueConverter.hpp
ValueConverter_impl.hpp
ValueFederate.h
ValueFederateManager.h
asyncFedCallInfo.h
HelicsPrimaryTypes.h
)

set(application_api_sources
CombinationFederate.cpp
Federate.cpp
MessageFederate.cpp
MessageFederateManager.cpp
MessageFilterFederate.cpp
MessageFilterFederateManager.cpp
MessageOperators.cpp
ValueFederate.cpp
ValueConverter.cpp
ValueFederateManager.cpp
Subscriptions.cpp
Publications.cpp
)

list(APPEND application_api_sources ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp.cpp)

add_library(application_api OBJECT ${application_api_sources} ${application_api_headers})

include_directories(${Boost_INCLUDE_DIR})

link_directories(${external_link_directories})

