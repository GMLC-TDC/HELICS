
if (ENABLE_TEST_TIMEOUTS)
add_definitions(-DENABLE_TEST_TIMEOUTS=1)
else(ENABLE_TEST_TIMEOUTS)
add_definitions(-DENABLE_TEST_TIMEOUTS=0)
endif(ENABLE_TEST_TIMEOUTS)

add_subdirectory (helics)