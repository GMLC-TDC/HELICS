FIND_PACKAGE(Java REQUIRED)
FIND_PACKAGE(JNI REQUIRED)
INCLUDE(UseJava)


ADD_CUSTOM_TARGET( Java_hello_world
    COMMAND ${Java_JAVAC_EXECUTABLE}
	ARGS -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/swig/java/ ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.java
	COMMENT "Building HelloWorld.java"
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
    VERBATIM
)

ADD_CUSTOM_TARGET( java_api_test
    COMMAND ${Java_JAVAC_EXECUTABLE}
	ARGS -classpath ${CMAKE_BINARY_DIR}/swig/java//buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/swig/java/ ${CMAKE_CURRENT_SOURCE_DIR}/ApplicationApiTest.java
    COMMENT "Building ApplicationApiTest.java"
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
    VERBATIM
)

ADD_TEST(NAME java_hello COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path${CMAKE_BINARY_DIR}/swig/java/-classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/swig/java/ HelloWorld)

ADD_TEST(NAME java_application_api_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java/ -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/swig/java/ ApplicationApiTest)

