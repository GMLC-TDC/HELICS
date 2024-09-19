/*
Copyright (c) 2017-2024,
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

/** create a new data buffer and copy an existing buffer
@return a data buffer object with a copy of the data
*/
HELICS_EXPORT HelicsDataBuffer helicsDataBufferClone(HelicsDataBuffer data);

/** convert an integer to serialized bytes in a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromInteger(HelicsDataBuffer data, int64_t value);

/** convert a double to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromDouble(HelicsDataBuffer data, double value);

/** convert a string to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromString(HelicsDataBuffer data, const char* value);

/** convert a raw string (may contain nulls) to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param str the value to serialize into the dataBuffer
@param stringSize the size of the string value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromRawString(HelicsDataBuffer data, const char* str, int stringSize);

/** convert a bool to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromBoolean(HelicsDataBuffer data, HelicsBool value);

/** convert a char to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromChar(HelicsDataBuffer data, char value);

/** convert a time to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromTime(HelicsDataBuffer data, HelicsTime value);

/** convert a complex pair to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param real the real part of the imaginary number to serialize into the dataBuffer
* @param imag the imaginary part of the imaginary number to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromComplex(HelicsDataBuffer data, double real, double imag);

/** convert a complex object to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromComplexObject(HelicsDataBuffer data, HelicsComplex value);

/** convert a real vector to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromVector(HelicsDataBuffer data, const double* value, int dataSize);

/** convert a named point to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromNamedPoint(HelicsDataBuffer data, const char* name, double value);

/** convert a complex vector to serialized bytesin a buffer
* @param data the HelicsDataBuffer to fill
* @param value the value to serialize into the dataBuffer
@return the buffer size after filling
*/
HELICS_EXPORT int32_t helicsDataBufferFillFromComplexVector(HelicsDataBuffer data, const double* value, int dataSize);

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
