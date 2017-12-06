cmake_minimum_required(VERSION 2.8.6)

# Generate SWIG wrapper (for both MATLAB and Octave)
set_source_files_properties(../helics.i)
swig_module_initialize(helicsMEX matlab)
swig_add_source_to_module(helicsMEX FALSE MEX_FILE ../helics.i)
get_directory_property(swig_extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${MEX_FILE}")

# Install the generated front-end to ${MATLAB_PREFIX}
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+helics DESTINATION ${MATLAB_PREFIX})

# Install MATLAB/Octave helper files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigRef.m DESTINATION ${MATLAB_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigMem.m DESTINATION ${MATLAB_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigGet.m DESTINATION ${MATLAB_PREFIX})
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SwigStorage.m DESTINATION ${MATLAB_PREFIX})
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/swig_typename_convertor_matlab2cpp.m
#               ${CMAKE_CURRENT_SOURCE_DIR}/SwigType.m
#               ${CMAKE_CURRENT_SOURCE_DIR}/returntypes_call.m
#               ${CMAKE_CURRENT_SOURCE_DIR}/returntypes.m
#   DESTINATION ${MATLAB_PREFIX})

# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/OptiCallbackHelper.m
#   DESTINATION ${MATLAB_PREFIX}/+helics)
# # MATLAB specific stuff
# if (WITH_MATLAB)
#   # Make target
#   add_custom_target(matlab DEPENDS helicsMATLAB)

#   # Find MATLAB
#   find_package(MATLAB REQUIRED)

#   # Compile MEX file
#   if (EXISTS ${MATLAB_INCLUDE_DIR}/../version/cpp_mexapi_version.cpp)
#     add_library(helicsMATLAB MODULE ${MEX_FILE} ${MATLAB_INCLUDE_DIR}/../version/cpp_mexapi_version.cpp)
#   else()
#     add_library(helicsMATLAB MODULE ${MEX_FILE})
#   endif()
#   target_include_directories(helicsMATLAB PUBLIC ${MATLAB_INCLUDE_DIR})
#   target_link_libraries(helicsMATLAB ${MATLAB_LIBRARIES} helics)
#   set_target_properties(helicsMATLAB
#     PROPERTIES OUTPUT_NAME "helicsMEX"
#                PREFIX ""
#                SUFFIX .${MATLAB_MEX_EXT})
#   install(TARGETS helicsMATLAB DESTINATION ${MATLAB_PREFIX})
# endif (WITH_MATLAB)

# # Octave specific stuff
# if (WITH_OCTAVE)
#   # Make target
#   add_custom_target(octave DEPENDS helicsOCTAVE)

#   # Find Octave
#   if (NOT OCTAVE_LIBRARIES)
#   find_package(OCTAVE REQUIRED)
#   endif()

#   # Compile MEX file
#   add_library(helicsOCTAVE MODULE ${MEX_FILE})
#   target_include_directories(helicsOCTAVE PUBLIC ${OCTAVE_INCLUDE_DIRS})
#   if(APPLE)
#     target_link_libraries(helicsOCTAVE helics)
#     set_target_properties(helicsOCTAVE PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
#   else()
#     target_link_libraries(helicsOCTAVE ${OCTAVE_LIBRARIES} helics)
#   endif()
#   set_target_properties(helicsOCTAVE
#     PROPERTIES OUTPUT_NAME "helicsMEX"
#                PREFIX ""
#                SUFFIX .mex)
#   install(TARGETS helicsOCTAVE DESTINATION ${MATLAB_PREFIX})
#     install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../octave/import.m DESTINATION ${MATLAB_PREFIX}/octave-extra)
#     if (WITH_OCTAVE_IMPORT)
#       message(WARNING "CMake option WITH_OCTAVE_IMPORT has been deprecated. 'import.m' is now copied to the octave-extra subdirectory.")
#       install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../octave/import.m DESTINATION ${MATLAB_PREFIX})
#   endif()
# endif (WITH_OCTAVE)

# # Example of how to extend helics with additional features
# if (WITH_EXTENDING_helics AND WITH_MATLAB)
#   set_source_files_properties(../extending_helics/extending_helics.i PROPERTIES CPLUSPLUS ON)
#   swig_add_module(extending_helicsMEX matlab ../extending_helics/extending_helics.i)
#   swig_link_libraries(extending_helicsMEX ${MATLAB_LIBRARIES} extending_helics)
#   set_target_properties(extending_helicsMEX PROPERTIES PREFIX "" SUFFIX .${MATLAB_MEX_EXT})

#   # Install the generated front-end to ${MATLAB_PREFIX}
#   install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/+extending_helics DESTINATION ${MATLAB_PREFIX})
#   install(TARGETS extending_helicsMEX DESTINATION ${MATLAB_PREFIX})

#   add_custom_target(extending_helics_matlab DEPENDS extending_helicsMEX extending_helics)
# endif()