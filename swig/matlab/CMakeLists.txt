# Generate SWIG wrapper (for both MATLAB)

FIND_PACKAGE(MATLAB REQUIRED)

MESSAGE(STATUS "Building MATLAB")
IF (DISABLE_SWIG)
ELSE(DISABLE_SWIG)

ENDIF(DISABLE_SWIG)


# Compile MEX file
set_property(SOURCE helicsMatlab.i PROPERTY CPLUSPLUS ON)

# SWIG_ADD_LIBRARY(helicsMEX TYPE MODULE LANGUAGE matlab SOURCES ../helics.i)

add_custom_target("helicsMATLAB" ALL)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# INCLUDE_DIRECTORIES(${Matlab_INCLUDE_DIRS})

ADD_CUSTOM_TARGET("helicsMATLABMEX" ALL DEPENDS helicsSharedLib)

ADD_CUSTOM_COMMAND(
    TARGET "helicsMATLABMEX" POST_BUILD
    COMMAND "mex" #
    ARGS "-I../../../src/helics/shared_api_library" "helicsMEX.cxx" "-lhelicsSharedLib" "-L../../src/helics/shared_api_library/"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Building Matlab extension"
)

set_target_properties (helicsMATLABMEX PROPERTIES FOLDER interfaces)