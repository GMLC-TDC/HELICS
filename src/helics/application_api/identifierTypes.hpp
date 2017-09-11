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

constexpr identifier_type invalid_id_value = (identifier_type) (-1);  //!< defining an invalid id value

/** the known types of identifiers*/
enum class identifiers : char
{
	publication,
	subscription,
	filter,
	endpoint,

};
/** class defining an  identifier type
@details  the intent of this class is to limit the operations available on a publication identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a publication_id for a subscription_id or other types of interfaces
*/
template<class BaseType, identifiers ID, BaseType invalidValue=(BaseType(-1))>
class identifier_id_t
{
  private:
    BaseType _value; //!< the underlying index value

  public:
	  static const identifiers identity{ ID }; //!< the type of the identifier
	  using underlyingType = BaseType;
    /** default constructor*/
    constexpr identifier_id_t() noexcept : _value (invalidValue){};
    /** value based constructor*/
    constexpr identifier_id_t(BaseType val) noexcept : _value (val){};
    /** copy constructor*/
    constexpr identifier_id_t(const identifier_id_t &id) noexcept : _value (id._value){};
    /** assignment from number*/
	identifier_id_t &operator= (BaseType val) noexcept
    {
        _value = val;
        return *this;
    };
    /** copy assignment*/
	identifier_id_t &operator= (const identifier_id_t &id) noexcept
    {
        _value = id._value;
        return *this;
    };

    /** get the underlying value*/
    BaseType value () const noexcept { return _value; };
	/** equality operator*/
    bool operator== (identifier_id_t id) const noexcept { return (_value == id._value); };
	/** iequality operator*/
	bool operator!= (identifier_id_t id) const noexcept { return (_value != id._value); };
	/** less than operator for sorting*/
    bool operator< (identifier_id_t id) const noexcept { return (_value < id._value); };
};

using publication_id_t = identifier_id_t<identifier_type, identifiers::publication, invalid_id_value>;
using subscription_id_t= identifier_id_t<identifier_type, identifiers::subscription, invalid_id_value>;
using endpoint_id_t = identifier_id_t<identifier_type, identifiers::endpoint, invalid_id_value>;
using filter_id_t = identifier_id_t<identifier_type, identifiers::filter, invalid_id_value>;

}
#endif
