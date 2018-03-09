# Generate SWIG wrapper (for Octave)

MESSAGE(STATUS "Building Octave interface")


FIND_PACKAGE(Octave REQUIRED)
if (NOT MKOCTFILE_EXECUTABLE)
SHOW_VARIABLE(OCTAVE_INSTALL_LOCATION PATH "installation location for Octave" "${OCTAVE_INSTALL_LOCATION}")
endif()

# Compile OCT file
set_property(SOURCE ../helics.i PROPERTY CPLUSPLUS ON)

link_directories(${OCTAVE_LINK_DIRS})
include(useSWIG_extra)

SWIG_GEN_SOURCES(helicsOctave TYPE MODULE LANGUAGE octave SOURCES ../helics.i)

#MESSAGE(STATUS ${CMAKE_SOURCE_DIR}/swig/octave)


#add_custom_target("helicsOCT" ALL)

ADD_CUSTOM_TARGET(helicsOCTfile ALL
    COMMAND ${CMAKE_COMMAND} -E echo "mkoctfile('-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/', '-L $<TARGET_LINKER_FILE_DIR:helicsSharedLib> -lhelicsSharedLibd -o${CMAKE_CURRENT_BINARY_DIR}/helics.oct','${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp')" >mkhelicsOctFile.m
	DEPENDS helicsOctave helicsSharedLib
    COMMENT "generating OCT file script"
	BYPRODUCTS mkhelicsOctFile.m
)

ADD_CUSTOM_TARGET(helicsOCT ALL
    COMMAND ${OCTAVE_EXECUTABLE} "-q" "${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOctFile.m"
	DEPENDS helicsOctave helicsSharedLib helicsOCTfile
    COMMENT "generating OCT file"
	WORKING_DIRECTORY ${OCTAVE_EXECUTABLE_DIR}
)

set_target_properties (helicsOCTfile PROPERTIES FOLDER interfaces)
set_target_properties (helicsOCT PROPERTIES FOLDER interfaces)
