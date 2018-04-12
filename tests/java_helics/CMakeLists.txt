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

ADD_CUSTOM_TARGET( Java_MessageFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
	COMMENT "Building MessageFederate.java"
    VERBATIM
)

ADD_CUSTOM_TARGET( Java_MessageFilter ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFilter.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
	COMMENT "Building MessageFilter.java"
    VERBATIM
)

ADD_CUSTOM_TARGET( Java_ValueFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestValueFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar
	COMMENT "Building ValueFederate.java"
    VERBATIM
)

ADD_TEST(NAME java_hello COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics HelloWorld)

ADD_TEST(NAME java_application_api_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics ApplicationApiTest)

ADD_TEST(NAME java_MessageFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestMessageFederate)

ADD_TEST(NAME java_MessageFilter_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestMessageFilter)

ADD_TEST(NAME java_ValueFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestValueFederate)

set_tests_properties(java_hello java_application_api_test java_MessageFederate_test java_MessageFilter_test java_ValueFederate_test PROPERTIES LABELS Java)
