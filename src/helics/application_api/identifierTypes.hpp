/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef IDENTIFIER_TYPES_H_
#define IDENTIFIER_TYPES_H_
#pragma once

namespace helics
{
using identifier_type = unsigned long;

constexpr identifier_type invalid_id_value = (identifier_type) (-1);  // defining an invalid id value
/** class defining a publication identifier
@details  the intent of this class is to limit the operations available on a publication identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a publication_id for a subscription_id or other types of interfaces
*/
class publication_id_t
{
  private:
    identifier_type _value; //!< the underlying index value

  public:
    /** default constructor*/
    constexpr publication_id_t () noexcept : _value (invalid_id_value){};
    /** value based constructor*/
    constexpr publication_id_t (identifier_type val) noexcept : _value (val){};
    /** copy constructor*/
    constexpr publication_id_t (const publication_id_t &pubid) noexcept : _value (pubid._value){};
    /** assignment from number*/
    publication_id_t &operator= (identifier_type val) noexcept
    {
        _value = val;
        return *this;
    };
    /** copy assignment*/
    publication_id_t &operator= (const publication_id_t &pubid) noexcept
    {
        _value = pubid._value;
        return *this;
    };

    /** get the underlying value*/
    identifier_type value () const noexcept { return _value; };
    bool operator== (publication_id_t pubid) const noexcept { return (_value == pubid._value); };
	bool operator!= (publication_id_t pubid) const noexcept { return (_value != pubid._value); };
    bool operator< (publication_id_t pubid) const noexcept { return (_value < pubid._value); };
};


/** defining a class for subscription identifiers
@details the same as publication_id_t*/
class subscription_id_t
{
  private:
    identifier_type _value;  //!< the underlying value
  public:
    constexpr subscription_id_t () noexcept : _value (invalid_id_value){};
    constexpr subscription_id_t (identifier_type val) noexcept : _value (val){};
    constexpr subscription_id_t (const subscription_id_t &subid) noexcept : _value (subid._value){};
    subscription_id_t &operator= (identifier_type val) noexcept
    {
        _value = val;
        return *this;
    };
    subscription_id_t &operator= (const subscription_id_t &subid) noexcept
    {
        _value = subid._value;
        return *this;
    };
    identifier_type value () const noexcept { return _value; };
    bool operator== (subscription_id_t subid) const noexcept { return (_value == subid._value); };
	bool operator!= (subscription_id_t subid) const noexcept { return (_value != subid._value); };
    bool operator< (subscription_id_t subid) const noexcept { return (_value < subid._value); };
};

/** defining a class for endpoint identifiers
@details the same as publication_id_t*/
class endpoint_id_t
{
  private:
    identifier_type _value;

  public:
    constexpr endpoint_id_t () noexcept : _value (invalid_id_value){};
    constexpr endpoint_id_t (identifier_type val) noexcept : _value (val){};
    constexpr endpoint_id_t (const endpoint_id_t &epid) noexcept : _value (epid._value){};
    endpoint_id_t &operator= (identifier_type val) noexcept
    {
        _value = val;
        return *this;
    };
    endpoint_id_t &operator= (const endpoint_id_t &epid) noexcept
    {
        _value = epid._value;
        return *this;
    };
    identifier_type value () const noexcept { return _value; };
    bool operator== (endpoint_id_t epid) const  noexcept { return (_value == epid._value); };
	bool operator!= (endpoint_id_t epid) const noexcept { return (_value != epid._value); };
    bool operator< (endpoint_id_t epid) const noexcept { return (_value < epid._value); };
};

/** defining a class for filter identifiers
@details the same as publication_id_t*/
class filter_id_t
{
  private:
    identifier_type _value;

  public:
    constexpr filter_id_t () noexcept : _value (invalid_id_value){};
    constexpr filter_id_t (identifier_type val) noexcept : _value (val){};
    constexpr filter_id_t (const filter_id_t &fid) noexcept : _value (fid._value){};
    filter_id_t &operator= (identifier_type val) noexcept
    {
        _value = val;
        return *this;
    };
    filter_id_t &operator= (const filter_id_t &fid) noexcept
    {
        _value = fid._value;
        return *this;
    };
    identifier_type value () const noexcept { return _value; };
    bool operator== (filter_id_t fid) const noexcept{ return (_value == fid._value); };
	bool operator!= (filter_id_t fid) const noexcept { return (_value !=fid._value); };
    bool operator< (filter_id_t fid) const noexcept{ return (_value < fid._value); };
};
}
#endif
