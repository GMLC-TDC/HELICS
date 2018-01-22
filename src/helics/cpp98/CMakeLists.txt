##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
set(helicsCpp98_headers
Broker.hpp
CombinationFederate.hpp
Federate.hpp
MessageFederate.hpp
ValueFederate.hpp
helics.hpp
)

add_library(helicsCpp98 STATIC ${helicsCpp98_headers})
set_target_properties(helicsCpp98 PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(helicsCpp98 PUBLIC "${PROJECT_SOURCE_DIR}/src/helics")
target_link_libraries(helicsCpp98 INTERFACE helicsSharedLib)

install(FILES ${helicsCpp98_headers} DESTINATION include/cpp98)
