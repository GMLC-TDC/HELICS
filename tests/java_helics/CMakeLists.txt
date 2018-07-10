find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

set(helicsJar ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar)
 
add_custom_target( Java_hello_world ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.java
    DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building HelloWorld.java"
    VERBATIM
)

add_custom_target( Java_Helics_Api ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/JavaHelicsApiTests.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
    COMMENT "Building JavaHelicsApiTests.java"
    VERBATIM
)

add_custom_target( Java_MessageFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building MessageFederate.java"
    VERBATIM
)

add_custom_target( Java_MessageFilter ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFilter.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building MessageFilter.java"
    VERBATIM
)

add_custom_target( Java_ValueFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/TestValueFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building ValueFederate.java"
    VERBATIM
)

# Windows uses a different separator for Java classpaths than other platforms
if (CMAKE_HOST_WIN32 AND NOT CYGWIN AND CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_JAVA_INCLUDE_FLAG_SEP ";")
else ()
    set(CMAKE_JAVA_INCLUDE_FLAG_SEP ":")
endif()
set(java_test_classpath "${helicsJar}${CMAKE_JAVA_INCLUDE_FLAG_SEP}${CMAKE_CURRENT_BINARY_DIR}")

add_test(NAME Java_hello_world_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics> -classpath "${java_test_classpath}" HelloWorld)

add_test(NAME Java_Helics_Api_tests COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics> -classpath "${java_test_classpath}" JavaHelicsApiTests)

add_test(NAME Java_MessageFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics> -classpath "${java_test_classpath}" TestMessageFederate)

add_test(NAME Java_MessageFilter_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics> -classpath "${java_test_classpath}" TestMessageFilter)

add_test(NAME Java_ValueFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=$<TARGET_FILE_DIR:JNIhelics> -classpath "${java_test_classpath}" TestValueFederate)

set_target_properties (Java_hello_world Java_Helics_Api Java_MessageFederate Java_MessageFilter Java_ValueFederate PROPERTIES FOLDER tests)

set_property(TEST Java_hello_world_test Java_Helics_Api_tests Java_MessageFederate_test Java_MessageFilter_test Java_ValueFederate_test PROPERTY LABELS Java Continuous Daily)

