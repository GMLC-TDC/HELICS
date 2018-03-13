
get_filename_component(LIBRARY_BUILD_LOCATION ${LIBRARY_FILE} DIRECTORY)
get_filename_component(LIBRARY_NAME ${LIBRARY_FILE} NAME_WE)

get_filename_component(BUILD_DIR ${BUILD_FILE} DIRECTORY)
configure_file(${SOURCE_DIR}/mkhelicsOctFile.m.in ${BUILD_DIR}/mkhelicsOctFile.m)