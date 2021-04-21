# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helicsCpp98_headers
    Broker.hpp
    Core.hpp
    CombinationFederate.hpp
    Federate.hpp
    MessageFederate.hpp
    helicsExceptions.hpp
    ValueFederate.hpp
    helics.hpp
    Publication.hpp
    Input.hpp
    Endpoint.hpp
    Filter.hpp
    config.hpp
)

add_library(helicsCpp98 INTERFACE)
target_link_libraries(helicsCpp98 INTERFACE helicsSharedLib)
add_library(HELICS::helicsCpp98 ALIAS helicsCpp98)
target_include_directories(
    helicsCpp98 INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/helics>
                          $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

# this is purely so the files show up nicely in an IDE, other ides might use it as well but that can
# be added when/if the need arises
if(MSVC)
    add_library(helicsCpp98_ide STATIC ${helicsCpp98_headers} ../../empty.cpp)
    target_include_directories(helicsCpp98_ide PRIVATE "${PROJECT_SOURCE_DIR}/src/helics")
    target_include_directories(
        helicsCpp98_ide PRIVATE "${PROJECT_BINARY_DIR}/src/helics/shared_api_library"
    )
    set_target_properties(helicsCpp98_ide PROPERTIES FOLDER interfaces)
endif(MSVC)

install(
    TARGETS helicsCpp98 ${HELICS_EXPORT_COMMAND}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${helicsCpp98_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/cpp98
        COMPONENT headers
)
