##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(application_api_headers
application_api.h
CombinationFederate.h
coreInstantiation.h
Publications.hpp
Subscriptions.hpp
Endpoints.hpp
Filters.hpp
Federate.h
identifierTypes.hpp
Message.h
MessageFederate.h
MessageFederateManager.h
MessageFilterFederate.h
MessageFilterFederateManager.h
MessageOperators.h
ValueConverter.hpp
ValueFederate.h
ValueFederateManager.h
asyncFedCallInfo.h
)

set(application_api_sources
CombinationFederate.cpp
coreInstantiation.cpp
Federate.cpp
Message.cpp
MessageFederate.cpp
MessageFederateManager.cpp
MessageFilterFederate.cpp
MessageFilterFederateManager.cpp
MessageOperators.cpp
ValueFederate.cpp
ValueFederateManager.cpp
)

list(APPEND application_api_sources ${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp.cpp)

add_library(application_api OBJECT ${application_api_sources} ${application_api_headers})

link_directories(${external_link_directories})

