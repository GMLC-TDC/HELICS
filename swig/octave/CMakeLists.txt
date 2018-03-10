# Generate SWIG wrapper (for Octave)
MESSAGE(STATUS "Building Octave interface")

FIND_PACKAGE(Octave REQUIRED)
if (NOT MKOCTFILE_EXECUTABLE)
	SHOW_VARIABLE(OCTAVE_INSTALL_LOCATION PATH "installation location for Octave" "${OCTAVE_INSTALL_LOCATION}")
endif()

if (DISABLE_SWIG)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/helicsOCTAVE_wrap.cpp ${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp COPYONLY)
else()

# Compile OCT file
link_directories(${OCTAVE_LINK_DIRS})

#add_custom_target("helicsOCT" ALL)
FILE(GLOB SHARED_LIB_HEADERS ${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/*.h)

#custom command for building the wrap file
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
  COMMAND "${SWIG_EXECUTABLE}"
    "-octave" "-c++"
    -o "helicsOCTAVE_wrap.cxx"
	"-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library"
	${CMAKE_SOURCE_DIR}/swig/Octave/helicsOctave.i
  DEPENDS ${CMAKE_SOURCE_DIR}/swig/helics.i ${CMAKE_SOURCE_DIR}/swig/Octave/helicsOctave.i ${SHARED_LIB_HEADERS}
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/modifyOctSourceFile.cmake
  DEPENDS ${PROJECT_CURENT_BINARY_DIR}/helicsOCTAVE_wrap.cxx
)

endif()

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOctFile.m
  COMMAND ${CMAKE_COMMAND} -E echo "mkoctfile('-I${CMAKE_SOURCE_DIR}/src/helics/shared_api_library/', '-L $<TARGET_LINKER_FILE_DIR:helicsSharedLib> -lhelicsSharedLibd -o${CMAKE_CURRENT_BINARY_DIR}/helics.oct','${CMAKE_CURRENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp')" >mkhelicsOctFile.m
  DEPENDS ${PROJECT_CURENT_BINARY_DIR}/helicsOCTAVE_wrap.cpp
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/helics.oct
  COMMAND ${OCTAVE_EXECUTABLE} "-q" "${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOctFile.m"
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mkhelicsOctFile.m helicsSharedLib
)

ADD_CUSTOM_TARGET(helicsOCT ALL
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/helics.oct
)

set_target_properties (helicsOCT PROPERTIES FOLDER interfaces)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.oct DESTINATION ${CMAKE_INSTALL_PREFIX}/octave)
INSTALL(FILES $<TARGET_FILE:helicsSharedLib> DESTINATION octave)
INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION octave)
