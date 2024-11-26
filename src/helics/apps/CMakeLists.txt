# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if(HELICS_BUILD_APP_LIBRARY)

    cmake_dependent_advanced_option(
        HELICS_DISABLE_WEBSERVER
        "Disable the embedded webserver app structure in the broker server" OFF
        "NOT HELICS_DISABLE_BOOST" ON
    )
    set(helics_apps_public_headers
        Player.hpp
        Recorder.hpp
        Echo.hpp
        Source.hpp
        Tracer.hpp
        helicsApp.hpp
        Clone.hpp
        CoreApp.hpp
        Probe.hpp
        Connector.hpp
        BrokerApp.hpp
    )

    set(helics_apps_broker_headers MultiBroker.hpp BrokerServer.hpp zmqBrokerServer.hpp
                                   AsioBrokerServer.hpp TypedBrokerServer.hpp
    )

    set(helics_apps_private_headers PrecHelper.hpp SignalGenerators.hpp)

    set(helics_apps_library_files
        Player.cpp
        Recorder.cpp
        PrecHelper.cpp
        SignalGenerators.cpp
        Echo.cpp
        Source.cpp
        Tracer.cpp
        helicsApp.cpp
        Clone.cpp
        Probe.cpp
        Connector.cpp
    )

    set(helics_apps_broker_files MultiBroker.cpp BrokerServer.cpp zmqBrokerServer.cpp
                                 TypedBrokerServer.cpp
    )

    if(NOT HELICS_DISABLE_ASIO)
        list(APPEND helics_apps_broker_files AsioBrokerServer.cpp)
    endif()

    if(NOT (HELICS_DISABLE_WEBSERVER OR HELICS_DISABLE_BOOST OR HELICS_DISABLE_ASIO))
        message(STATUS "Building webserver Boost version ${Boost_VERSION} ${BOOST_VERSION_LEVEL}")
        list(APPEND helics_apps_broker_files helicsWebServer.cpp RestApiConnection.cpp)
        list(APPEND helics_apps_broker_headers helicsWebServer.hpp indexPage.hpp
             RestApiConnection.hpp
        )
    endif()

    add_library(
        helics_apps STATIC
        ../helics_apps.hpp
        ${helics_apps_library_files}
        ${helics_apps_public_headers}
        ${helics_apps_broker_headers}
        ${helics_apps_private_headers}
        ${helics_apps_broker_files}
    )

    target_link_libraries(helics_apps PUBLIC HELICS::application_api)
    target_link_libraries(
        helics_apps
        PRIVATE HELICS::compile_flags_target
                spdlog::spdlog
                fmt::fmt
                helics::zmq
                HELICS::common
                gmlc::networking
    )

    if(NOT (HELICS_DISABLE_WEBSERVER OR HELICS_DISABLE_BOOST OR HELICS_DISABLE_ASIO))
        target_compile_definitions(helics_apps PUBLIC BOOST_DATE_TIME_NO_LIB)
        if(${Boost_VERSION_MINOR} LESS 81)
            target_compile_definitions(helics_apps PUBLIC BOOST_BEAST_USE_STD_STRING_VIEW)
        endif()
        target_compile_definitions(helics_apps PUBLIC HELICS_ENABLE_WEBSERVER=1)
        if(WIN32)
            target_link_libraries(helics_apps PRIVATE bcrypt)
        endif()

        if(MSVC)
            target_compile_options(helics_apps PRIVATE "/bigobj")
        endif()
    endif()
    # add and alias library to match the find_package
    add_library(HELICS::apps ALIAS helics_apps)

    if(HELICS_BUILD_APP_EXECUTABLES)
        add_executable(helics_player playerMain.cpp)
        target_link_libraries(helics_player PUBLIC HELICS::apps)
        target_link_libraries(helics_player PRIVATE compile_flags_target)
        set_target_properties(helics_player PROPERTIES FOLDER apps)
        install(TARGETS helics_player ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
                COMPONENT applications
        )

        add_executable(helics_recorder recorderMain.cpp)
        target_link_libraries(helics_recorder PUBLIC HELICS::apps)
        set_target_properties(helics_recorder PROPERTIES FOLDER apps)
        target_link_libraries(helics_recorder PRIVATE compile_flags_target)
        install(TARGETS helics_recorder ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
                COMPONENT applications
        )

        add_executable(helics_connector connectorMain.cpp)
        target_link_libraries(helics_connector PUBLIC HELICS::apps)
        target_link_libraries(helics_connector PRIVATE compile_flags_target)
        set_target_properties(helics_connector PROPERTIES FOLDER apps)
        install(TARGETS helics_connector ${HELICS_EXPORT_COMMAND}
                DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
        )

        add_executable(helics_broker helics-broker.cpp)
        target_link_libraries(helics_broker PUBLIC HELICS::apps)
        target_link_libraries(helics_broker PRIVATE compile_flags_target)
        set_target_properties(helics_broker PROPERTIES FOLDER apps)
        set(HELICS_BROKER_LOC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            CACHE INTERNAL "build folder location of the broker"
        )

        add_test(NAME broker_version_return COMMAND helics_broker --version)
        add_test(NAME broker_help_return COMMAND helics_broker --help)
        set_property(TEST broker_version_return broker_help_return PROPERTY LABELS Continuous)

        add_executable(helics_broker_server helics-broker-server.cpp)
        target_link_libraries(helics_broker_server PUBLIC HELICS::apps)
        target_link_libraries(helics_broker_server PRIVATE compile_flags_target)
        set_target_properties(helics_broker_server PROPERTIES FOLDER apps)
        set(HELICS_BROKER_SERVER_LOC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            CACHE INTERNAL "build folder location of the broker server"
        )
        install(TARGETS helics_broker helics_broker_server ${HELICS_EXPORT_COMMAND}
                DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
        )

        add_executable(helics_app appMain.cpp)
        target_link_libraries(helics_app PUBLIC HELICS::apps)
        target_link_libraries(helics_app PRIVATE spdlog::spdlog compile_flags_target)
        set_target_properties(helics_app PROPERTIES FOLDER apps)
        install(TARGETS helics_app ${HELICS_EXPORT_COMMAND} DESTINATION ${CMAKE_INSTALL_BINDIR}
                COMPONENT applications
        )

        add_test(NAME app_version_return COMMAND helics_app --version)
        add_test(NAME app_help_return COMMAND helics_app --help)
        set_property(TEST app_version_return app_help_return PROPERTY LABELS Continuous)

        add_test(NAME app_subcommand_version_return COMMAND helics_app player --version)
        add_test(NAME app_subcommand_help_return COMMAND helics_app player --help)
        set_property(
            TEST app_subcommand_version_return app_subcommand_help_return PROPERTY LABELS
                                                                                   Continuous
        )
    endif(HELICS_BUILD_APP_EXECUTABLES)

    if(HELICS_BUILD_CXX_SHARED_LIB)
        add_library(
            helicscpp-apps SHARED ../helics_apps.hpp ${helics_apps_library_files}
                                  ${helics_apps_public_headers} ${helics_apps_private_headers}
        )

        target_link_libraries(helicscpp-apps PUBLIC HELICS::helicscpp)
        target_link_libraries(
            helicscpp-apps PRIVATE HELICS::common spdlog::spdlog fmt::fmt gmlc::utilities
                                   gmlc::networking
        )

        # add and alias library to match the find_package
        add_library(HELICS::helicscpp-apps ALIAS helicscpp-apps)

        target_compile_definitions(helicscpp-apps PRIVATE helicscpp_EXPORTS)

        target_include_directories(
            helicscpp-apps
            INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/helics_cxx>
                      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
            PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        )

        target_include_directories(
            helicscpp-apps SYSTEM INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
        )

        set_target_properties(
            helicscpp-apps PROPERTIES VERSION ${HELICS_VERSION} SOVERSION ${HELICS_VERSION_MAJOR}
        )

        set_target_properties(
            helicscpp-apps PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden
                                      VISIBILITY_INLINES_HIDDEN ON
        )

        install(
            TARGETS helicscpp-apps ${HELICS_EXPORT_COMMAND}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

        if(WIN32)
            install(FILES $<TARGET_LINKER_FILE:helicscpp-apps> DESTINATION ${CMAKE_INSTALL_LIBDIR}
                    COMPONENT libs
            )
            install(FILES $<TARGET_FILE:helicscpp-apps> DESTINATION ${CMAKE_INSTALL_BINDIR}
                    COMPONENT Runtime
            )
        else()
            install(TARGETS helicscpp-apps DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT RUNTIME)
        endif()

        if(NOT HELICS_BINARY_ONLY_INSTALL)

            if(MSVC AND NOT HELICS_EMBEDDED_DEBUG_INFO)
                install(
                    FILES $<TARGET_PDB_FILE:helicscpp-apps>
                    DESTINATION ${CMAKE_INSTALL_BINDIR}
                    OPTIONAL
                    COMPONENT libs
                )
            endif()

        endif()
    endif()

endif(HELICS_BUILD_APP_LIBRARY)
