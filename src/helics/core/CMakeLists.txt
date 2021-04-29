# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(SRC_FILES
    CoreFactory.cpp
    BrokerFactory.cpp
    BrokerBase.cpp
    CommonCore.cpp
    FederateState.cpp
    PublicationInfo.cpp
    InputInfo.cpp
    InterfaceInfo.cpp
    FilterInfo.cpp
    EndpointInfo.cpp
    ActionMessage.cpp
    CoreBroker.cpp
    TimeCoordinator.cpp
    ForwardingTimeCoordinator.cpp
    TimeDependencies.cpp
    HandleManager.cpp
    FilterCoordinator.cpp
    UnknownHandleManager.cpp
    federate_id.cpp
    TimeoutMonitor.cpp
    coreTypeOperations.cpp
    helicsCLI11JsonConfig.cpp
    FilterFederate.cpp
    TimeCoordinatorProcessing.cpp
)

set(PUBLIC_INCLUDE_FILES
    Core.hpp
    Broker.hpp
    CoreFactory.hpp
    BrokerFactory.hpp
    core-exceptions.hpp
    core-types.hpp
    core-data.hpp
    helics-time.hpp
    CoreFederateInfo.hpp
    helicsVersion.hpp
    federate_id.hpp
    helics_definitions.hpp
    helicsCLI11.hpp
)

set(INCLUDE_FILES
    coreTypeOperations.hpp
    BrokerBase.hpp
    TimeDependencies.hpp
    TimeCoordinator.hpp
    ForwardingTimeCoordinator.hpp
    loggingHelper.hpp
    global_federate_id.hpp
    basic_core_types.hpp
    TimeoutMonitor.h
    CoreBroker.hpp
    InterfaceInfo.hpp
    ActionMessageDefintions.hpp
    ActionMessage.hpp
    CommonCore.hpp
    FederateState.hpp
    PublicationInfo.hpp
    InputInfo.hpp
    EndpointInfo.hpp
    flagOperations.hpp
    BasicHandleInfo.hpp
    federate_id_extra.hpp
    FilterInfo.hpp
    FilterCoordinator.hpp
    HandleManager.hpp
    UnknownHandleManager.hpp
    queryHelpers.hpp
    fileConnections.hpp
    helicsCLI11JsonConfig.hpp
    FilterFederate.hpp
    TimeCoordinatorProcessing.hpp
    ../helics_enums.h
)

if(NOT HELICS_DISABLE_ASIO)
    list(APPEND SRC_FILES MessageTimer.cpp)
    list(APPEND INCLUDE_FILES MessageTimer.hpp)
endif()

add_library(helics_core STATIC ${SRC_FILES} ${INCLUDE_FILES} ${PUBLIC_INCLUDE_FILES})

target_link_libraries(
    helics_core PUBLIC helics_common PRIVATE spdlog::spdlog fmt::fmt compile_flags_target
)

add_library(HELICS::core ALIAS helics_core)

target_compile_options(
    helics_core
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-strict-aliasing>>
)

# adding a definition flag for some interaction with the shared library
target_compile_definitions(helics_core PUBLIC HELICS_STATIC_CORE_LIBRARY)
