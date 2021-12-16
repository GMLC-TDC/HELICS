# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# takes a list of names and license file paths and combines the contents into a single
# file
function(combineLicenses OUT_FILE)
    file(WRITE "${OUT_FILE}.in" "Third-Party Licenses")
    list(LENGTH ARGN num_entries)
    math(EXPR end_entries "${num_entries}-1")
    foreach(
        entry
        RANGE
        0
        ${end_entries}
        2
    )
        file(
            APPEND "${OUT_FILE}.in"
            "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"
        )
        list(GET ARGN ${entry} entry_name)
        math(EXPR entry_file "${entry}+1")
        list(GET ARGN ${entry_file} entry_file)
        file(APPEND "${OUT_FILE}.in" "License for ${entry_name}:\n\n")
        file(READ ${entry_file} file_contents)
        file(APPEND "${OUT_FILE}.in" "${file_contents}")
    endforeach()
    configure_file("${OUT_FILE}.in" ${OUT_FILE} COPYONLY)
endfunction()
