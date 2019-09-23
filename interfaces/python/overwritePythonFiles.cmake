configure_file(helicsPython.c ${TARGET_DIR}/helicsPython.c COPYONLY)

 FILE(GLOB PYTHONI_FILES *.py)
 
 FILE(COPY ${PYTHONI_FILES} DESTINATION ${TARGET_DIR})

 message(STATUS "overwriting python interface files to source directory")