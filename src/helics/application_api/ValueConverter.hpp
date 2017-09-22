/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_VALUE_CONVERTER_
#define _HELICS_VALUE_CONVERTER_

#pragma once
/** the purpose of these objects are to convert a specific type into a data block for use in the core algorithms
*/

#include <string>

#include <type_traits>
#include <complex>
#include "core/core.h"
#include "Message.h"

namespace helics
{
	/** template class for generating a known name of a type*/
	template <class X>
	inline std::string typeNameString()
	{
		//this will probably not be the same on all platforms
		return std::string(typeid(X).name());
	}
	template <>
	inline std::string typeNameString <std::vector<std::string>>()
	{
		return "string_vector";
	}
	template <>
	inline std::string typeNameString <std::vector<double>>()
	{
		return "double_vector";
	}
	template <>
	inline std::string typeNameString <std::vector<data_block>>()
	{
		return "block_vector";
	}
	/** for float*/
	template <>
	inline std::string typeNameString<double>()
	{
		return "double";
	}

	/** for float*/
	template <>
	inline std::string typeNameString<float>()
	{
		return "float";
	}
	/** for character*/
	template <>
	inline std::string typeNameString<char>()
	{
		return "char";
	}
	/** for unsigned character*/
	template <>
	inline std::string typeNameString<unsigned char>()
	{
		return "uchar";
	}
	/** for integer*/
	template <>
	inline std::string typeNameString<std::int32_t>()
	{
		return "int32";
	}
	/** for unsigned integer*/
	template <>
	inline std::string typeNameString<std::uint32_t>()
	{
		return "uint32";
	}
	/** for 64 bit unsigned integer*/
	template <>
	inline std::string typeNameString<std::int64_t>()
	{
		return "int64";
	}
	/** for 64 bit unsigned integer*/
	template <>
	inline std::string typeNameString<std::uint64_t>()
	{
		return "uint64";
	}
	/** for complex double*/
	template <>
	inline std::string typeNameString<std::complex<float>>()
	{
		return "complex_f";
	}
	/** for complex double*/
	template <>
	inline std::string typeNameString<std::complex<double>>()
	{
		return "complex";
	}
	template <>
	inline std::string typeNameString<std::string>()
	{
		return "string";
	}

	/** converter for a basic value*/
	template<class X>
	class ValueConverter
	{
	public:
		using baseType = X;
		/** convert the value to a block of data*/
		static data_block convert(const X &val);
		
		/** convert the value and store to a specific block of data*/
		static void convert(const X &val, data_block &store);
	
		/** interpret a view of the data and convert back to a val*/
		static X interpret(const data_view &block);
		
		/** interpret a view of the data block and store to the specified value*/
		static void interpret(const data_view &block, X &val);
		
		/** get the type of the value*/
		static std::string type()
		{
			return typeNameString<X>();
		}
		
	};

	/** converter for a single string value*/
	template<>
	class ValueConverter<std::string>
	{
	public:
		using baseType = std::string;
		static data_block convert(std::string &&val)
		{
			return data_block(std::move(val));
		}
		static data_block convert(const std::string &val)
		{
			return data_block(val);
		}
		static void convert(const std::string &val, data_block &store)
		{
			store = val;
		}
		static std::string interpret(const data_view &block)
		{
			return block.string();
		}
		static void interpret(const data_view &block, std::string &val)
		{
			val = interpret(block);
		}
		static std::string type()
		{
			return "string";
		}
	};
}
#endif
