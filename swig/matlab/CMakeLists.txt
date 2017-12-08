# Generate SWIG wrapper (for both MATLAB and Octave)

MESSAGE(STATUS "Building MATLAB")

# Compile MEX file
set_property(SOURCE ../helics.i PROPERTY CPLUSPLUS ON)

# SWIG_ADD_LIBRARY(helicsMEX TYPE MODULE LANGUAGE matlab SOURCES ../helics.i)

MESSAGE(STATUS ${CMAKE_SOURCE_DIR}/swig/matlab)
MESSAGE(STATUS ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target("helicsMATLAB" ALL)

ADD_CUSTOM_COMMAND(
    TARGET "helicsMATLAB" PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/swig/matlab ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copying MATLAB files"
    VERBATIM
)

# INCLUDE_DIRECTORIES(${Matlab_INCLUDE_DIRS})

ADD_CUSTOM_TARGET("helicsMATLABMEX" ALL DEPENDS helicsSharedLib)

ADD_CUSTOM_COMMAND(
    TARGET "helicsMATLABMEX" POST_BUILD
    COMMAND "mex" #
    ARGS "-I../../../src/helics/shared_api_library" "helicsMEX.cxx" "-lhelicsSharedLib" "-L../../src/helics/shared_api_library/"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building Matlab extension"
)
