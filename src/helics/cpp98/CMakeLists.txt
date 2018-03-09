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

add_library(helicsCpp98 INTERFACE)
target_include_directories(helicsCpp98 INTERFACE "${PROJECT_SOURCE_DIR}/src/helics")
target_link_libraries(helicsCpp98 INTERFACE helicsSharedLib)

IF (MSVC)
add_library(helicsCpp98_ide STATIC ${helicsCpp98_headers} ../../empty.cpp)
target_include_directories(helicsCpp98_ide PRIVATE "${PROJECT_SOURCE_DIR}/src/helics")
ENDIF(MSVC)

install(FILES ${helicsCpp98_headers} DESTINATION include/cpp98)

