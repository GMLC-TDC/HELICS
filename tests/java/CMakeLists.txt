# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

set(helicsJar ${CMAKE_BINARY_DIR}/interfaces/java/buildjar/helics-${HELICS_VERSION}.jar)

add_custom_target(
    Java_hello_world
    ALL
    ${Java_JAVAC_EXECUTABLE}
    -classpath
    ${CMAKE_BINARY_DIR}/interfaces/java/buildjar/helics-${HELICS_VERSION}.jar
    -d
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.java
    DEPENDS
        JNIhelics
        ${CMAKE_BINARY_DIR}/interfaces/java/buildjar/helics-${HELICS_VERSION}.jar
    COMMENT "Building HelloWorld.java"
    VERBATIM
)

add_custom_target(
    Java_Helics_Api
    ALL
    ${Java_JAVAC_EXECUTABLE}
    -classpath
    ${CMAKE_BINARY_DIR}/interfaces/java/buildjar/helics-${HELICS_VERSION}.jar
    -d
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/JavaHelicsApiTests.java
    DEPENDS
        JNIhelics
        ${CMAKE_BINARY_DIR}/interfaces/java/buildjar/helics-${HELICS_VERSION}.jar
    COMMENT "Building JavaHelicsApiTests.java"
    VERBATIM
)

# Windows uses a different separator for Java classpaths than other platforms
if(CMAKE_HOST_WIN32 AND NOT CYGWIN AND CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_JAVA_INCLUDE_FLAG_SEP ";")
else()
    set(CMAKE_JAVA_INCLUDE_FLAG_SEP ":")
endif()
set(
    java_test_classpath
    "${helicsJar}${CMAKE_JAVA_INCLUDE_FLAG_SEP}${CMAKE_CURRENT_BINARY_DIR}"
)

add_test(
    NAME Java_hello_world_test
    COMMAND
        ${Java_JAVA_EXECUTABLE}
        -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics>
        -classpath
        "${java_test_classpath}"
        HelloWorld
)

add_test(
    NAME Java_Helics_Api_tests
    COMMAND
        ${Java_JAVA_EXECUTABLE}
        -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics>
        -classpath
        "${java_test_classpath}"
        JavaHelicsApiTests
)

set_target_properties(Java_hello_world Java_Helics_Api PROPERTIES FOLDER tests)

set_property(
    TEST Java_hello_world_test Java_Helics_Api_tests
    PROPERTY LABELS Java Continuous Daily
)

copy_key_files_to_target_located_at(Java_Helics_Api ${CMAKE_CURRENT_BINARY_DIR})
copy_shared_target_at(Java_Helics_Api ${CMAKE_CURRENT_BINARY_DIR} helicsSharedLib)
