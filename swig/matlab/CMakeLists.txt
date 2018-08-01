#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC All rights
# reserved. See LICENSE file and DISCLAIMER for more details.
#

# Generate SWIG wrapper for MATLAB
if(NOT MATLAB_GENERATE_ONLY)
  set(Matlab_FIND_COMPONENTS "MAIN_PROGRAM")
  find_package(MATLAB)
endif()

if(NOT DISABLE_SWIG AND SWIG_EXECUTABLE)

  execute_process(COMMAND ${SWIG_EXECUTABLE} -help
                  OUTPUT_VARIABLE SWIG_HELP_OUTPUT)
  string(FIND "${SWIG_HELP_OUTPUT}" "-matlab" MATLAB_HELP_FOUND)
  if(${MATLAB_HELP_FOUND} LESS 0)
    set(MATLAB_SWIG_NOT_AVAILABLE 1)
    message(
      warning " SWIG VERSION does not support matlab, reverting to build only")
  endif()
endif()
message(STATUS "Building MATLAB")
if(DISABLE_SWIG OR MATLAB_SWIG_NOT_AVAILABLE)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
                     COMMAND "${CMAKE_COMMAND}" -E copy
                             ${CMAKE_CURRENT_SOURCE_DIR}/helicsMEX.cpp
                             ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
                     DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/helicsMEX.cpp)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/+helics
          DESTINATION matlab
          COMPONENT matlab)

  file(GLOB MATLAB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.m)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/+helics
          DESTINATION matlab
          COMPONENT matlab)
  install(FILES ${MATLAB_FILES} DESTINATION matlab COMPONENT matlab)
else()

  file(GLOB SHARED_LIB_HEADERS
            ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

  # custom command for building the wrap file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
    COMMAND "${SWIG_EXECUTABLE}" "-matlab" "-c++" -o "helicsMEX.cpp"
            "-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
            ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
    DEPENDS ${CMAKE_SOURCE_DIR}/swig/helics.i
            ${CMAKE_CURRENT_SOURCE_DIR}/helicsMatlab.i
            ${SHARED_LIB_HEADERS}
            ${CMAKE_CURRENT_SOURCE_DIR}/matlab_maps.i)

  if(OVERWRITE_MATLAB_FILES)
    add_custom_target(
      mfile_overwrite ALL
      COMMAND ${CMAKE_COMMAND} -D TARGET_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
              ${CMAKE_CURRENT_SOURCE_DIR}/overwriteMatlabFiles.cmake
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp)
  endif(OVERWRITE_MATLAB_FILES)

  # add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp COMMAND
  # ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake
  # DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+helics
          DESTINATION matlab
          COMPONENT matlab)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m
                ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m
                ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m
          DESTINATION matlab
          COMPONENT matlab)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pisender.m
                ${CMAKE_CURRENT_SOURCE_DIR}/pireceiver.m
                ${CMAKE_CURRENT_SOURCE_DIR}/GetFullPath.m
          DESTINATION matlab
          COMPONENT matlab)
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m
  COMMAND
    ${CMAKE_COMMAND} -D LIBRARY_FILE=$<TARGET_LINKER_FILE:helicsSharedLib> -D
    BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp -D
    LIBRARY_INCLUDE_LOCATION=${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/
    -D
    OTHER_INCLUDE_LOCATION=${PROJECT_BINARY_DIR}/src/helics/shared_api_library/
    -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P
    ${CMAKE_CURRENT_SOURCE_DIR}/generateMEXcreationScript.cmake
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp)

if(WIN32)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
    COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nosplash -minimize -r
            "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m helicsSharedLib
            ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp)
else()
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
    COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r
            "\"run('${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m');quit;\""
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsMEXFile.m helicsSharedLib
            ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp)
endif()

if(NOT MATLAB_GENERATE_ONLY)
  add_custom_target(
    helicsMEX ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
            helicsSharedLib)
  set_target_properties(helicsMEX PROPERTIES FOLDER interfaces)
endif(NOT MATLAB_GENERATE_ONLY)

file(GLOB HSHARED_LIB_SOURCES
          ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.${Matlab_MEX_EXTENSION}
        DESTINATION matlab
        COMPONENT matlab)
install(FILES $<TARGET_FILE:helicsSharedLib>
        DESTINATION matlab
        COMPONENT matlab)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsMEX.cpp
        DESTINATION matlab
        COMPONENT matlab)
install(FILES $<TARGET_LINKER_FILE:helicsSharedLib>
        DESTINATION matlab
        COMPONENT matlab)
install(FILES ${KEY_LIBRARY_FILES} DESTINATION matlab COMPONENT matlab)
install(FILES ${HSHARED_LIB_SOURCES}
        DESTINATION matlab/headers
        COMPONENT matlab)
install(
  FILES ${PROJECT_BINARY_DIR}/src/helics/shared_api_library/helics_export.h
  DESTINATION matlab/headers
  COMPONENT matlab)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generatehelicsMEXFile.m
        DESTINATION matlab
        COMPONENT matlab)
install(FILES helicsSharedLib.h libhelicsSharedLib.h
        DESTINATION matlab
        COMPONENT matlab)
