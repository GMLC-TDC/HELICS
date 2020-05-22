# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

macro(install_key_files_with_comp comp)
    install(
        FILES $<TARGET_FILE:helicsSharedLib>
        DESTINATION ${comp}
        COMPONENT ${comp}
    )
    install(
        FILES ${KEY_LIBRARY_FILES}
        DESTINATION ${comp}
        COMPONENT ${comp}
    )

    if(WIN32)
        if(TARGET libzmq)
            install(
                FILES $<TARGET_FILE:libzmq>
                DESTINATION ${comp}
                COMPONENT ${comp}
            )
        endif()

    endif(WIN32)
endmacro()
