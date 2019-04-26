

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------

file(COPY ${HELICS_SOURCE_DIR}/config/cxx14.cmake DESTINATION ThirdParty/fmtlib/support/cmake/)
set(FMT_ASSUME_CPP14_SUPPORT ON)
set(FMT_SILENT ON)
# get the FMT header only library
add_subdirectory(ThirdParty/fmtlib EXCLUDE_FROM_ALL)
hide_variable(FMT_DOC)
hide_variable(FMT_INSTALL)
hide_variable(FMT_PEDANTIC)
hide_variable(FMT_TEST)
hide_variable(FMT_WERROR)