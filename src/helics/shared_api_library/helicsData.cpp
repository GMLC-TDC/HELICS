/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsData.h"

#include "../application_api/HelicsPrimaryTypes.hpp"


HELICS_EXPORT int32_t helicsIntToBytes(int64_t value, void* data, int datasize)
{
    if (datasize < helics::detail::getBinaryLength(value)) {
        return (-1);
    }
    auto size=helics::detail::convertToBinary(reinterpret_cast<std::byte*>(data), value);
    return static_cast<int32_t>(size);
}

/** convert a double to serialized bytes*/
HELICS_EXPORT int32_t helicsDoubleToBytes(int64_t value, void* data, int datasize) {
    if (datasize < helics::detail::getBinaryLength(value)) {
        return (-1);
    }
    auto size = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(data), value);
    return static_cast<int32_t>(size);
}

/** convert a string to serialized bytes*/
HELICS_EXPORT int32_t helicsStringToBytes(const char* str, void* data, int datasize) {
    if (datasize < helics::detail::getBinaryLength(str)) {
        return (-1);
    }
    auto size = helics::detail::convertToBinary(reinterpret_cast<std::byte*>(data), str);
    return static_cast<int32_t>(size);
}
