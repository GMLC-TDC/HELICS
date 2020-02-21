# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

function(escape_string outstring instring)
    # message(STATUS "${outstring} ${instring}")

    string(REGEX REPLACE "\\\\" "\\\\\\\\" OUT_STRING_TEMP ${instring})
    string(REGEX REPLACE "\\\(" "\\\\(" OUT_STRING_TEMP ${OUT_STRING_TEMP})
    string(REGEX REPLACE "\\\)" "\\\\)" OUT_STRING_TEMP ${OUT_STRING_TEMP})

    # string(REGEX REPLACE ";" "\\\\;" ${outstring} ${outstring})
    string(REGEX REPLACE " " "\\\\ " OUT_STRING_TEMP ${OUT_STRING_TEMP})
    # string(REGEX REPLACE "\\\"" "\\\\"" OUT_STRING_TEMP ${OUT_STRING_TEMP})
    # message(STATUS "${OUT_STRING_TEMP} :: ${instring}")

    # message(STATUS "${${outstring}}")

    # message(STATUS "${outstring} should be set to ${OUT_STRING_TEMP}")
    set(${outstring} ${OUT_STRING_TEMP} PARENT_SCOPE)
endfunction()
