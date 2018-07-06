##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


if (WIN32)
	configure_file(runTestsWIN32.m.in runTests.m)
	add_test(NAME matlab_tests COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nosplash -minimize -r "\"run('${CMAKE_CURRENT_BINARY_DIR}/runTests.m');quit;\""  )
		
else()
	configure_file(runTestsUNIX.m.in runTests.m)
	add_test(NAME matlab_tests COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r "\"run('${CMAKE_CURRENT_BINARY_DIR}/runTests.m');quit;\"")
endif()
	

