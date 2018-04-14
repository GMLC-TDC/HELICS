# Generate SWIG wrapper (for Octave)
MESSAGE(STATUS "Building Octave interface")

FIND_PACKAGE(Octave REQUIRED)
if (NOT MKOCTFILE_EXECUTABLE)
	SHOW_VARIABLE(OCTAVE_INSTALL_LOCATION PATH "installation location for Octave" "${OCTAVE_INSTALL_LOCATION}")
endif()

if (DISABLE_SWIG OR NOT SWIG_EXECUTABLE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helicsOCTAVE_wrap.cpp ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp COPYONLY)

else()

# Compile OCT file
#add_custom_target("helicsOCT" ALL)
FILE(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

#custom command for building the wrap file
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
  COMMAND "${SWIG_EXECUTABLE}"
    "-octave" "-c++"
    -o "helicsOCTAVE_wrap.cxx"
	"-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
	${CMAKE_CURRENT_SOURCE_DIR}/helicsOctave.i
  DEPENDS ${CMAKE_SOURCE_DIR}/swig/helics.i ${CMAKE_SOURCE_DIR}/swig/octave/helicsOctave.i ${SHARED_LIB_HEADERS}
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
  COMMAND ${CMAKE_COMMAND} -D VOID_SIZE=${CMAKE_SIZEOF_VOID_P} -P ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
)

endif()


ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m
  COMMAND ${CMAKE_COMMAND} 
  -D LIBRARY_FILE=$<TARGET_LINKER_FILE:helicsSharedLib> 
  -D BUILD_FILE=${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp 
  -D LIBRARY_INCLUDE_LOCATION=${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/
  -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  -P ${CMAKE_CURRENT_SOURCE_DIR}/generateOCTcreationScript.cmake
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helics.oct
  COMMAND ${OCTAVE_EXECUTABLE} "-q" "run(\"${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m\")"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOCTFile.m helicsSharedLib ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
)

ADD_CUSTOM_TARGET(helicsOCT ALL
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helics.oct
)

set_target_properties (helicsOCT PROPERTIES FOLDER interfaces)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.oct DESTINATION octave COMPONENT octave)
INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION octave COMPONENT octave)
INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION octave COMPONENT octave)
