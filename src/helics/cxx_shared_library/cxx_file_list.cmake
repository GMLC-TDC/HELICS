# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# this file just contains the lists of header files to use for the cxx shared library
# and app library for public headers
set(helics_shared_public_headers
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/CombinationFederate.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Publications.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Subscriptions.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Endpoints.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Filters.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Federate.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/helicsTypes.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/data_view.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/MessageFederate.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/MessageOperators.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/ValueConverter.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/ValueConverter_impl.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/ValueFederate.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/HelicsPrimaryTypes.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/queryFunctions.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/FederateInfo.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/Inputs.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/BrokerApp.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/CoreApp.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/timeOperations.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/typeOperations.hpp
)

set(helics_shared_private_headers
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/MessageFederateManager.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/ValueFederateManager.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/AsyncFedCallInfo.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/FilterOperations.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api/FilterFederateManager.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/cxx_shared_library/BrokerFactory.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/cxx_shared_library/CoreFactory.hpp
)

set(conv_headers
    ${HELICS_LIBRARY_SOURCE_DIR}/application_api.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/ValueFederates.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/MessageFederates.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/helics.hpp
)
set(basic_headers ${HELICS_LIBRARY_SOURCE_DIR}/helics_enums.h)
set(basic_app_headers ${HELICS_LIBRARY_SOURCE_DIR}/helics_apps.hpp)

set(core_include_files
    ${HELICS_LIBRARY_SOURCE_DIR}/core/helics-time.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/core-data.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/core-types.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/federate_id.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/CoreFederateInfo.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/Core.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/Broker.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/helics_definitions.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/helicsCLI11.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/helicsVersion.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/core/core-exceptions.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/cxx_shared_library/BrokerFactory.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/cxx_shared_library/CoreFactory.hpp
)

set(utilities_include_files
    ${HELICS_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/timeRepresentation.hpp
)

set(helics_apps_public_headers
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Player.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Recorder.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Echo.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Source.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Tracer.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/helicsApp.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/Clone.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/CoreApp.hpp
    ${HELICS_LIBRARY_SOURCE_DIR}/apps/BrokerApp.hpp
)
