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

message(STATUS "include ${OCTAVE_INCLUDE_DIRS} lib ${OCTAVE_LINK_DIRS}")

MESSAGE(STATUS ${CMAKE_SOURCE_DIR}/swig/octave)
#MESSAGE(STATUS ${CMAKE_CURRENT_BINARY_DIR})

#add_custom_target("helicsOCT" ALL)

ADD_CUSTOM_TARGET(helicsOCT ALL
    COMMAND ${MKOCTFILE_EXECUTABLE} "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/" -l $<TARGET_LINKER_FILE:helicsSharedLib> "helicsOCTAVE_wrap.cpp"
	DEPENDS helicsOctave helicsSharedLib
    COMMENT "generating OCT file"
)

# INCLUDE_DIRECTORIES(${Matlab_INCLUDE_DIRS})

#ADD_CUSTOM_TARGET("helicsMATLABMEX" ALL DEPENDS helicsSharedLib)

#ADD_CUSTOM_COMMAND(
#    TARGET "helicsMATLABMEX" POST_BUILD
#    COMMAND "mex" #
#    ARGS "-I../../../src/helics/shared_api_library" "helicsMEX.cxx" "-lhelicsSharedLib" "-L../../src/helics/shared_api_library/"
#    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
#    COMMENT "Building Matlab extension"
#)
