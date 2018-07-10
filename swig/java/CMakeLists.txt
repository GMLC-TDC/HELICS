##############################################################################
#Copyright (c) 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

find_package(Java REQUIRED)
find_package(JNI REQUIRED)
include(UseJava)

include_directories(${JAVA_INCLUDE_PATH})
include_directories(${JNI_INCLUDE_DIRS})

if (DISABLE_SWIG OR NOT SWIG_EXECUTABLE)
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		# Update if necessary
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long")
	endif()
	add_library(JNIhelics SHARED interface/helicsJavaJAVA_wrap.c)
	target_link_libraries(JNIhelics helicsSharedLib ${JAVA_LIBRARIES})
	set_target_properties (JNIhelics PROPERTIES FOLDER interfaces)
else(DISABLE_SWIG)
	#Enable generation using swig

	set_property(SOURCE helicsJava.i PROPERTY SWIG_MODULE_NAME JNIhelics)

	set(CMAKE_SWIG_FLAGS "-package;com.java.helics")
	if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER 3.7)
		SWIG_ADD_LIBRARY(JNIhelics TYPE MODULE LANGUAGE java SOURCES helicsJava.i)
	else ()
		SWIG_ADD_MODULE(JNIhelics java helicsJava.i)
	endif ()

	SWIG_LINK_LIBRARIES(JNIhelics helicsSharedLib)
	SWIG_LINK_LIBRARIES(JNIhelics ${JAVA_LIBRARIES})


	set_target_properties (JNIhelics PROPERTIES FOLDER interfaces)

endif() #DISABLE_SWIG

configure_file(MakeJarCMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt @ONLY)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/buildjar/)

# -D ADDITIONAL_JAR_FILES=$<TARGET_FILE:helicsSharedLib>;$<TARGET_FILE:JNIhelics>
add_custom_command(
    TARGET "JNIhelics" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_FILE:JNIhelics> -P ${CMAKE_CURRENT_SOURCE_DIR}/addLoadLibraryCommand.cmake
)

add_custom_command(
    TARGET "JNIhelics" POST_BUILD
    COMMAND ${CMAKE_COMMAND}
	ARGS ..
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/buildjar"
	VERBATIM
)

add_custom_command(
    TARGET "JNIhelics" POST_BUILD
    COMMAND ${CMAKE_COMMAND}
	ARGS  --build . --target helics
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/buildjar/"
    COMMENT "Building jar file"
	VERBATIM
)

if (WIN32)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET JNIhelics POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:JNIhelics>/")                 # <--this is out-file path
endforeach(keyfile)

add_custom_command(TARGET JNIhelics POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
        "$<TARGET_FILE_DIR:JNIhelics>/")                 # <--this is out-file path

endif ()
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/buildjar/helics-${HELICS_VERSION}.jar DESTINATION java COMPONENT java)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/buildjar/helics.jar DESTINATION java COMPONENT java OPTIONAL)
install(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION java COMPONENT java)
install(TARGETS JNIhelics DESTINATION java COMPONENT java)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION java COMPONENT java)
