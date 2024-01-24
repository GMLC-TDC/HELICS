/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/**
@file
the purpose of these objects are to convert a specific type into a data block for use in the core
algorithms
*/

#include "../core/SmallBuffer.hpp"
#include "data_view.hpp"
#include "helicsTypes.hpp"
#include "helics_cxx_export.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace helics {

namespace detail {
    constexpr size_t getBinaryLength(double /*val*/)
    {
        return 16;
    }
    constexpr size_t getBinaryLength(std::int64_t /*val*/)
    {
        return 16;
    }
    constexpr size_t getBinaryLength(std::complex<double> /*val*/)
    {
        return 24;
    }
    inline size_t getBinaryLength(std::string_view val)
    {
        return val.size() + 8;
    }
    inline size_t getBinaryLength(const std::vector<double>& val)
    {
        return val.size() * sizeof(double) + 8;
    }
    inline size_t getBinaryLength(const double* /*val*/, size_t size)
    {
        return size * sizeof(double) + 8;
    }

    inline size_t getBinaryLength(const NamedPoint& np)
    {
        return np.name.size() + 16;
    }

    inline size_t getBinaryLength(const std::vector<std::complex<double>>& cv)
    {
        return cv.size() * sizeof(double) * 2 + 8;
    }
    inline size_t getBinaryLength(const std::complex<double>* /*unused*/, size_t size)
    {
        return size * sizeof(double) * 2 + 8;
    }

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, double val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, std::int64_t val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, std::complex<double> val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, std::string_view val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, const NamedPoint& val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, const std::vector<double>& val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data, const double* val, size_t size);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data,
                                             const std::vector<std::complex<double>>& val);

    HELICS_CXX_EXPORT size_t convertToBinary(std::byte* data,
                                             const std::complex<double>* val,
                                             size_t size);

    /** detect the contained data type,  assumes data is at least 1 byte long*/
    HELICS_CXX_EXPORT DataType detectType(const std::byte* data);

    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, double& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, std::int64_t& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, std::complex<double>& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, char* val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, std::string& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, std::string_view& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, NamedPoint& val);

    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, std::vector<double>& val);
    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data, double* val);

    HELICS_CXX_EXPORT void convertFromBinary(const std::byte* data,
                                             std::vector<std::complex<double>>& val);

    /** get the size of the data from the data stream for a specific type
    @details this returns the number of elements of the specific data type  it is NOT in bytes
    */
    HELICS_CXX_EXPORT size_t getDataSize(const std::byte* data);
}  // namespace detail

/** converter for a basic value*/
template<class X>
class ValueConverter {
  public:
    using baseType = X;
    /** convert the value to a block of data*/
    static SmallBuffer convert(const X& val)
    {
        auto dv = SmallBuffer();
        convert(val, dv);
        return dv;
    }

    /** convert the value and store to a specific block of data*/
    static void convert(const X& val, SmallBuffer& store)
    {
        store.resize(detail::getBinaryLength(val));
        detail::convertToBinary(store.data(), val);
    }

    /** convert a raw vector of objects and store to a specific block*/
    static void convert(const X* vals, size_t size, SmallBuffer& store)
    {
        store.resize(detail::getBinaryLength(vals, size));
        detail::convertToBinary(store.data(), vals, size);
    }

    /** convert a raw vector of objects and store to a specific block*/
    static SmallBuffer convert(const X* vals, size_t size)
    {
        auto dv = SmallBuffer();
        convert(vals, size, dv);
        return dv;
    }

    /** interpret a view of the data and convert back to a val*/
    static X interpret(const data_view& block)
    {
        X val;
        detail::convertFromBinary(block.bytes(), val);
        return val;
    }

    /** interpret a view of the data block and store to the specified value*/
    static void interpret(const data_view& block, X& val)
    {
        detail::convertFromBinary(block.bytes(), val);
    }

    /** get the type of the value*/
    static std::string type() { return typeNameString<X>(); }
};

template<>
class ValueConverter<std::vector<std::string>> {
  public:
    using baseType = std::vector<std::string>;
    /** convert the value to a block of data*/
    static SmallBuffer convert(const std::vector<std::string>& val)
    {
        auto dv = SmallBuffer();
        convert(val, dv);
        return dv;
    }

    /** convert the value and store to a specific block of data*/
    static void convert(const std::vector<std::string>& val, SmallBuffer& store);

    /** interpret a view of the data block and store to the specified value*/
    static void interpret(const data_view& block, std::vector<std::string>& val);

    /** interpret a view of the data and convert back to a val*/
    static std::vector<std::string> interpret(const data_view& block)
    {
        std::vector<std::string> val;
        interpret(block, val);
        return val;
    }

    /** get the type of the value*/
    static std::string type() { return "string_vector"; }
};
}  // namespace helics
