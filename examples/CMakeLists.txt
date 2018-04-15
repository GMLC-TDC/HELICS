##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

IF (EXAMPLES_WARNINGS_AS_ERROR)
	#the examples had better compile without warnings otherwise they should not be allowed
	IF(WIN32)
		IF (MSVC)
			add_compile_options(/WX)
		ELSE(MSVC)
			add_compile_options(-Werror)
		ENDIF(MSVC)
	ELSE(WIN32)
		add_compile_options(-Werror)
	ENDIF(WIN32)
ENDIF()

add_subdirectory (ipcTest)
add_subdirectory (valueFederates1)
add_subdirectory (messageFederates1)
add_subdirectory (FilterFederate1)
add_subdirectory (comboFederates1)

if(BUILD_C_SHARED_LIB)
	add_subdirectory (CInterface)
	add_subdirectory (CppInterface)
endif(BUILD_C_SHARED_LIB)

