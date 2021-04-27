# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# LLNS Copyright Start
# Copyright (c) 2014-2021, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department
# of Energy by Lawrence Livermore National Laboratory in part under
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# ~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~

find_package(MPI)
if(NOT MPI_C_FOUND)
    if(MSVC) # the cmake find MPI doesn't completely work for visual studio 2017

        if(NOT MPI_CXX_COMPILER)
            # message(STATUS "not mpi cxx compiler") For building MPI programs the
            # selected Visual Studio compiler is used, namely cl.exe. So there is no
            # need to set a specific MPI compiler.
            set(
                MPI_CXX_COMPILER
                "${CMAKE_CXX_COMPILER}"
                CACHE FILEPATH "Mpi CXX compiler" FORCE
            )
            set(
                MPI_C_COMPILER
                "${CMAKE_C_COMPILER}"
                CACHE FILEPATH "Mpi C compiler" FORCE
            )
        endif(NOT MPI_CXX_COMPILER)
        if(MPIEXEC_EXECUTABLE) # if we found this then the target was found
            if(NOT MPI_C_LIBRARIES)
                if(MPI_msmpi_LIBRARY)
                    set(
                        MPI_C_LIBRARIES
                        ${MPI_msmpi_LIBRARY}
                        CACHE STRING "MPI C libraries" FORCE
                    )
                else()
                    # TODO not sure how MPICH libraries are laid out on Windows
                endif()
            endif()
            set(MPI_C_FOUND TRUE CACHE BOOL "MPI C FOUND" FORCE)
        endif()
    else()
        # message(STATUS "MPI ${MPIEXEC} yyyyyy88888 ${MPI_C_LIBRARIES}")
        if(MPIEXEC AND MPI_C_LIBRARIES) # if we found this then the target was found
            set(MPI_C_FOUND TRUE)
        endif()
    endif()
endif()
#
# Add targets to use
#
if(MPI_C_FOUND AND NOT TARGET MPI::MPI_C)
    add_library(MPI::MPI_C INTERFACE IMPORTED)
    set_property(TARGET MPI::MPI_C PROPERTY INTERFACE_LINK_LIBRARIES "")
    if(MPI_C_LIBRARIES)
        set_property(
            TARGET MPI::MPI_C
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_C_LIBRARIES}"
        )
    endif()
    if(MPI_C_LINK_FLAGS)
        string(STRIP "${MPI_C_LINK_FLAGS}" trimmed_mpi_c_flags)
        set_property(
            TARGET MPI::MPI_C
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES "${trimmed_mpi_c_flags}"
        )
    endif()
    set_target_properties(
        MPI::MPI_C
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            "${MPI_C_INCLUDE_DIR};${MPI_C_HEADER_DIR};${MPI_C_ADDITIONAL_INCLUDE_DIRS};${MPI_C_INCLUDE_PATH}"
    )
    set_target_properties(
        MPI::MPI_C
        PROPERTIES INTERFACE_COMPILE_OPTIONS "${MPI_C_COMPILE_OPTIONS}"
    )
    set_target_properties(
        MPI::MPI_C
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${MPI_C_COMPILE_DEFINITIONS}"
    )
endif()
if(MPI_CXX_FOUND AND NOT TARGET MPI::MPI_CXX)
    add_library(MPI::MPI_CXX INTERFACE IMPORTED)
    set_property(TARGET MPI::MPI_CXX PROPERTY INTERFACE_LINK_LIBRARIES "")
    if(MPI_CXX_LIBRARIES)
        set_property(
            TARGET MPI::MPI_CXX
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES "${MPI_CXX_LIBRARIES}"
        )
    endif()
    if(MPI_CXX_LINK_FLAGS)
        string(STRIP "${MPI_CXX_LINK_FLAGS}" trimmed_mpi_cxx_flags)
        set_property(
            TARGET MPI::MPI_CXX
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES "${trimmed_mpi_cxx_flags}"
        )
    endif()
    set_target_properties(
        MPI::MPI_CXX
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            "${MPI_CXX_INCLUDE_DIR};${MPI_CXX_HEADER_DIR};${MPI_CXX_ADDITIONAL_INCLUDE_DIRS};${MPI_CXX_INCLUDE_PATH}"
    )
    set_target_properties(
        MPI::MPI_CXX
        PROPERTIES INTERFACE_COMPILE_OPTIONS "${MPI_CXX_COMPILE_OPTIONS}"
    )
    set_target_properties(
        MPI::MPI_CXX
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${MPI_CXX_COMPILE_DEFINITIONS}"
    )

endif()
