/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsData.h"

#include "../application_api/HelicsPrimaryTypes.hpp"
#include "../core/SmallBuffer.hpp"
#include "internal/api_objects.h"

#include <algorithm>
#include <string>
#include <vector>

static constexpr int gBufferValidationIdentifier = 0x24EA'663F;

HelicsDataBuffer createAPIDataBuffer(helics::SmallBuffer& buff)
{
    buff.userKey = gBufferValidationIdentifier;

    return static_cast<HelicsDataBuffer>(&buff);
}

HelicsDataBuffer helicsCreateDataBuffer(int32_t initialCapacity)
{
    auto* ptr = new helics::SmallBuffer();
    ptr->userKey = gBufferValidationIdentifier;
    ptr->reserve(initialCapacity);
    return static_cast<HelicsDataBuffer>(ptr);
}

HelicsDataBuffer helicsWrapDataInBuffer(void* data, int dataSize, int dataCapacity)
{
    auto* ptr = new helics::SmallBuffer();
    ptr->userKey = gBufferValidationIdentifier;
    ptr->spanAssign(data, dataSize, dataCapacity);
    ptr->lock(true);
    return static_cast<HelicsDataBuffer>(ptr);
}

helics::SmallBuffer* getBuffer(HelicsDataBuffer data)
{
    auto* ptr = reinterpret_cast<helics::SmallBuffer*>(data);
    if (ptr != nullptr && ptr->userKey == gBufferValidationIdentifier) {
        return ptr;
    }
    auto* message = getMessageObj(data, nullptr);
    if (message != nullptr) {
        return &message->data;
    }
    return nullptr;
}

void helicsDataBufferFree(HelicsDataBuffer data)
{
    auto* ptr = reinterpret_cast<helics::SmallBuffer*>(data);
    if (ptr != nullptr && ptr->userKey == gBufferValidationIdentifier) {
        delete ptr;
    }
}

HelicsBool helicsDataBufferIsValid(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    return (ptr != nullptr) ? HELICS_TRUE : HELICS_FALSE;
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

HelicsDataBuffer helicsDataBufferClone(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return nullptr;
    }
    auto* newptr = new helics::SmallBuffer(*ptr);
    newptr->userKey = gBufferValidationIdentifier;
    return static_cast<HelicsDataBuffer>(newptr);
}

HELICS_EXPORT int32_t helicsDataBufferFillFromInteger(HelicsDataBuffer data, int64_t value)
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
HELICS_EXPORT int32_t helicsDataBufferFillFromDouble(HelicsDataBuffer data, double value)
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
HELICS_EXPORT int32_t helicsDataBufferFillFromString(HelicsDataBuffer data, const char* str)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<std::string_view>::convert(std::string_view(str), *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsDataBufferFillFromRawString(HelicsDataBuffer data, const char* str, int stringSize)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<std::string_view>::convert(std::string_view(str, stringSize), *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int32_t helicsDataBufferFillFromBoolean(HelicsDataBuffer data, HelicsBool value)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    const auto* dataValue = (value == HELICS_FALSE) ? "0" : "1";
    try {
        helics::ValueConverter<std::string>::convert(dataValue, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

/** convert a char to serialized bytes*/
int32_t helicsDataBufferFillFromChar(HelicsDataBuffer data, char value)
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
int32_t helicsDataBufferFillFromTime(HelicsDataBuffer data, HelicsTime value)
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

int32_t helicsDataBufferFillFromComplex(HelicsDataBuffer data, double real, double imag)
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

int32_t helicsDataBufferFillFromNamedPoint(HelicsDataBuffer data, const char* name, double val)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<helics::NamedPoint>::convert(helics::NamedPoint(AS_STRING_VIEW(name), val), *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

int32_t helicsDataBufferFillFromComplexObject(HelicsDataBuffer data, HelicsComplex value)
{
    return helicsDataBufferFillFromComplex(data, value.real, value.imag);
}

int32_t helicsDataBufferFillFromVector(HelicsDataBuffer data, const double* value, int dataSize)
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

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
// std::complex is explicitly allowed to alias like this in the standard
#endif

int32_t helicsDataBufferFillFromComplexVector(HelicsDataBuffer data, const double* value, int dataSize)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    try {
        helics::ValueConverter<std::complex<double>>::convert(reinterpret_cast<const std::complex<double>*>(value), dataSize, *ptr);
        return static_cast<int32_t>(ptr->size());
    }
    catch (const std::bad_alloc&) {
        return 0;
    }
}

#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

int helicsDataBufferType(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_DATA_TYPE_UNKNOWN;
    }
    return static_cast<int>(helics::detail::detectType(ptr->data()));
}

int64_t helicsDataBufferToInteger(HelicsDataBuffer data)
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

HelicsBool helicsDataBufferToBoolean(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_FALSE;
    }
    bool val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return (val) ? HELICS_TRUE : HELICS_FALSE;
}

char helicsDataBufferToChar(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_FALSE;
    }
    char val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return val;
}

HelicsTime helicsDataBufferToTime(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_TIME_INVALID;
    }
    helics::Time val;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), val);
    return static_cast<HelicsTime>(val);
}

