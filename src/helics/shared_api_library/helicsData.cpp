/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsData.h"

#include "../application_api/HelicsPrimaryTypes.hpp"
#include "../core/SmallBuffer.hpp"

#include <string>

static const int bufferValidationIdentifier = 0x24EA'663F;

HelicsDataBuffer helicsCreateDataBuffer(int32_t initialCapacity)
{
    auto* ptr = new helics::SmallBuffer();
    ptr->userKey = bufferValidationIdentifier;
    ptr->reserve(initialCapacity);
    return static_cast<HelicsDataBuffer>(ptr);
}

HelicsDataBuffer helicsWrapDataInBuffer(void* data, int dataSize, int dataCapacity)
{
    auto* ptr = new helics::SmallBuffer();
    ptr->userKey = bufferValidationIdentifier;
    ptr->spanAssign(data, dataSize, dataCapacity);
    ptr->lock(true);
    return static_cast<HelicsDataBuffer>(ptr);
}

static helics::SmallBuffer* getBuffer(HelicsDataBuffer data)
{
    helics::SmallBuffer* ptr = reinterpret_cast<helics::SmallBuffer*>(data);
    return (ptr != nullptr && ptr->userKey == bufferValidationIdentifier) ? ptr : nullptr;
}

void helicsDataBufferFree(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr != nullptr) {
        delete ptr;
    }
}

int32_t helicsDataBufferSize(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    return (ptr != nullptr) ? static_cast<int32_t>(ptr->size()) : 0;
}

int32_t helicsDataBufferCapacity(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    return (ptr != nullptr) ? static_cast<int32_t>(ptr->capacity()) : 0;
}

void* helicsDataBufferData(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    return (ptr != nullptr) ? static_cast<void*>(ptr->data()) : nullptr;
}

HelicsBool helicsDataBufferReserve(HelicsDataBuffer data, int32_t newCapacity)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_FALSE;
    }
    try {
        ptr->reserve(newCapacity);
        return HELICS_TRUE;
    }
    catch (const std::bad_alloc&) {
        return HELICS_FALSE;
    }
}

HELICS_EXPORT int32_t helicsIntToBytes(int64_t value, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<int64_t>::convert(value, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a double to serialized bytes*/
HELICS_EXPORT int32_t helicsDoubleToBytes(double value, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<double>::convert(value, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsStringToBytes(const char* str, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<std::string>::convert(str, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int32_t helicsBoolToBytes(HelicsBool value, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    auto dataValue = (value == HELICS_FALSE) ? "0" : "1";
    try {
        helics::ValueConverter<std::string>::convert(dataValue, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a char to serialized bytes*/
int32_t helicsCharToBytes(char value, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<int64_t>::convert(value, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a HelicsTime to serialized bytes*/
int32_t helicsTimeToBytes(HelicsTime value, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<int64_t>::convert(helics::Time(value).getBaseTimeCode(), *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int32_t helicsComplexToBytes(double real, double imag, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<std::complex<double>>::convert(std::complex<double>(real, imag), *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int32_t helicsVectorToBytes(const double* value, int dataSize, HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<double>::convert(value, dataSize, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int helicsDataBufferType(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_DATA_TYPE_UNKNOWN;
    }
    return static_cast<int>(helics::detail::detectType(ptr->data()));
}

int64_t helicsDataBufferToInt(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return helics::invalidValue<int64_t>();
    }
    int64_t val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return val;
}

double helicsDataBufferToDouble(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return helics::invalidValue<double>();
    }
    double val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return val;
}

HelicsBool helicsDataBufferToBool(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_FALSE;
    }
    bool val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return val;
}
