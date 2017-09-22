##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
IF(UNIX)
  add_compile_options(-fvisibility=hidden)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-fvisibility=hidden)
  ENDIF(MINGW)
ENDIF(UNIX)

set(helicsShared_headers
helics.h
api-data.h
)

set(helicsShared_sources
helicsExport.cpp
)


add_library(helicsSharedLib SHARED ${helicsShared_sources} ${helicsShared_headers})



target_link_libraries(helicsSharedLib helics)
link_directories(${external_link_directories})
target_link_libraries(helicsSharedLib ${external_library_list})


INSTALL(TARGETS helicsSharedLib DESTINATION lib)


