

set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
if (NOT PYTHON_EXECUTABLE)
    find_package(PythonInterp 3 REQUIRED)
endif()

message(STATUS "Python ${PYTHON_EXECUTABLE}")

add_test(
  NAME python_basic_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_helics.py
)

add_test(
  NAME python_message_federate_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_federate.py
)

add_test(
  NAME python_message_filter_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_filter.py
)

add_test(
  NAME python_value_federate_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_value_federate.py
)

set_property(
  TEST
    python_basic_test
    python_message_federate_test
    python_message_filter_test
    python_value_federate_test
  PROPERTY
    LABELS
    Python
    Continuous
    Daily
)
