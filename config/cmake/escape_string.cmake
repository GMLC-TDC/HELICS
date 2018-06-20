function(escape_string outstring instring )
#message(STATUS "${outstring} ${instring}")

string(REGEX REPLACE "\\\\" "\\\\\\\\" OUT_STRING_TEMP ${instring})
string(REGEX REPLACE "\\\(" "\\\\(" OUT_STRING_TEMP ${OUT_STRING_TEMP})
string(REGEX REPLACE "\\\)" "\\\\)" OUT_STRING_TEMP ${OUT_STRING_TEMP})

#string(REGEX REPLACE ";" "\\\\;" ${outstring} ${outstring})
string(REGEX REPLACE " " "\\\\ " OUT_STRING_TEMP ${OUT_STRING_TEMP})
#string(REGEX REPLACE "\\\"" "\\\\"" OUT_STRING_TEMP ${OUT_STRING_TEMP})
#message(STATUS "${OUT_STRING_TEMP} :: ${instring}")

#message(STATUS "${${outstring}}")

#message(STATUS "${outstring} should be set to ${OUT_STRING_TEMP}")
set(${outstring} ${OUT_STRING_TEMP} PARENT_SCOPE)
 endfunction()

