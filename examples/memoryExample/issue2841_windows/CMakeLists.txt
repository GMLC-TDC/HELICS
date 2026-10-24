# Windows-portable copy of the HELICS issue 2841 memory reproducer.
add_executable(helicsIssue2841Publisher publisher.cpp)
target_link_libraries(helicsIssue2841Publisher PRIVATE helics_application_api)

add_executable(helicsIssue2841Subscriber subscriber.cpp)
target_link_libraries(helicsIssue2841Subscriber PRIVATE helics_application_api)

if(WIN32)
    target_link_libraries(helicsIssue2841Publisher PRIVATE Psapi)
    target_link_libraries(helicsIssue2841Subscriber PRIVATE Psapi)
endif()

set_target_properties(
    helicsIssue2841Publisher helicsIssue2841Subscriber
    PROPERTIES FOLDER "examples/memoryExample/issue2841_windows"
)

add_custom_command(
    TARGET helicsIssue2841Publisher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/pub_fed.json"
            "$<TARGET_FILE_DIR:helicsIssue2841Publisher>/pub_fed.json"
)
set_property(
    TARGET helicsIssue2841Publisher APPEND PROPERTY LINK_DEPENDS
                                                    "${CMAKE_CURRENT_SOURCE_DIR}/pub_fed.json"
)
add_custom_command(
    TARGET helicsIssue2841Subscriber POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/sub_fed.json"
            "$<TARGET_FILE_DIR:helicsIssue2841Subscriber>/sub_fed.json"
)
set_property(
    TARGET helicsIssue2841Subscriber APPEND PROPERTY LINK_DEPENDS
                                                     "${CMAKE_CURRENT_SOURCE_DIR}/sub_fed.json"
)
