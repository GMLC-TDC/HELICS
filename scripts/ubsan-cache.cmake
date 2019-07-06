set(CMAKE_C_FLAGS "-fsanitize=undefined -fsanitize-blacklist=.ci/undef_blacklist.txt -g" CACHE STRING "Flags used by the compiler during all build types.")
set(CMAKE_CXX_FLAGS "-fsanitize=undefined -fsanitize-blacklist=.ci/undef_blacklist.txt -g" CACHE STRING "Flags used by the compiler during all build types.")
set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=undefined -fsanitize-blacklist=.ci/undef_blacklist.txt" CACHE STRING "Flags used by the linker during all build types.")
set(CMAKE_MODULE_LINKER_FLAGS "-fsanitize=undefined -fsanitize-blacklist=.ci/undef_blacklist.txt" CACHE STRING "Flags used by the linker during all build types.")
