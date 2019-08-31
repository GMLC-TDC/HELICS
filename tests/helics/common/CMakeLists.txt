#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

# -----------------------------------------------------------------------------
# Common library tests using Boost
# -----------------------------------------------------------------------------

set(common_test_headers)

set(
  common_test_sources
  common-tests.cpp
  TimeTests.cpp
  CircularBufferTests.cpp
  StackQueueTests.cpp
)

add_executable(common-tests ${common_test_sources} ${common_test_headers})
target_link_libraries(common-tests PRIVATE helics-static helics_google_test_base)
set_target_properties(common-tests PROPERTIES FOLDER tests)

add_test(
  NAME common-tests
  COMMAND common-tests --log_level=message --report_level=short
)
set_property(
  TEST common-tests
  PROPERTY
    LABELS
    Common
    Daily
    Coverage
    Continuous
)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    common-tests
    POST_BUILD # Adds a post-build event to core tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:common-tests>/"
  ) # <--this is out-file path
endforeach(keyfile)
