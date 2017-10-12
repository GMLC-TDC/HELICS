add_executable(valueFed valueFed.cpp)
link_directories(${external_link_directories})

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)

target_link_libraries(valueFed helics ${external_library_list})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh
               ${CMAKE_CURRENT_BINARY_DIR}/run.sh
               COPYONLY)
