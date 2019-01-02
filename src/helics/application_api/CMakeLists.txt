#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#
set(
  public_application_api_headers
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
  FederateInfo.hpp
  Inputs.hpp
)

set (conv_headers
   ../application_api.hpp
   ../ValueFederates.hpp
   ../MessageFederates.hpp
   )
   
set(
  private_application_api_headers
  MessageFederateManager.hpp
  ValueFederateManager.hpp
  AsyncFedCallInfo.hpp
  FilterOperations.hpp
  FilterFederateManager.hpp
)

set(
  application_api_sources
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
  FilterFederateManager.cpp
  Endpoints.cpp
  helicsTypes.cpp
  queryFunctions.cpp
  FederateInfo.cpp
  Inputs.cpp
)

add_library(
  application_api
  OBJECT
  ${application_api_sources}
  ${conv_headers}
  ${public_application_api_headers}
  ${private_application_api_headers}
)

target_include_directories(
  application_api
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_INCLUDE_DIRECTORIES>
)
target_include_directories(
  application_api SYSTEM
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>
)
target_compile_definitions(
  application_api PRIVATE
  $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_DEFINITIONS>
)
target_compile_options(
  application_api
  PRIVATE $<TARGET_PROPERTY:helics_base_includes,INTERFACE_COMPILE_OPTIONS>
)

if(MSYS AND USE_LIBCXX)
  target_compile_definitions(application_api PUBLIC "-DSTX_NO_STD_OPTIONAL=1")
endif()

install(
  FILES ${public_application_api_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/application_api
  COMPONENT headers
)

install(
  FILES ${conv_headers}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
  COMPONENT headers
)
