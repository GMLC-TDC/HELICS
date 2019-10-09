#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# SPDX-License-Identifier: BSD-3-Clause
#

find_package(Git)

function(git_version_describe source_path result)
  if (GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} --work-tree ${source_path} describe --tags OUTPUT_VARIABLE TAG_DESCRIPTION)
	
	string(LENGTH ${TAG_DESCRIPTION} TAG_LENGTH)
	if (TAG_LENGTH GREATER 12)
		execute_process(COMMAND ${GIT_EXECUTABLE} --work-tree ${source_path} describe --all --tags --dirty OUTPUT_VARIABLE TAG_DESCRIPTION2)
	endif()

	message(STATUS "${TAG_DESCRIPTION}" )
    message(STATUS "${TAG_DESCRIPTION2}" )
	set(${result} ${TAG_DESCRIPTION} PARENT_SCOPE)
  else()
	set(${result} "" PARENT_SCOPE)
  endif(GIT_FOUND)
endfunction()

