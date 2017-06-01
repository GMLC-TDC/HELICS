/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_CORE_C_
#define _HELICS_CORE_C_

#include <stdint.h>

#include "core-types.h"
/**
 * HELICS Core API exposed with a C interface.
 * 
 * Common types for C/C++ are defined here.
 */
extern "C" {

/* C names should include a namespace prefix */

typedef void* helics_time_t;
typedef void* helics_core_t;



/* Defaults to ns units */
helics_time_t helics_time (uint64_t value);
helics_time_t helics_time_new_unit (uint64_t value, helics_time_unit unit);
void helics_time_set (helics_time_t time, uint64_t value, helics_time_unit unit);
void helics_time_set_unit (helics_time_t time, uint64_t value);
uint64_t helics_time_get (helics_time_t time);
uint64_t helics_time_get_unit (helics_time_t time, helics_time_unit unit);
void helics_time_delete (helics_time_t time);


helics_core_t helics_core_factory_create (helics_core_type type);
int helics_core_factory_available (helics_core_type type);


void helics_core_initialize (helics_core_t core, const char *initializationString);
int helics_core_isInitialized (helics_core_t core);
// ... complete when API is finished.
}

 
#endif /* _HELICS_CORE_C_ */
