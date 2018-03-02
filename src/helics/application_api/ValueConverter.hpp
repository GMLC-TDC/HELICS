/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

/**
@file
the purpose of these objects are to convert a specific type into a data block for use in the core algorithms
*/

#include "../core/Core.hpp"
#include "data_view.hpp"
#include <complex>
#include <type_traits>

namespace helics
{
/** converter for a basic value*/
template <class X>
class ValueConverter
{
  public:
    using baseType = X;
    /** convert the value to a block of data*/
    static data_block convert (const X &val);

    /** convert the value and store to a specific block of data*/
    static void convert (const X &val, data_block &store);

    /** convert a raw vector of objects and store to a specific block*/
    static void convert (const X *vals, size_t size, data_block &store);

    /** convert a raw vector of objects and store to a specific block*/
    static data_block convert (const X *vals, size_t size);

    /** interpret a view of the data and convert back to a val*/
    static X interpret (const data_view &block);

    /** interpret a view of the data block and store to the specified value*/
    static void interpret (const data_view &block, X &val);

    /** get the type of the value*/
    static std::string type () { return typeNameString<X> (); }
};

/** converter for a single string value*/
template <>
class ValueConverter<std::string>
{
  public:
    using baseType = std::string;
    static data_block convert (std::string &&val) { return data_block (std::move (val)); }
    static data_block convert (const std::string &val) { return data_block (val); }
    static void convert (const std::string &val, data_block &store) { store = val; }
    static std::string interpret (const data_view &block) { return block.string (); }
    static void interpret (const data_view &block, std::string &val) { val = interpret (block); }
    static std::string type () { return "string"; }
};
}  // namespace helics
