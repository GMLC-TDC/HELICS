#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. 
# 
SPDX-License-Identifier: BSD-3-Clause
#

include_directories("${PROJECT_SOURCE_DIR}/src/helics")
include_directories("${PROJECT_SOURCE_DIR}/src/helics/cpp98")

add_executable(nonlings_fed1_cpp nonlings_fed1.cpp)
add_executable(nonlings_fed2_cpp nonlings_fed2.cpp)

target_link_libraries(nonlings_fed1_cpp helicsCpp98)
target_link_libraries(nonlings_fed2_cpp helicsCpp98)

set_property(TARGET nonlings_fed1_cpp PROPERTY CXX_STANDARD 98)
set_property(TARGET nonlings_fed2_cpp PROPERTY CXX_STANDARD 98)

if(!MSVC)
  target_compile_options(nonlings_fed1_cpp PRIVATE -pedantic -Werror)
  target_compile_options(nonlings_fed2_cpp PRIVATE -pedantic -Werror)
endif()

set_target_properties(nonlings_fed1_cpp nonlings_fed2_cpp PROPERTIES FOLDER examples)

if(BUILD_HELICS_TESTS AND NOT MSVC)
  add_test(
    NAME nonlings_example
    COMMAND
      ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 $<TARGET_FILE:nonlings_fed1_cpp>
      $<TARGET_FILE:nonlings_fed2_cpp>
  )
  set_property(TEST nonlings_example PROPERTY LABELS Continuous)
endif()

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    nonlings_fed1_cpp
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:nonlings_fed1_cpp>/"
  )
  # <--this is out-file path
endforeach(keyfile)

add_custom_command(
  TARGET
  nonlings_fed1_cpp
  POST_BUILD # Adds a post-build event to api tests
  COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_if_different # which executes "cmake - E copy_if_different..."
    "$<TARGET_FILE:helicsSharedLib>" # <--this is in-file
    "$<TARGET_FILE_DIR:nonlings_fed1_cpp>/"
) # <--this is out- file path
