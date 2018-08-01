#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC All rights
# reserved. See LICENSE file and DISCLAIMER for more details.
#

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

# -------------------------------------------------------------
# Add the main HELICS library
# -------------------------------------------------------------

add_library(helics-static
            STATIC
            $<TARGET_OBJECTS:application_api>
            $<TARGET_OBJECTS:helics_core>
            $<TARGET_OBJECTS:helics_common>)

target_link_libraries(helics-static PUBLIC helics_base)
# add and alias library to match the find_package
add_library(HELICS::helics-static ALIAS helics-static)
if(BUILD_CXX_SHARED_LIB)
  add_library(helics-shared
              SHARED
              ../empty.cpp
              $<TARGET_OBJECTS:application_api>
              $<TARGET_OBJECTS:helics_core>
              $<TARGET_OBJECTS:helics_common>)

  add_library(HELICS::helics-shared ALIAS helics-shared)
  target_link_libraries(helics-shared PRIVATE helics_base)

  if(WIN32)
    set_target_properties(helics-shared
                          PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  endif()

endif()

install(FILES flag-definitions.h helics.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
        COMPONENT headers)

install(TARGETS helics-static
        EXPORT helics-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libs)

if(BUILD_CXX_SHARED_LIB)
  install(TARGETS helics-shared
          EXPORT helics-targets
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

  install(FILES $<TARGET_LINKER_FILE:helics-shared>
          DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT libs)
  install(FILES $<TARGET_FILE:helics-shared>
          DESTINATION ${CMAKE_INSTALL_BINDIR}
          COMPONENT libs)

endif(BUILD_CXX_SHARED_LIB)

add_subdirectory(apps)

if(BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
  add_subdirectory(shared_api_library)
  add_subdirectory(cpp98)
  install(FILES chelics.h
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
          COMPONENT headers)
endif()
