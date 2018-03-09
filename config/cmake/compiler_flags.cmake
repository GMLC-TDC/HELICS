
OPTION(ENABLE_EXTRA_COMPILER_WARNINGS "disable compiler warning for ${CMAKE_PROJECT_NAME} build" ON)
OPTION(ENABLE_ERROR_ON_WARNINGS "generate a compiler error for any warning encountered" OFF)

# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID}")
IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built

  if (ENABLE_ERROR_ON_WARNINGS)
  add_compile_options(-Werror)
  endif(ENABLE_ERROR_ON_WARNINGS)

   if (ENABLE_EXTRA_COMPILER_WARNINGS)
  add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
  #this options produces lots of warning but is useful for checking every once in a while with Clang, GCC warning notices with this aren't as useful
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)
  # add some gnu specific options if the compiler is newer
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
     # this option produces a number of warnings in third party libraries but useful for checking for any internal usages
 # add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wlogical-op>)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
   add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wduplicated-cond>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wnull-dereference>)
  endif()
  endif()
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  option (USE_LIBCXX "Use Libc++ vs as opposed to the default" OFF)
  IF(USE_LIBCXX)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
      link_libraries("-stdlib=libc++")
     ENDIF(USE_LIBCXX)
ELSE(UNIX)
  IF(MINGW)
    if (ENABLE_ERROR_ON_WARNINGS)
  add_compile_options(-Werror)
  endif(ENABLE_ERROR_ON_WARNINGS)

  if (ENABLE_EXTRA_COMPILER_WARNINGS)
  add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)

  #this options produces lots of warning but is useful for checking ever once in a while with Clang, GCC warning notices with this aren't as useful
  #add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   # this option produces a number of warnings in third party libraries but useful for checking for any internal usages
 # add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wlogical-op>)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
   add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wduplicated-cond>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wnull-dereference>)
  endif()
  endif()
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
  option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MINGW)
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
IF(MSVC)
  if (ENABLE_ERROR_ON_WARNINGS)
  add_compile_options(/WX)
  endif(ENABLE_ERROR_ON_WARNINGS)

  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(/MP /EHsc)
  add_compile_options(/sdl)
  if (ENABLE_EXTRA_COMPILER_WARNINGS)
  add_compile_options(-W4  /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484 /wd4702 /wd4996 )
  endif(ENABLE_EXTRA_COMPILER_WARNINGS)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0601)
ENDIF(MSVC)
  ENDIF(MINGW)
ENDIF(UNIX)

# -------------------------------------------------------------
# Check and set latest CXX Standard supported by compiler
# -------------------------------------------------------------
OPTION(ENABLE_CXX_17 "set to ON to enable C++17 compilation features" OFF)
include(CheckLatestCXXStandardOption)
IF (VERSION_OPTION)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
ELSE ()
	set(CMAKE_CXX_STANDARD 14)
ENDIF ()

