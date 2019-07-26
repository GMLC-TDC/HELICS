

# takes a list of names and license file paths and combines the contents into a single file
function(combineLicenses OUT_FILE)
	set(COMBINE_TMP "Third-Party Licenses")
	list(LENGTH ARGN num_entries)
	math(EXPR end_entries "${num_entries}/2-1")
	foreach(entry RANGE 0 ${end_entries} 2)
		string(APPEND COMBINE_TMP "\n==============================\n")
		list(GET ARGN ${entry} entry_name)
		math(EXPR entry_file "${entry}+1")
		list(GET ARGN ${entry_file} entry_file)
		string(APPEND COMBINE_TMP "License for ${entry_name}:\n\n")
		file(READ ${entry_file} file_contents)
		string(APPEND COMBINE_TMP "${file_contents}")
	endforeach()
	file(WRITE "${OUT_FILE}.in" "${COMBINE_TMP}")
	configure_file("${OUT_FILE}.in" ${OUT_FILE} COPYONLY)
endfunction()
