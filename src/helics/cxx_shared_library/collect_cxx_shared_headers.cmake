# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# this file copies the header files for the C++ shared library to a specific location

include(${CMAKE_CURRENT_LIST_DIR}/cxx_file_list.cmake)

file(COPY ${helics_shared_public_headers}
     DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics/application_api
)

file(COPY ${conv_headers} ${basic_headers}
     DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics
)

file(COPY ${core_include_files} DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics/core)

file(COPY ${utilities_include_files}
     DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics/utilities
)

file(COPY ${HELICS_SOURCE_DIR}/ThirdParty/helics/external
     DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics
)

file(COPY ${HELICS_BINARY_DIR}/helics_generated_includes/helics
     DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/
)

if(HELICS_BUILD_APP_LIBRARY)
    file(COPY ${helics_apps_public_headers}
         DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics/apps
    )
    file(COPY ${basic_app_headers}
         DESTINATION ${HELICS_CXX_HEADER_FILE_LOC}/helics
    )
endif()
