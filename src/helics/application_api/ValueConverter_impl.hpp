/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** the purpose of these objects are to convert a specific type into a data block for use in the
 * core algorithms
 */

#include "../core/core-data.hpp"
#include "data_view.hpp"
#include "helicsTypes.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <complex>
#include <cstring>
#include <helics/external/cereal/archives/portable_binary.hpp>
#include <helics/external/cereal/cereal.hpp>
#include <helics/external/cereal/types/complex.hpp>
#include <helics/external/cereal/types/utility.hpp>
#include <helics/external/cereal/types/vector.hpp>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
//#include <helics/external/cereal/archives/binary.hpp>
#include <helics/external/cereal/types/string.hpp>

using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

namespace helics {
template<class Archive>
void save(Archive& ar, const data_block& db)
{
    ar(db.to_string());
}

template<class Archive>
void load(Archive& ar, data_block& db)
{
    std::string val;
    ar(val);
    db = std::move(val);
}

template<class Archive>
void save(Archive& ar, const data_view& db)
{
    ar(db.string());
}

template<class Archive>
void load(Archive& ar, data_view& db)
{
    std::string val;
    ar(val);
    db = data_view{std::move(val)};
}

template<class Archive>
void serialize(Archive& archive, NamedPoint& m)
{
    archive(m.name, m.value);
}

namespace detail {
    // templates for this derived from http://www.voidcn.com/article/p-vjnlygmc-gy.html
    class membuf: public std::streambuf {
      public:
        membuf(const char* buf, size_t size): begin_(buf), end_(buf + size), current_(buf) {}

      private:
        int_type underflow()
        {
            if (current_ == end_) {
                return traits_type::eof();
            }

            return traits_type::to_int_type(*current_);
        }
        int_type uflow()
        {
            if (current_ == end_) {
                return traits_type::eof();
            }

            return traits_type::to_int_type(*current_++);
        }
        int_type pbackfail(int_type ch)
        {
            if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1])) {
                return traits_type::eof();
            }

            return traits_type::to_int_type(*--current_);
        }
        std::streamsize showmanyc() { return end_ - current_; }

      public:
        // copy ctor and assignment not implemented;
        // copying not allowed
        membuf(const membuf&) = delete;
        membuf& operator=(const membuf&) = delete;

      private:
        const char* const begin_;
        const char* const end_;
        const char* current_;
    };

    struct imemstream: virtual membuf, std::istream {
        imemstream(const char* buf, size_t size):
            membuf(buf, size), std::istream(static_cast<std::streambuf*>(this))
        {
        }
    };

    /** class to create a stream directly to a string that can be extracted*/
    class ostringbuf: public std::streambuf {
      public:
        ostringbuf()
        {
            char* base = abuf_.data();
            setp(base, base + bufsize - 1);  // one less than the buffer size
        }
        /** reserve a size of the buffer*/
        void reserve(size_t size) { sbuf_.reserve(size); }
        /** extract the string in the buffer and reset the string buffer*/
        std::string extractString()
        {
            std::string retString(std::move(sbuf_));
            sbuf_.clear();
            return retString;
        }

      protected:
        void move_to_string_and_flush()
        {
            sbuf_.append(pbase(), pptr());
            std::ptrdiff_t n = pptr() - pbase();
            pbump(static_cast<int>(-n));
        }

      private:
        int_type overflow(int_type ch)
        {
            if (ch != traits_type::eof()) {
                *pptr() = static_cast<char>(ch);
                pbump(1);  // always safe due to buffer at 1 space reserved
                move_to_string_and_flush();
                return ch;
            }

            return traits_type::eof();
        }
        int sync()
        {
            move_to_string_and_flush();
            return 0;
        }

        // copy ctor and assignment not implemented;
        // copying not allowed
        ostringbuf(const ostringbuf&) = delete;
        ostringbuf& operator=(const ostringbuf&) = delete;

      private:
        static constexpr size_t bufsize = 64;
        std::array<char, bufsize> abuf_;
        std::string sbuf_;
    };

    struct ostringbufstream: virtual ostringbuf, std::ostream {
        ostringbufstream(): std::ostream(static_cast<std::streambuf*>(this)) {}
    };

    struct ostreambuf: public std::streambuf {
        ostreambuf(char* buf, size_t size) { this->setg(buf, buf, buf + size); }
    };
}  // namespace detail

template<class X>
void ValueConverter<X>::convert(const X& val, data_block& store)
{
    detail::ostringbufstream s;
    archiver oa(s);

    oa(val);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
    store = s.extractString();
}

template<class X>
void ValueConverter<X>::convert(const X* vals, size_t size, data_block& store)
{
    detail::ostringbufstream s;
    archiver oa(s);
    oa(cereal::make_size_tag(static_cast<cereal::size_type>(size)));  // number of elements
    for (size_t ii = 0; ii < size; ++ii) {
        oa(vals[ii]);
    }
    // don't forget to flush the stream to finish writing into the buffer
    s.flush();
    // store = std::move (data);
    store = s.extractString();
}

/** template trait for figuring out if something is a vector of objects*/
template<typename T, typename _ = void>
struct is_vector {
    static constexpr bool value = false;
};
template<typename T>
struct is_vector<
    T,
    typename std::enable_if_t<
        std::is_same<T, std::vector<typename T::value_type, typename T::allocator_type>>::value>> {
    static constexpr bool value = true;
};

/** template trait for figuring out if something is an iterable container*/
template<typename T, typename _ = void>
struct is_iterable {
    static constexpr bool value = false;
};

template<typename T>
struct is_iterable<T,
                   typename std::enable_if_t<std::is_same<
                       decltype(std::begin(T()) != std::end(T()),  // begin/end and operator != and
                                                                   // has default constructor
                                void(),
                                void(*std::begin(T())),  // dereference operator
                                std::true_type{}),
                       std::true_type>::value>> {
    static constexpr bool value = true;
};

/** for non iterable classes and not strings*/
template<class X>
constexpr std::enable_if_t<!is_iterable<X>::value && !std::is_convertible<X, std::string>::value,
                           size_t>
    getMinSize()
{
    return sizeof(X) + 1;
}

/** for class that are iterable and not strings like vector*/
template<class X>
constexpr std::enable_if_t<is_iterable<X>::value && !std::is_convertible<X, std::string>::value,
                           size_t>
    getMinSize()
{
    return 9;
}

template<class X>
constexpr std::enable_if_t<std::is_convertible<X, std::string>::value, size_t> getMinSize()
{
    return 0;
}

/** min size for a named point*/
template<>
constexpr size_t getMinSize<NamedPoint>()
{
    return 10;
}

template<class X>
data_block ValueConverter<X>::convert(const X* vals, size_t size)
{
    auto dv = data_block();
    convert(vals, size, dv);
    return dv;
}

/** converter for a basic value*/
template<class X>
data_block ValueConverter<X>::convert(const X& val)
{
    auto dv = data_block();
    convert(val, dv);
    return dv;
}

template<class X>
void ValueConverter<X>::interpret(const data_view& block, X& val)
{
    if (block.size() < getMinSize<X>()) {
        std::string arg = std::string("invalid data size: expected ") +
            std::to_string(getMinSize<X>()) + ", received " + std::to_string(block.size());
        throw std::invalid_argument(arg);
    }
    detail::imemstream s(block.data(), block.size());
    retriever ia(s);
    try {
        ia(val);
    }
    catch (const cereal::Exception& ce) {
        throw std::invalid_argument(ce.what());
    }
}

template<class X>
X ValueConverter<X>::interpret(const data_view& block)
{
    X val;
    interpret(block, val);
    return val;
}
}  // namespace helics
