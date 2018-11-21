
get_filename_component(LIBRARY_BUILD_LOCATION ${LIBRARY_FILE} DIRECTORY)
get_filename_component(LIBRARY_NAME ${LIBRARY_FILE} NAME_WE)

IF (UNIX)
	string(REGEX REPLACE "^lib" "" LIBRARY_NAME ${LIBRARY_NAME})
ENDIF(UNIX)

get_filename_component(BUILD_DIR ${BUILD_FILE} DIRECTORY)
configure_file(${SOURCE_DIR}/mkhelicsOCTFile.m.in ${BUILD_DIR}/mkhelicsOCTFile.m)
