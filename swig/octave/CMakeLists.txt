# Generate SWIG wrapper (for Octave)

MESSAGE(STATUS "Building Octave interface")


FIND_PACKAGE(Octave REQUIRED)
if (NOT MKOCTFILE_EXECUTABLE)
SHOW_VARIABLE(OCTAVE_INSTALL_LOCATION PATH "installation location for Octave" "${OCTAVE_INSTALL_LOCATION}")
endif()

# Compile OCT file
set_property(SOURCE ../helics.i PROPERTY CPLUSPLUS ON)

link_directories(${OCTAVE_LINK_DIRS})

SWIG_ADD_LIBRARY(helicsOctave TYPE MODULE LANGUAGE octave SOURCES ../helics.i)
target_include_directories(helicsOctave PRIVATE ${OCTAVE_INCLUDE_DIRS})
if (UNIX OR MINGW)
target_compile_options(helicsOctave PRIVATE -Wno-unused-parameter -Wno-overloaded-virtual -Wno-pedantic)
if (MINGW)
target_compile_definitions(helicsOctave PRIVATE -DEBADMSG=0x456 -DEIDRM=0x242 -DENOLINK=0x256f -DENODATA=0x253f -DENOSR=0x222f -DENOSTR=0x12341 -DENOTRECOVERABLE=0x6227 -DETIME=0x34536 -DETXTBSY=0x6252e)
endif(MINGW)
endif()

SWIG_LINK_LIBRARIES(helicsOctave ${OCTAVE_LIBRARY})

set_target_properties (helicsOctave PROPERTIES FOLDER interfaces)

message(STATUS "include ${OCTAVE_INCLUDE_DIRS} lib ${OCTAVE_LINK_DIRS}")

MESSAGE(STATUS ${CMAKE_SOURCE_DIR}/swig/octave)
#MESSAGE(STATUS ${CMAKE_CURRENT_BINARY_DIR})

#add_custom_target("helicsOCT" ALL)

#ADD_CUSTOM_COMMAND(
#    TARGET "helicsOctave" PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_SOURCE_DIR}/swig/octave ${CMAKE_CURRENT_BINARY_DIR}
#    COMMENT "Copying OCTAVE files"
#    VERBATIM
#)

# INCLUDE_DIRECTORIES(${Matlab_INCLUDE_DIRS})

#ADD_CUSTOM_TARGET("helicsMATLABMEX" ALL DEPENDS helicsSharedLib)

#ADD_CUSTOM_COMMAND(
#    TARGET "helicsMATLABMEX" POST_BUILD
#    COMMAND "mex" #
#    ARGS "-I../../../src/helics/shared_api_library" "helicsMEX.cxx" "-lhelicsSharedLib" "-L../../src/helics/shared_api_library/"
#    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
#    COMMENT "Building Matlab extension"
#)
