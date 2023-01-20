/*
Copyright (c) 2017-2023,
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

/** create a helics managed data buffer with initial capacity*/
HELICS_EXPORT HelicsDataBuffer helicsCreateDataBuffer(int32_t initialCapacity);

/** check whether a buffer is valid*/
HELICS_EXPORT HelicsBool helicsDataBufferIsValid(HelicsDataBuffer data);

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

/** increase the capacity a data buffer can hold without reallocating memory
@return HELICS_TRUE if the reservation was successful HELICS_FALSE otherwise*/
HELICS_EXPORT HelicsBool helicsDataBufferReserve(HelicsDataBuffer data, int32_t newCapacity);

/** create a new data buffer and copy an existing buffer*/
HELICS_EXPORT HelicsDataBuffer helicsDataBufferClone(HelicsDataBuffer data);

/** convert an integer to serialized bytes*/
HELICS_EXPORT int32_t helicsIntegerToBytes(int64_t value, HelicsDataBuffer data);

/** convert a double to serialized bytes*/
HELICS_EXPORT int32_t helicsDoubleToBytes(double value, HelicsDataBuffer data);

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsStringToBytes(const char* value, HelicsDataBuffer data);

/** convert a raw string (may contain nulls) to serialized bytes*/
HELICS_EXPORT int32_t helicsRawStringToBytes(const char* str, int stringSize, HelicsDataBuffer data);

/** convert a bool to serialized bytes*/
HELICS_EXPORT int32_t helicsBooleanToBytes(HelicsBool value, HelicsDataBuffer data);

/** convert a char to serialized bytes*/
HELICS_EXPORT int32_t helicsCharToBytes(char value, HelicsDataBuffer data);

/** convert a time to serialized bytes*/
HELICS_EXPORT int32_t helicsTimeToBytes(HelicsTime value, HelicsDataBuffer data);

/** convert a complex pair to serialized bytes*/
HELICS_EXPORT int32_t helicsComplexToBytes(double real, double imag, HelicsDataBuffer data);

/** convert a complex object to serialized bytes*/
HELICS_EXPORT int32_t helicsComplexObjectToBytes(HelicsComplex value, HelicsDataBuffer data);

/** convert a real vector to serialized bytes*/
HELICS_EXPORT int32_t helicsVectorToBytes(const double* value, int dataSize, HelicsDataBuffer data);

/** convert a named point to serialized bytes*/
HELICS_EXPORT int32_t helicsNamedPointToBytes(const char* name, double value, HelicsDataBuffer data);

/** convert a complex vector to serialized bytes*/
HELICS_EXPORT int32_t helicsComplexVectorToBytes(const double* value, int dataSize, HelicsDataBuffer data);

/** extract the data type from the data buffer, if the type isn't recognized UNKNOWN is returned*/
HELICS_EXPORT int helicsDataBufferType(HelicsDataBuffer data);

/** convert a data buffer to an int*/
HELICS_EXPORT int64_t helicsDataBufferToInteger(HelicsDataBuffer data);

/** convert a data buffer to a double*/
HELICS_EXPORT double helicsDataBufferToDouble(HelicsDataBuffer data);

/** convert a data buffer to a boolean*/
HELICS_EXPORT HelicsBool helicsDataBufferToBoolean(HelicsDataBuffer data);

/** convert a data buffer to a char*/
HELICS_EXPORT char helicsDataBufferToChar(HelicsDataBuffer data);

/** get the size of memory required to retrieve a string from a data buffer this includes space for a null terminator*/
HELICS_EXPORT int helicsDataBufferStringSize(HelicsDataBuffer data);

/* convert a data buffer to a string including a null terminator
NOTE:  data may contain null bytes prior to the end but actualLength will be the number of characters in the string
outputString[actualLength] is a null terminator*/
HELICS_EXPORT void helicsDataBufferToString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength);

/* convert a data buffer to a Raw string with no null terminator
 */
HELICS_EXPORT void helicsDataBufferToRawString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength);

/** convert a data buffer to a time*/
HELICS_EXPORT HelicsTime helicsDataBufferToTime(HelicsDataBuffer data);

/** convert a data buffer to a complex object*/
HELICS_EXPORT HelicsComplex helicsDataBufferToComplexObject(HelicsDataBuffer data);

/** convert a data buffer to complex values*/
HELICS_EXPORT void helicsDataBufferToComplex(HelicsDataBuffer data, double* real, double* imag);

/** get the number of elements that would be required if a vector were retrieved*/
HELICS_EXPORT int helicsDataBufferVectorSize(HelicsDataBuffer data);

/** convert a data buffer to double vector values*/
HELICS_EXPORT void helicsDataBufferToVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize);

/** convert a data buffer to complex double vector values
@param data the buffer containing data
@param values the storage for the converted data
@param maxlen the number of complex values that the values vector can hold
@param actualSize the number of complex values copied to values array
*/
HELICS_EXPORT void helicsDataBufferToComplexVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize);

HELICS_EXPORT void
    helicsDataBufferToNamedPoint(HelicsDataBuffer data, char* outputString, int maxStringLength, int* actualLength, double* val);

/** convert the data in a data buffer to a different type representation
@param data the buffer to convert
@param newDataType the type that it is desired for the buffer to be converted to
@return true if the conversion was successful*/
HELICS_EXPORT HelicsBool helicsDataBufferConvertToType(HelicsDataBuffer data, int newDataType);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHAREDDATA_FUNCTIONS_H_ */
