# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(NOT TARGET gmlc::networking)
    set(GMLC_NETWORKING_INSTALL OFF CACHE INTERNAL "")
    set(GMLC_NETWORKING_ASIO_INCLUDE ${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include)
    set(GMLC_NETWORKING_CONCURRENCY_INCLUDE ${PROJECT_SOURCE_DIR}/ThirdParty/concurrency)
    set(GMLC_NETWORKING_DISABLE_ASIO ${HELICS_DISABLE_ASIO} CACHE INTERNAL "")
    set(GMLC_NETWORKING_JSON_INCLUDE ${PROJECT_SOURCE_DIR}/ThirdParty)

    # hide some of the JSON in the networking library
    set(JSON_CI OFF CACHE INTERNAL "")
    set(JSON_Diagnostics OFF CACHE INTERNAL "")
    set(JSON_ImplicitConversions ON CACHE INTERNAL "")
    set(JSON_Install OFF CACHE INTERNAL "")
    set(JSON_MultipleHeaders OFF CACHE INTERNAL "")
    set(JSON_SystemInclude OFF CACHE INTERNAL "")

    if(HELICS_ENABLE_ENCRYPTION)
        list(APPEND VCPKG_MANIFEST_FEATURES "encryption")
        set(GMLC_NETWORKING_ENABLE_ENCRYPTION ON CACHE INTERNAL "")
    else()
        set(GMLC_NETWORKING_ENABLE_ENCRYPTION OFF CACHE INTERNAL "")
    endif()

    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
        add_subdirectory(ThirdParty/networking EXCLUDE_FROM_ALL SYSTEM)
    else()
        add_subdirectory(ThirdParty/networking EXCLUDE_FROM_ALL)
    endif()

    hide_variable(GMLC_NETWORKING_GENERATE_DOXYGEN_DOC)
    hide_variable(GMLC_NETWORKING_INCLUDE_BOOST)
    hide_variable(GMLC_NETWORKING_WITH_CMAKE_PACKAGE)
    hide_variable(GMLC_NETWORKING_OBJECT_LIB)
    hide_variable(GMLC_NETWORKING_STATIC_LIB)
    hide_variable(GMLC_NETWORKING_CLANG_TIDY_OPTIONS)
    hide_variable(GMLC_NETWORKING_ENABLE_SUBMODULE_UPDATE)
    hide_variable(GMLC_NETWORKING_USE_EXTERNAL_JSON)
    hide_variable(GMLC_NETWORKING_ENABLE_ENCRYPTION)
    hide_variable(GMLC_NETWORKING_DISABLE_ASIO)
    set_target_properties(gmlc_networking PROPERTIES FOLDER Extern)

endif()
