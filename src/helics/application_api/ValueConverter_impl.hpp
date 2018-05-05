/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

/** the purpose of these objects are to convert a specific type into a data block for use in the core algorithms
 */

#include "../core/core-data.hpp"
#include "data_view.hpp"
#include "helicsTypes.hpp"
#include <algorithm>
#include <cassert>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <complex>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
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


template<class Archive>
void serialize(Archive & archive,
    named_point & m)
{
    archive(m.name, m.value);
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

template <class X>
void ValueConverter<X>::convert (const X *vals, size_t size, data_block &store)
{
    std::string data;
    data.reserve (sizeof (store) + 1);
    boost::iostreams::back_insert_device<std::string> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s (inserter);
    archiver oa (s);
    oa (cereal::make_size_tag (size));  // number of elements
    for (size_t ii = 0; ii < size; ++ii)
    {
        oa (vals[ii]);
    }
    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
    store = std::move (data);
}

/** template trait for figuring out if something is a vector of objects*/
template <typename T, typename _ = void>
struct is_vector
{
    static constexpr bool value = false;
};
template <typename T>
struct is_vector<T,
                 typename std::enable_if_t<
                   std::is_same<T, std::vector<typename T::value_type, typename T::allocator_type>>::value>>
{
    static constexpr bool value = true;
};

/** template trait for figuring out if something is an iterable container*/
template <typename T, typename _ = void>
struct is_iterable
{
    static constexpr bool value = false;
};

template <typename T>
struct is_iterable<
  T,
  typename std::enable_if_t<std::is_same<
    decltype (std::begin (T ()) != std::end (T ()),  // begin/end and operator != and has default constructor
              void(),
              void(*std::begin (T ())),  // dereference operator
              std::true_type{}),
    std::true_type>::value>>
{
    static constexpr bool value = true;
};

template <class X>
constexpr std::enable_if_t<!is_iterable<X>::value && !std::is_convertible<X, std::string>::value, size_t>
getMinSize ()
{
    return sizeof (X) + 1;
}

template <class X>
constexpr std::enable_if_t<is_iterable<X>::value && !std::is_convertible<X, std::string>::value, size_t>
getMinSize ()
{
    return 9;
}

template <class X>
constexpr std::enable_if_t<std::is_convertible<X, std::string>::value, size_t> getMinSize ()
{
    return 0;
}

/** min size for a named point*/
template<>
constexpr size_t getMinSize<named_point>()
{
    return 10;
}

template <class X>
data_block ValueConverter<X>::convert (const X *vals, size_t size)
{
    auto dv = data_block ();
    convert (vals, size, dv);
    return dv;
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
    if (block.size () < getMinSize<X> ())
    {
        throw std::invalid_argument ("invalid data size");
    }
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
}  // namespace helics
