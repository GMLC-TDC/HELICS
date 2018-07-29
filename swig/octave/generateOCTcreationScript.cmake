
get_filename_component(LIBRARY_BUILD_LOCATION ${LIBRARY_FILE} DIRECTORY)
get_filename_component(LIBRARY_NAME_EXT ${LIBRARY_FILE} NAME_WE)

if (WIN32)
set(LIBRARY_NAME ${LIBRARY_NAME_EXT})
else(WIN32)
#string the lib prefix
string(SUBSTRING ${LIBRARY_NAME_EXT} 3 -1 LIBRARY_NAME)
endif(WIN32)

get_filename_component(BUILD_DIR ${BUILD_FILE} DIRECTORY)
configure_file(${SOURCE_DIR}/mkhelicsOCTFile.m.in ${BUILD_DIR}/mkhelicsOCTFile.m)
