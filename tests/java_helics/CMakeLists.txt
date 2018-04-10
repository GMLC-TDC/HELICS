FIND_PACKAGE(Java REQUIRED)
FIND_PACKAGE(JNI REQUIRED)
INCLUDE(UseJava)


ADD_CUSTOM_TARGET( Java_hello_world ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
	COMMENT "Building HelloWorld.java"
    VERBATIM
)

ADD_CUSTOM_TARGET( java_api_test ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/ApplicationApiTest.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
    COMMENT "Building ApplicationApiTest.java"
    VERBATIM
)

ADD_TEST(NAME java_hello COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics HelloWorld)

ADD_TEST(NAME java_application_api_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics ApplicationApiTest)

