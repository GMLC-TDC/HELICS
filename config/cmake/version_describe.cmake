#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(Git)

function(git_version_describe source_path result)
  message(STATUS "Source ${source_path}")
  if (GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path} describe --tags OUTPUT_VARIABLE TAG_DESCRIPTION OUTPUT_STRIP_TRAILING_WHITESPACE)
	message(STATUS "output=${TAG_DESCRIPTION}" )
	string(LENGTH ${TAG_DESCRIPTION} tag_length)
	if (tag_length GREATER 16)
	    string(FIND ${TAG_DESCRIPTION} "-" last_dash_loc REVERSE)
		string(SUBSTRING ${TAG_DESCRIPTION} ${last_dash_loc} -1 hash_string)
		message (STATUS "hash string=${hash_string}")
		execute_process(COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path} describe --all --tags --dirty OUTPUT_VARIABLE TAG_DESCRIPTION2 OUTPUT_STRIP_TRAILING_WHITESPACE)
		message(STATUS "${TAG_DESCRIPTION2}" )
		string(FIND ${TAG_DESCRIPTION2} "-dirty" dirty_loc)
		if (dirty_loc GREATER 0)
			string(SUBSTRING ${TAG_DESCRIPTION2} 0 ${dirty_loc} tag_desc)
			string(APPEND tag_desc ${hash_string} "-dirty")
		else()
			set(tag_desc ${TAG_DESCRIPTION2})
			string(APPEND tag_desc ${hash_string})
		endif()
		
		string(FIND ${tag_desc} "heads/" hloc)
		if (hloc EQUAL 0)
			string(SUBSTRING ${tag_desc} 6 -1 tag_desc)
	    endif()
	else()
	   set(tag_desc "")
	endif()


	set(${result} ${tag_desc} PARENT_SCOPE)
  else(GIT_FOUND)
	set(${result} "" PARENT_SCOPE)
  endif(GIT_FOUND)
  message(STATUS "final version tag is ${tag_desc}")
endfunction()

