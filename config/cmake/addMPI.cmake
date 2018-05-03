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
	
	message(STATUS "mpi lib ${MPI_C_LIBRARIES} CXX ${MPI_CXX_LIBRARIES}")
   #This is for finding MS-MPI.
   #set(_MPI_PREFIX_PATH)
  #list(APPEND _MPI_PREFIX_PATH
 #"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\MPI;Path]/..")
  # set(MPIEXEC "C:/Program Files/Microsoft MPI/Bin/mpiexec.exe")
  
  
  #set(MPI_CXX_INCLUDE_PATH "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
   # Make sure the correct libraries (64-bit or 32-bit) are selected.
   # Decide between 32-bit and 64-bit libraries for Microsoft's MPI
  # if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
  #   set(MS_MPI_ARCH_DIR x64)
  #else()
  #  set(MS_MPI_ARCH_DIR x86)
  # endif()
  #set(MPI_CXX_LIBRARIES "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/${MS_MPI_ARCH_DIR}/msmpi.lib")
  
  # set(MPI_C_LIBRARIES "{${MPI_CXX_LIBRARIES}")
#	set(MPI_C_FOUND YES)
	#set(MPI_CXX_FOUND YES)
#	endif()
endif()