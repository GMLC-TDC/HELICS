#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

if(BUILD_APP_LIBRARY)

  set(
    helics_apps_public_headers
    Player.hpp
    Recorder.hpp
    Echo.hpp
    Source.hpp
    Tracer.hpp
    helicsApp.hpp
    BrokerApp.hpp
	MultiBroker.hpp
	Clone.hpp
	BrokerServer.hpp
  )

  set(helics_apps_private_headers PrecHelper.hpp SignalGenerators.hpp)

  set(
    helics_apps_library_files
    Player.cpp
    Recorder.cpp
    PrecHelper.cpp
    SignalGenerators.cpp
    Echo.cpp
    Source.cpp
    Tracer.cpp
    helicsApp.cpp
    BrokerApp.cpp
	MultiBroker.cpp
	Clone.cpp
	BrokerServer.cpp
  )

  add_library(
    helics_apps
    STATIC
	../helics_apps.hpp
    ${helics_apps_library_files}
    ${helics_apps_public_headers}
    ${helics_apps_private_headers}
  )

  target_link_libraries(helics_apps PUBLIC helics-static)
  target_link_libraries(helics_apps PRIVATE compile_flags_target)
  target_include_directories(
    helics_apps SYSTEM
    PRIVATE $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_INCLUDE_DIRECTORIES>
  )
  target_compile_definitions(
    helics_apps PRIVATE
    $<TARGET_PROPERTY:fmt::fmt-header-only,INTERFACE_COMPILE_DEFINITIONS>
  )
  # add and alias library to match the find_package
  add_library(HELICS::helics_apps ALIAS helics_apps)

  if (BUILD_APP_EXECUTABLES)
    add_executable(helics_player playerMain.cpp)
    target_link_libraries(helics_player PUBLIC helics_apps)
	target_link_libraries(helics_player PRIVATE compile_flags_target)
    set_target_properties(helics_player PROPERTIES FOLDER apps)
    install(
      TARGETS helics_player
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT applications
    )

    add_executable(helics_recorder recorderMain.cpp)
    target_link_libraries(helics_recorder PUBLIC helics_apps)
    set_target_properties(helics_recorder PROPERTIES FOLDER apps)
	target_link_libraries(helics_recorder PRIVATE compile_flags_target)
    install(
      TARGETS helics_recorder
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT applications
    )

    add_executable(helics_broker helics-broker.cpp)
    target_link_libraries(helics_broker PUBLIC helics_apps)
	target_link_libraries(helics_broker PRIVATE compile_flags_target)
    set_target_properties(helics_broker PROPERTIES FOLDER apps)
    set(
      HELICS_BROKER_LOC ${CMAKE_CURRENT_BINARY_DIR}
      CACHE INTERNAL "build folder location of the broker"
    )
    
	add_test(NAME broker_version_return COMMAND helics_broker --version)
	add_test(NAME broker_help_return COMMAND helics_broker --help)
	set_property(TEST broker_version_return broker_help_return PROPERTY LABELS Continuous)


	add_executable(helics_broker_server helics-broker-server.cpp)
    target_link_libraries(helics_broker_server PUBLIC helics_apps)
	target_link_libraries(helics_broker_server PRIVATE compile_flags_target)
    set_target_properties(helics_broker_server PROPERTIES FOLDER apps)
    set(
      HELICS_BROKER_SERVER_LOC ${CMAKE_CURRENT_BINARY_DIR}
      CACHE INTERNAL "build folder location of the broker server"
    )
    install(
      TARGETS helics_broker helics_broker_server
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT applications
    )
	
    add_executable(helics_app appMain.cpp)
    target_link_libraries(helics_app PUBLIC helics_apps)
	target_link_libraries(helics_app PRIVATE compile_flags_target)
    set_target_properties(helics_app PROPERTIES FOLDER apps)
    install(TARGETS helics_app DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)

	copy_key_files_to_target_location(helics_app)

	add_test(NAME app_version_return COMMAND helics_app --version)
	add_test(NAME app_help_return COMMAND helics_app --help)
    set_property(TEST app_version_return app_help_return PROPERTY LABELS Continuous)

	add_test(NAME app_subcommand_version_return COMMAND helics_app player --version)
	add_test(NAME app_subcommand_help_return COMMAND helics_app player --help)
	set_property(TEST app_subcommand_version_return app_subcommand_help_return PROPERTY LABELS Continuous)
  endif(BUILD_APP_EXECUTABLES)

  if (NOT DISABLE_STATIC_LIB_INSTALL)
  install(
    TARGETS helics_apps
    ${HELICS_EXPORT_COMMAND}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libs
  )
  endif()

  if (HELICS_INSTALL_CPP14_HEADERS)
  install(
    FILES ${helics_apps_public_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/apps
    COMPONENT headers
  )
  install(
    FILES ../helics_apps.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
    COMPONENT headers
  )
  endif(HELICS_INSTALL_CPP14_HEADERS)
endif (BUILD_APP_LIBRARY)

# configure executable app

if (NOT CYGWIN)
if (BUILD_APP_EXECUTABLES)
configure_file(helicsConfigMain.h.in ${CMAKE_CURRENT_BINARY_DIR}/helicsConfigMain.h)
add_executable(
  helics-config helicsConfigMain.cpp ${CMAKE_CURRENT_BINARY_DIR}/helicsConfigMain.h
)

target_link_libraries(helics-config PRIVATE compile_flags_target)
target_include_directories(helics-config PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(helics-config PROPERTIES FOLDER apps)

target_include_directories(helics-config SYSTEM PRIVATE ${HELICS_SOURCE_DIR}/ThirdParty)

install(
  TARGETS helics-config
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libs
)

endif(BUILD_APP_EXECUTABLES)

endif(NOT CYGWIN)
