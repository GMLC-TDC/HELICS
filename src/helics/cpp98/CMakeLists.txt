##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
set(helicsCpp98_headers
Broker.hpp
Core.hpp
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

