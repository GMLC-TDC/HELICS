configure_file(helicsJava.c ${TARGET_DIR}/helicsJava.c COPYONLY)

 FILE(GLOB JAVA_FILES *.java)

 FILE(COPY ${JAVA_FILES} DESTINATION ${TARGET_DIR})

 message(STATUS "overwriting java files in source directory")