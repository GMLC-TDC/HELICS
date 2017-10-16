add_executable(ipcTest1 ipcTest1.cpp common.hpp)
add_executable(ipcTest2 ipcTest2.cpp common.hpp)
link_directories(${external_link_directories})

include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)

target_link_libraries(ipcTest1 helics ${external_library_list})
target_link_libraries(ipcTest2 helics ${external_library_list})
