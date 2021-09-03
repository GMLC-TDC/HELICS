/*
Copyright (c) 2017-2021,
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

#include "../core/Core.hpp"
#include "data_view.hpp"
#include "helicsTypes.hpp"

#include <string>
#include <utility>
#include <vector>

namespace helics {
/** converter for a basic value*/
template<class X>
class ValueConverter {
  public:
    using baseType = X;
    /** convert the value to a block of data*/
    static data_block convert(const X& val);

    /** convert the value and store to a specific block of data*/
    static void convert(const X& val, data_block& store);

    /** convert a raw vector of objects and store to a specific block*/
    static void convert(const X* vals, size_t size, data_block& store);

    /** convert a raw vector of objects and store to a specific block*/
    static data_block convert(const X* vals, size_t size);

    /** interpret a view of the data and convert back to a val*/
    static X interpret(const data_view& block);

    /** interpret a view of the data block and store to the specified value*/
    static void interpret(const data_view& block, X& val);

    /** get the type of the value*/
    static std::string type() { return typeNameString<X>(); }
};

/** converter for a single string value*/
template<>
class ValueConverter<std::string> {
  public:
    using baseType = std::string;
    static data_block convert(std::string&& val) { return data_block(std::move(val)); }
    static data_block convert(const std::string& val) { return data_block(val); }
    static void convert(const std::string& val, data_block& store) { store = val; }
    static std::string interpret(const data_view& block) { return block.string(); }
    static void interpret(const data_view& block, std::string& val) { val = interpret(block); }
    static std::string type() { return "string"; }
};
}  // namespace helics

// This should be at the end since it depends on the definitions in here
#ifndef HELICS_CXX_STATIC_DEFINE
#    include "ValueConverter_impl.hpp"
#endif

namespace helics {
namespace detail {
    /** detect the contained data type,  assumes data is at least 1 byte long*/
    HELICS_CXX_EXPORT data_type detectType3(const unsigned char* data);

    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, double& val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, std::int64_t& val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, std::complex<double>& val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, char* val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, std::string& val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, NamedPoint& val);

    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, std::vector<double>& val);
    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data, double* val);

    HELICS_CXX_EXPORT void convertFromBinary3(const unsigned char* data,
                                              std::vector<std::complex<double>>& val);

    /** get the size of the data from the data stream for a specific type
    @details this returns the number of elements of the specific data type  it is NOT in bytes
    */
    HELICS_CXX_EXPORT size_t getDataSize3(const unsigned char* data);
}  // namespace detail

/** converter for a basic value*/
template<class X>
class ValueConverter3 {
  public:
    using baseType = X;

    /** interpret a view of the data and convert back to a val*/
    static X interpret(const data_view& block)
    {
        X val;
        if (detail::detectType3(reinterpret_cast<const unsigned char*>(block.data())) ==
            helicsType<X>()) {
            detail::convertFromBinary3(reinterpret_cast<const unsigned char*>(block.data()), val);
        } else {
            ValueConverter<X>::interpret(block, val);
        }

        return val;
    }

    /** interpret a view of the data block and store to the specified value*/
    static void interpret(const data_view& block, X& val)
    {
        if (detail::detectType3(reinterpret_cast<const unsigned char*>(block.data())) ==
            helicsType<X>()) {
            detail::convertFromBinary3(reinterpret_cast<const unsigned char*>(block.data()), val);
        } else {
            ValueConverter<X>::interpret(block, val);
        }
    }

    /** get the type of the value*/
    static std::string type() { return typeNameString<X>(); }
};

}  // namespace helics
