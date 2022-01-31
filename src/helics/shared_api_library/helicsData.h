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
/** create a helics managed data buffer with initial capacity*/
HELICS_EXPORT HelicsDataBuffer helicsCreateDataBuffer(int32_t initialCapacity);

/** wrap user data in a buffer object*/
HELICS_EXPORT HelicsDataBuffer helicsWrapDataInBuffer(void* data, int dataSize, int dataCapacity);

/** free a DataBuffer */
HELICS_EXPORT void helicsDataBufferFree(HelicsDataBuffer data);

/** get the data buffer size*/
HELICS_EXPORT int32_t helicsDataBufferSize(HelicsDataBuffer data);

/** get the data buffer capacity*/
HELICS_EXPORT int32_t helicsDataBufferCapacity(HelicsDataBuffer data);

/** get a pointer to the raw data*/
HELICS_EXPORT void* helicsDataBufferData(HelicsDataBuffer data);

HELICS_EXPORT HelicsBool helicsDataBufferReserve(HelicsDataBuffer data, int32_t newCapabity);

/** convert an integer to serialized bytes*/
HELICS_EXPORT int32_t helicsIntToBytes(int64_t value, HelicsDataBuffer data);

/** convert a double to serialized bytes*/
HELICS_EXPORT int32_t helicsDoubleToBytes(double value, HelicsDataBuffer data);

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsStringToBytes(const char* str, HelicsDataBuffer data);

/** convert a bool to serialized bytes*/
HELICS_EXPORT int32_t helicsBoolToBytes(HelicsBool value, HelicsDataBuffer data);

/** convert a char to serialized bytes*/
HELICS_EXPORT int32_t helicsCharToBytes(char value, HelicsDataBuffer data);

/** convert a time to serialized bytes*/
HELICS_EXPORT int32_t helicsTimeToBytes(HelicsTime value, HelicsDataBuffer data);

/** convert a complex pair to serialized bytes*/
HELICS_EXPORT int32_t helicsComplexToBytes(double real, double imag, HelicsDataBuffer data);

/** convert a real vector to serialized bytes*/
HELICS_EXPORT int32_t helicsVectorToBytes(const double* value, int dataSize, HelicsDataBuffer data);

/** extract the data type from the data buffer, if the type isn't recognized UNKNOWN is returned*/
HELICS_EXPORT int helicsDataBufferType(HelicsDataBuffer data);

/** convert a data buffer to an int*/
HELICS_EXPORT int64_t helicsDataBufferToInt(HelicsDataBuffer data);

#endif /*HELICS_APISHAREDDATA_FUNCTIONS_H_ */
