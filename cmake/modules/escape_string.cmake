function(escape_string outstring instring )
#message(STATUS "${outstring} ${instring}")

STRING(REGEX REPLACE "\\\\" "\\\\\\\\" OUT_STRING_TEMP ${instring})
STRING(REGEX REPLACE "\\\(" "\\\\(" OUT_STRING_TEMP ${OUT_STRING_TEMP})
STRING(REGEX REPLACE "\\\)" "\\\\)" OUT_STRING_TEMP ${OUT_STRING_TEMP})

#STRING(REGEX REPLACE ";" "\\\\;" ${outstring} ${outstring})
STRING(REGEX REPLACE " " "\\\\ " OUT_STRING_TEMP ${OUT_STRING_TEMP})
#STRING(REGEX REPLACE "\\\"" "\\\\"" OUT_STRING_TEMP ${OUT_STRING_TEMP})
#message(STATUS "${OUT_STRING_TEMP} :: ${instring}")

#message(STATUS "${${outstring}}")

#message(STATUS "${outstring} should be set to ${OUT_STRING_TEMP}")
set(${outstring} ${OUT_STRING_TEMP} PARENT_SCOPE)
 endfunction()