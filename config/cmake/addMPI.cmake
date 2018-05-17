# LLNS Copyright Start
# Copyright (c) 2014-2018, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End


##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
find_package(MPI)
if(MSVC)

	if (NOT MPI_CXX_COMPILER)
		message(STATUS "not mpi cxx compiler")
		# For building MPI programs the selected Visual Studio compiler is used,
		#namely cl.exe.
		# So there is no need to set a specific MPI compiler.
		set(MPI_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE FILEPATH "Mpi cxx compiler" FORCE)
		set(MPI_C_COMPILER "${CMAKE_C_COMPILER}" CACHE FILEPATH "Mpi c compiler" FORCE)
	endif(NOT MPI_CXX_COMPILER)
	
endif()

############################################################
# Add targets to use
############################################################
if (NOT MPI::MPI_C)
	if (NOT TARGET MPI::MPI_C)
		if(MPI_C_LIBRARIES)
		
			add_library(MPI::MPI_C STATIC IMPORTED)
		
			set_property(TARGET MPI::MPI_C PROPERTY INTERFACE_LINK_LIBRARIES "")
			if(MPI_C_LINK_FLAGS)
				set_property(TARGET MPI::MPI_C APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_C_LINK_FLAGS}")
			endif()
			set_target_properties(MPI::MPI_C PROPERTIES IMPORT_LOCATION ${MPI_C_LIBRARIES})
		else()
			add_library(MPI::MPI_C INTERFACE IMPORTED)
		
			set_property(TARGET MPI::MPI_C PROPERTY INTERFACE_LINK_LIBRARIES "")
			if(MPI_C_LINK_FLAGS)
				set_property(TARGET MPI::MPI_C APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_C_LINK_FLAGS}")
			endif()
		endif()
	else()
		set_target_properties(MPI::MPI_C PROPERTIES INTERFACE_LINK_LIBRARIES "")
		if(MPI_C_LINK_FLAGS)
			set_property(TARGET MPI::MPI_C APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_C_LINK_FLAGS}")
		endif()
		if(MPI_C_LIBRARIES)
			set_property(TARGET MPI::MPI_C APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_C_LIBRARIES}")
		endif()
	endif()
	set_target_properties(MPI::MPI_C PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MPI_C_INCLUDE_DIR};${MPI_C_HEADER_DIR};${MPI_C_ADDITIONAL_INCLUDE_DIRS}")
	set_target_properties(MPI::MPI_C PROPERTIES INTERFACE_COMPILE_OPTIONS "${MPI_C_COMPILE_OPTIONS}")
	set_target_properties(MPI::MPI_C PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${MPI_C_COMPILE_DEFINITIONS}")

endif()