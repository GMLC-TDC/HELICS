add_executable(ipcTest1 ipcTest1.cpp common.hpp)
add_executable(ipcTest2 ipcTest2.cpp common.hpp)

include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

target_link_libraries(ipcTest1 helics-static)
target_link_libraries(ipcTest2 helics-static)


set_target_properties (ipcTest1 PROPERTIES
    FOLDER examples
)

set_target_properties (ipcTest2 PROPERTIES
    FOLDER examples
)