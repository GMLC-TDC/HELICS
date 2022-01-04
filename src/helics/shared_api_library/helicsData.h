/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_APISHAREDDATA_FUNCTIONS_H_
#define HELICS_APISHAREDDATA_FUNCTIONS_H_

#include "api-data.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "helics_export.h"

#ifdef __cplusplus
} /* end of extern "C" { */
#endif
/** convert an integer to serialized bytes*/
HELICS_EXPORT int32_t helicsIntToBytes(int64_t value, void* data, int datasize);

/** convert a double to serialized bytes*/
HELICS_EXPORT int32_t helicsDoubleToBytes(int64_t value, void* data, int datasize);

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsStringToBytes(const char *str, void* data, int datasize);

/** convert a bool to serialized bytes*/
HELICS_EXPORT int32_t helicsBoolToBytes(HelicsBool value, void* data, int datasize);

/** convert a bool to serialized bytes*/
HELICS_EXPORT int32_t helicsCharToBytes(char value, void* data, int datasize);

/** convert a bool to serialized bytes*/
HELICS_EXPORT int32_t helicsTimeToBytes(HelicsTime value, void* data, int datasize);

#endif HELICS_APISHAREDDATA_FUNCTIONS_H_
