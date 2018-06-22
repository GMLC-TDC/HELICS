find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)


add_custom_target( Java_hello_world ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.java
    DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building HelloWorld.java"
    VERBATIM
)

add_custom_target( Java_api_test ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/ApplicationApiTest.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
    COMMENT "Building ApplicationApiTest.java"
    VERBATIM
)

add_custom_target( Java_MessageFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building MessageFederate.java"
    VERBATIM
)

add_custom_target( Java_MessageFilter ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestMessageFilter.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building MessageFilter.java"
    VERBATIM
)

add_custom_target( Java_ValueFederate ALL
    ${Java_JAVAC_EXECUTABLE} -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar -d ${CMAKE_BINARY_DIR}/tests/java_helics ${CMAKE_CURRENT_SOURCE_DIR}/TestValueFederate.java
	DEPENDS JNIhelics ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar
	COMMENT "Building ValueFederate.java"
    VERBATIM
)

add_test(NAME Java_hello_world_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar:${CMAKE_BINARY_DIR}/tests/java_helics HelloWorld)

add_test(NAME Java_application_api_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar:${CMAKE_BINARY_DIR}/tests/java_helics ApplicationApiTest)

add_test(NAME Java_MessageFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestMessageFederate)

add_test(NAME Java_MessageFilter_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestMessageFilter)

add_test(NAME Java_ValueFederate_test COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${CMAKE_BINARY_DIR}/swig/java -classpath ${CMAKE_BINARY_DIR}/swig/java/buildjar/helics-${HELICS_VERSION}.jar:${CMAKE_BINARY_DIR}/tests/java_helics TestValueFederate)

set_target_properties (Java_hello_world Java_api_test Java_MessageFederate Java_MessageFilter Java_ValueFederate PROPERTIES FOLDER tests)

set_property(TEST Java_hello_world_test Java_application_api_test Java_MessageFederate_test Java_MessageFilter_test Java_ValueFederate_test PROPERTY LABELS Java Continuous Daily Release)
