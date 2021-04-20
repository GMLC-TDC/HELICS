# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(MSVC)

    set(doc_sources
        apps/Echo.md
        apps/Broker.md
        apps/App.md
        apps/Player.md
        apps/index.md
        apps/Source.md
        apps/Clone.md
        apps/BrokerServer.md
        configuration/CoreTypes.md
        configuration/Federate.md
        configuration/index.md
        configuration/Timing.md
        configuration/Filters.md
        configuration/MessageFederate.md
        configuration/ValueFederate.md
    )

    add_custom_target(helics_docs SOURCES ${doc_sources})

    file(GLOB user_guide_sources user-guide/*.md)

    add_custom_target(helics_user_guide SOURCES ${user_guide_sources})

    set(helics_main_docs
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/CHANGELOG.md
        ${PROJECT_SOURCE_DIR}/CONTRIBUTORS.md
        ${PROJECT_SOURCE_DIR}/CONTRIBUTING.md
        ${PROJECT_SOURCE_DIR}/docs/Public_API.md
        ${PROJECT_SOURCE_DIR}/docs/docker.md
        ${PROJECT_SOURCE_DIR}/docs/ROADMAP.md
    )

    add_custom_target(helics_package_docs SOURCES ${helics_main_docs})

    set_target_properties(helics_user_guide helics_docs helics_package_docs PROPERTIES FOLDER docs)

endif(MSVC)
