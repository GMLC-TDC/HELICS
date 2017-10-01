/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_VALUE_CONVERTER_IMPL_
#define _HELICS_VALUE_CONVERTER_IMPL_

#pragma once
/** the purpose of these objects are to convert a specific type into a data block for use in the core algorithms
 */

#include "Message.h"
#include "core/core-data.h"
#include <algorithm>
#include <cassert>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/vector.hpp>
#include <complex>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
//#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

namespace helics
{
template <class Archive>
void save (Archive &ar, const data_block &db)
{
    ar (db.to_string ());
}

template <class Archive>
void load (Archive &ar, data_block &db)
{
    std::string val;
    ar (val);
    db = std::move (val);
}

template <class Archive>
void save (Archive &ar, const data_view &db)
{
    ar (db.string ());
}

template <class Archive>
void load (Archive &ar, data_view &db)
{
    std::string val;
    ar (val);
    db = data_view (std::move (val));
}

template <class X>
void ValueConverter<X>::convert (const X &val, data_block &store)
{
    std::string data;
    data.reserve (sizeof (store) + 1);
    boost::iostreams::back_insert_device<std::string> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s (inserter);
    archiver oa (s);

    oa (val);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
    store = std::move (data);
}
/** converter for a basic value*/
template <class X>
data_block ValueConverter<X>::convert (const X &val)
{
    auto dv = data_block ();
    convert (val, dv);
    return dv;
}

template <class X>
void ValueConverter<X>::interpret (const data_view &block, X &val)
{
    boost::iostreams::basic_array_source<char> device (block.data (), block.size ());
    boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s (device);
    retriever ia (s);
    try
    {
        ia (val);
    }
    catch (const cereal::Exception &ce)
    {
        throw std::invalid_argument (ce.what ());
    }
}

template <class X>
X ValueConverter<X>::interpret (const data_view &block)
{
    X val;
    interpret (block, val);
    return val;
}
}
#endif
