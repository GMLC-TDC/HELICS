# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hiding it merely makes it internal)

macro(HIDE_VARIABLE var)
  if(DEFINED ${var})
    set(${var} "${${var}}" CACHE INTERNAL "")
  endif(DEFINED ${var})
endmacro(HIDE_VARIABLE)

macro(SHOW_VARIABLE var type doc default)
  if(DEFINED ${var})
    set(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  else(DEFINED ${var})
    set(${var} "${default}" CACHE "${type}" "${doc}")
  endif(DEFINED ${var})
endmacro(SHOW_VARIABLE)