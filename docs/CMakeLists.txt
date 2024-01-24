# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(MSVC)

    set(app_doc_sources
        references/apps/Echo.md
        references/apps/Broker.md
        references/apps/App.md
        references/apps/Player.md
        references/apps/index.md
        references/apps/Source.md
        references/apps/Clone.md
        references/apps/BrokerServer.md
        references/apps/cmdArgs.md
        references/apps/Recorder.md
        references/apps/Tracer.md
    )

    add_custom_target(helics_app_docs SOURCES ${app_doc_sources})

    file(GLOB user_guide_sources user-guide/*.md)
    add_custom_target(helics_user_guide SOURCES ${user_guide_sources})

    file(GLOB developer_guide_sources developer-guide/*.md)
    add_custom_target(helics_developer_guide SOURCES ${developer_guide_sources})

    set(helics_main_docs
        ${PROJECT_SOURCE_DIR}/README.md
        ${PROJECT_SOURCE_DIR}/CHANGELOG.md
        ${PROJECT_SOURCE_DIR}/CONTRIBUTORS.md
        ${PROJECT_SOURCE_DIR}/CONTRIBUTING.md
        ${PROJECT_SOURCE_DIR}/docs/Public_API.md
        ${PROJECT_SOURCE_DIR}/docs/ROADMAP.md
    )

    add_custom_target(helics_package_docs SOURCES ${helics_main_docs})

    set_target_properties(
        helics_user_guide helics_developer_guide helics_app_docs helics_package_docs
        PROPERTIES FOLDER docs
    )

endif(MSVC)
