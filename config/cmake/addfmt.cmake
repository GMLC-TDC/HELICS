

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------

set(FMT_SILENT ON)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14) # Supported values are ``14``, and ``17``.
endif()

set(SUPPORTS_VARIADIC_TEMPLATES ON)
set(SUPPORTS_USER_DEFINED_LITERALS ON)
set(FMT_HAS_VARIANT OFF)

# get the FMT header only library
add_subdirectory(ThirdParty/fmtlib EXCLUDE_FROM_ALL)
hide_variable(FMT_DOC)
hide_variable(FMT_INSTALL)
hide_variable(FMT_PEDANTIC)
hide_variable(FMT_TEST)
hide_variable(FMT_WERROR)
hide_variable(FMT_FUZZ)
