configure_file(helicsMEX.cpp ${TARGET_DIR}/helicsMEX.cpp COPYONLY)

 FILE(GLOB MATLAB_FILES *.m)
 message(STATUS ${MATLAB_FILES})
 list(REMOVE_ITEM MATLAB_FILES mkhelicsMEXFile.m generatehelicsMEXFile.m)
 FILE(COPY ${MATLAB_FILES} DESTINATION ${TARGET_DIR})
 FILE(COPY +helics DESTINATION ${TARGET_DIR})
 message(STATUS "overwriting matlab files in source directory")
