if (BUILD_PYTHON_INTERFACE)
	SET(Python_ADDITIONAL_VERSIONS 3.4 3.5 3.6 3.7 3.8)
	find_package(PythonInterp 3)
	find_package(PythonLibs 3 REQUIRED )
endif(BUILD_PYTHON_INTERFACE)

message(STATUS "Python ${PYTHON_EXECUTABLE}")

add_test(NAME python_basic_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig/python COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_helics.py)

add_test(NAME python_message_federate_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig/python COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_federate.py)

add_test(NAME python_message_filter_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig/python COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_message_filter.py)

add_test(NAME python_value_federate_test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/swig/python COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/test_value_federate.py)

set_tests_properties(python_basic_test python_message_federate_test python_message_filter_test python_value_federate_test PROPERTIES LABELS Python)