HelicsComplex helicsDataBufferToComplexObject(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    HelicsComplex val{HELICS_TIME_INVALID, HELICS_TIME_INVALID};
    if (ptr != nullptr) {
        std::complex<double> v;
        helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);
        val.real = v.real();
        val.imag = v.imag();
    }
    return val;
}

void helicsDataBufferToComplex(HelicsDataBuffer data, double* real, double* imag)
{
    auto* ptr = getBuffer(data);
    if (ptr != nullptr) {
        std::complex<double> v;
        helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);
        if (real != nullptr) {
            *real = v.real();
        }
        if (imag != nullptr) {
            *imag = v.imag();
        }
        return;
    }
    if (real != nullptr) {
        *real = HELICS_TIME_INVALID;
    }
    if (imag != nullptr) {
        *imag = HELICS_TIME_INVALID;
    }
}

int helicsDataBufferStringSize(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    switch (helics::detail::detectType(ptr->data())) {
        case helics::DataType::HELICS_STRING:
            return static_cast<int>(helics::detail::getDataSize(ptr->data())) + 1;
        case helics::DataType::HELICS_CHAR:
            return 1;
        case helics::DataType::HELICS_UNKNOWN:
            return static_cast<int>(ptr->size());
        default: {
            std::string v;
            helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);
            return static_cast<int>(v.size()) + 1;
        } break;
    }
}

void helicsDataBufferToRawString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength)
{
    if ((outputString == nullptr) || (maxStringLen <= 0)) {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
        return;
    }
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
        return;
    }
    std::string v;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);

    auto length = (std::min)(static_cast<int>(v.size()), maxStringLen);
    // NOLINTNEXTLINE
    std::memcpy(outputString, v.data(), length);

    if (actualLength != nullptr) {
        *actualLength = length;
    }
}

void helicsDataBufferToString(HelicsDataBuffer data, char* outputString, int maxStringLen, int* actualLength)
{
    if ((outputString == nullptr) || (maxStringLen <= 0)) {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
        return;
    }
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
        return;
    }
    std::string v;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);

    auto length = (std::min)(static_cast<int>(v.size()), maxStringLen);
    std::memcpy(outputString, v.data(), length);

    // add a null terminator
    if (length >= maxStringLen) {
        --length;
    }
    outputString[length] = '\0';
    if (actualLength != nullptr) {
        *actualLength = length;
    }
}

int helicsDataBufferVectorSize(HelicsDataBuffer data)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return 0;
    }
    switch (helics::detail::detectType(ptr->data())) {
        case helics::DataType::HELICS_VECTOR:
        case helics::DataType::HELICS_COMPLEX_VECTOR:
            return static_cast<int>(helics::detail::getDataSize(ptr->data()));
        case helics::DataType::HELICS_BOOL:
        case helics::DataType::HELICS_INT:
        case helics::DataType::HELICS_DOUBLE:
            return 1;
        case helics::DataType::HELICS_COMPLEX:
            return 2;
        default: {
            std::vector<double> v;
            helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);
            return static_cast<int>(v.size());
        } break;
    }
}

void helicsDataBufferToVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize)
{
    if ((values == nullptr) || (maxlen <= 0)) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }

    std::vector<double> v;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);

    auto length = (std::min)(static_cast<int>(v.size()), maxlen);
    std::memcpy(values, v.data(), length * sizeof(double));
    if (actualSize != nullptr) {
        *actualSize = length;
    }
}

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
// std::complex is explicitly allowed to alias like this in the standard
#endif

void helicsDataBufferToComplexVector(HelicsDataBuffer data, double values[], int maxlen, int* actualSize)
{
    if ((values == nullptr) || (maxlen <= 0)) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }

    std::vector<std::complex<double>> v;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);

    auto length = (std::min)(static_cast<int>(v.size()), maxlen);
    std::memcpy(values, reinterpret_cast<const double*>(v.data()), length * sizeof(double) * 2);

    if (actualSize != nullptr) {
        *actualSize = length;
    }
}

#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

void helicsDataBufferToNamedPoint(HelicsDataBuffer data, char* outputString, int maxStringLength, int* actualLength, double* val)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
        return;
    }

    helics::NamedPoint v;
    helics::valueExtract(helics::data_view(*ptr), helics::detail::detectType(ptr->data()), v);

    if (outputString != nullptr && maxStringLength > 0) {
        auto length = (std::min)(static_cast<int>(v.name.size()), maxStringLength);
        // NOLINTNEXTLINE
        std::memcpy(outputString, reinterpret_cast<const char*>(v.name.data()), length);

        if (actualLength != nullptr) {
            *actualLength = length;
        }
    } else {
        if (actualLength != nullptr) {
            *actualLength = 0;
        }
    }

    if (val != nullptr) {
        *val = v.value;
    }
}

HelicsBool helicsDataBufferConvertToType(HelicsDataBuffer data, int newDataType)
{
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        return HELICS_FALSE;
    }
    auto type = helics::detail::detectType(ptr->data());
    if (static_cast<int>(type) == newDataType) {
        return HELICS_TRUE;
    }
    helics::defV oVal;
    helics::valueExtract(*ptr, helics::DataType::HELICS_ANY, oVal);
    *ptr = helics::typeConvertDefV(static_cast<helics::DataType>(newDataType), oVal);
    return HELICS_TRUE;
}
