# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# modifyOctSourceFile.cmake

if (WIN32)
    if(helicsOCTAVE_wrap.cxx IS_NEWER_THAN helicsOCTAVE_wrap.cpp)
        if(VOID_SIZE EQUAL 8)
            file(READ helicsOCTAVE_wrap.cxx HELICS_OCT_SOURCE)

string(REPLACE "long swig_this\(\) const"
       "long long swig_this\(\)" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")
string(REPLACE "return \(long\) this"
       "return \(long long\) this" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")
string(REPLACE "\(long\) types[0].second.ptr"
       "\(long long\) types[0].second.ptr" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")

            file(WRITE helicsOCTAVE_wrap.cpp "${HELICS_OCT_SOURCE}")
            set(FILE_WRITTEN TRUE)
        endif()
    endif()
endif(WIN32)

if (NOT FILE_WRITTEN)
    configure_file(helicsOCTAVE_wrap.cxx helicsOCTAVE_wrap.cpp COPYONLY)
endif()
