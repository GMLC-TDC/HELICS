/*
Copyright (c) 2017-2020,
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

#include <cstddef>
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

namespace helics::detail {
size_t getBinaryLength(double val);
size_t getBinaryLength(std::int64_t val);
size_t getBinaryLength(std::complex<double> val);
size_t getBinaryLength(std::string_view val);
size_t getBinaryLength(const std::vector<double>& val);
size_t getBinaryLength(const double* val, size_t size);

size_t getBinaryLength(const NamedPoint& np);

size_t getBinaryLength(const std::vector<std::complex<double>>& cv);

size_t convertToBinary(std::byte* data, double val);

size_t convertToBinary(std::byte* data, std::int64_t val);

size_t convertToBinary(std::byte* data, const std::complex<double>& val);

size_t convertToBinary(std::byte* data, std::string_view val);

size_t convertToBinary(std::byte* data, const NamedPoint& val);

size_t convertToBinary(std::byte* data, const std::vector<double>& val);

size_t convertToBinary(std::byte* data, const double* val, size_t size);

size_t convertToBinary(std::byte* data, const std::vector<std::complex<double>>& val);

/** detect the contained data type,  assumes data is at least 1 byte long*/
data_type detectType(const std::byte* data);

void convertFromBinary(const std::byte* data, double& val);
void convertFromBinary(const std::byte* data, std::int64_t& val);
void convertFromBinary(const std::byte* data, std::complex<double>& val);
void convertFromBinary(const std::byte* data, char* val);
void convertFromBinary(const std::byte* data, std::string& val);
void convertFromBinary(const std::byte* data, NamedPoint& val);

void convertFromBinary(const std::byte* data, std::vector<double>& val);
void convertFromBinary(const std::byte* data, double* val);

void convertFromBinary(const std::byte* data, std::vector<std::complex<double>>& val);

/** get the size of the data from the data stream for a specific type
@details this returns the number of elements of the specific data type  it is NOT in bytes
*/
size_t getDataSize(const std::byte* data);
}  // namespace helics::detail
