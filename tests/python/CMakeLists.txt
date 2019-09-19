

if(NOT ${CMAKE_VERSION} VERSION_LESS "3.12.0")
	if (PYTHON_ROOT_DIR)
		set(Python3_ROOT_DIR ${PYTHON_ROOT_DIR})
	endif()
    # Use the Python_add_library function added in 3.12.0
    find_package(Python3 COMPONENTS Interpreter)
	set(HELICS_PYTHON_EXE ${Python3_EXECUTABLE})
  else()
    set(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
    find_package(PythonInterp 3 REQUIRED)
	set(HELICS_PYTHON_EXE ${PYTHON_EXECUTABLE})
  endif()

message(STATUS "Python ${HELICS_PYTHON_EXE}")

add_test(
  NAME python_basic_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${HELICS_PYTHON_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_helics.py
)

add_test(
  NAME python_message_federate_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${HELICS_PYTHON_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_federate.py
)

add_test(
  NAME python_message_filter_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${HELICS_PYTHON_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_filter.py
)

add_test(
  NAME python_value_federate_test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/interfaces/python
  COMMAND
    ${HELICS_PYTHON_EXE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_value_federate.py
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
