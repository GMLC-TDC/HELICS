configure_file(helicsMex.cpp ${TARGET_DIR}/helicsMex.cpp COPYONLY)

 FILE(GLOB MATLAB_FILES *.m)
 
 FILE(COPY ${MATLAB_FILES} DESTINATION ${TARGET_DIR})
 FILE(COPY +helics DESTINATION ${TARGET_DIR})
 message(STATUS "writing matlab files to source directory")