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

#include "Message.h"
#include "core/core-data.h"
#include <string>
#include <cstring>
#include <vector>
#include <complex>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace helics
{
	/** template class for generating a known name of a type*/
	template <class X>
	inline std::string typeNameString()
	{
		return std::string("block") + std::to_string(sizeof(X));
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

	const int byte_order_check = 1;
	/** converter for a basic value*/
	template<class X>
	class ValueConverter
	{
	public:
		using baseType = X;
		/** convert the value to a block of data*/
		static data_block convert(const X &val)
		{
			auto dv = data_block(size(val));
			convert(val,dv);
			return dv;
		}
		/** convert the value and store to a specific block of data*/
		static void convert(const X &val, data_block &store)
		{
			store.assign((char *)(&val), sizeof(X));
			store.append((char *)(&byte_order_check), 4);
		}
		/** interpret a view of the data and convert back to a val*/
		static X interpret(const data_view &block)
		{
			if ((block.size() - 4) == sizeof(X))
			{
				return *(reinterpret_cast<const X *>(block.data()));
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** interpret a view of the data block and store to the specified value*/
		static void interpret(const data_view &block, X &val)
		{
			if ((block.size() - 4) == sizeof(X))
			{
				val = *(reinterpret_cast<const X *>(block.data()));
				return;
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** get the type of the value*/
		static std::string type()
		{
			return typeNameString<X>();
		}
		/** get the size of the value*/
		static size_t size(const X &)
		{
			return sizeof(X)+4;
		}
	};
	/** template specialization for char*/
	template<>
	class ValueConverter<char>
	{
	public:
		using baseType = char;
		/** convert the value to a block of data*/
		static data_block convert(char val)
		{
			auto dv = data_block(1,val);
			return dv;
		}
		/** convert the value and store to a specific block of data*/
		static void convert(char val, data_block &store)
		{
			store.assign(&val, 1);
		}
		/** interpret a view of the data and convert back to a val*/
		static char interpret(const data_view &block)
		{
			if (!block.empty())
			{
				return block[0];
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** interpret a view of the data block and store to the specified value*/
		static void interpret(const data_view &block, char &val)
		{
			if (!block.empty())
			{
				val = block[0];
				return;
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** get the type of the value*/
		static std::string type()
		{
			return std::string("char");
		}
		/** get the size of the value*/
		static size_t size(char)
		{
			return 1;
		}
	};


	/** template specialization for unsigned char*/
	template<>
	class ValueConverter<unsigned char>
	{
	public:
		using baseType = unsigned char;
		/** convert the value to a block of data*/
		static data_block convert(unsigned char val)
		{
			auto dv = data_block(1, static_cast<char>(val));
			return dv;
		}
		/** convert the value and store to a specific block of data*/
		static void convert(unsigned char val, data_block &store)
		{
			store.assign(reinterpret_cast<char *>(&val), 1);
		}
		/** interpret a view of the data and convert back to a val*/
		static unsigned char interpret(const data_view &block)
		{
			if (!block.empty())
			{
				return static_cast<unsigned char>(block[0]);
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** interpret a view of the data block and store to the specified value*/
		static void interpret(const data_view &block, unsigned char &val)
		{
			if (!block.empty())
			{
				val = static_cast<unsigned char>(block[0]);
				return;
			}
			throw(std::invalid_argument("block does not match required size"));
		}
		/** get the type of the value*/
		static std::string type()
		{
			return std::string("uchar");
		}
		/** get the size of the value*/
		static size_t size(unsigned char)
		{
			return 1;
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
			val=interpret(block);
		}
		static std::string type()
		{
			return "string";
		}
		static size_t size(const std::string &val)
		{
			return val.length();
		}
	};
	/** converter for a regular vector of basic types */
	template<class X>
	class ValueConverter<std::vector<X>>
	{
	public:
		using baseType = std::vector<X>;

		static data_block convert(const std::vector<X> &val)
		{
			data_block dv(size(val));
			convert(val,dv);
			return dv;
		}

		static void convert(const std::vector<X> &val, data_block &store)
		{
			store.assign(reinterpret_cast<const char *>(val.data()), val.size() * sizeof(X));
			store.append((char *)(&byte_order_check), 4);
		}
		static std::vector<X> interpret(const data_view &block)
		{
			std::vector<X> vals;
			interpret(block, vals);
			return vals;
		}
		static void interpret(const data_view &block, std::vector<X> &val)
		{
			val.resize((block.size()-4) / sizeof(X));
			memcpy(val.data(), block.data(), val.size() * sizeof(X));
		}
		static std::string type()
		{
			return std::string("vector_") +typeNameString<X>();
		}
		static size_t size(const std::vector<X> &val)
		{
			return val.size() * sizeof(X)+4;
		}
	};

	/** converter for a vector of strings
	*/
	template<>
	class ValueConverter<std::vector<std::string>>
	{
	public:
		using baseType = std::vector<std::string >;
		static data_block convert(const std::vector<std::string> &val)
		{
			data_block dv(size(val));
			convert(val, dv);
			return dv;
		}
		static void convert(const std::vector<std::string> &val, data_block &store)
		{
			store.resize(size(val));
			auto sz = static_cast<unsigned short>(val.size());
			store[0] = sz >> 8;
			store[1] = (sz&0xFF);
			unsigned int index = 2;
			for (auto &str : val)
			{
				auto ssz= static_cast<unsigned short>(str.size());
				store[index] = ssz >> 8;
				store[index+1] = (ssz&0xFF); //bit operations
				memcpy(&store[index + 2], str.data(), ssz);
				index += ssz + 2;
			}
		}
		static std::vector<std::string> interpret(const data_view &block)
		{
			std::vector<std::string> strv;
			interpret(block, strv);
			return strv;
		}

		static void interpret(const data_view &block, std::vector<std::string> &val)
		{
			unsigned int vsize = static_cast<unsigned char>(block[0]) * 256 + static_cast<unsigned char>(block[1]);
			val.resize(vsize);
			unsigned int index = 2;
			for (unsigned int ii = 0; ii < vsize; ii++)
			{
				unsigned int nsize = static_cast<unsigned char>(block[index]) * 256 + static_cast<unsigned char>(block[index + 1]);
				index += 2;
				val[ii] = std::string(block.data()+index, nsize);
				index += nsize;
			}
		}

		static std::string type()
		{
			return "string_vector";
		}
		static size_t size(const std::vector<std::string > &val)
		{
			size_t sz = 2+2*val.size();
			for (const auto &str : val)
			{
				sz += str.size();
			}
			return sz;
		}
	};

	/** converter for a vector of data_view object
	*/
	template<>
	class ValueConverter<std::vector<data_view>>
	{
	public:
		using baseType = std::vector<data_view >;
		static data_block convert(const std::vector<data_view> &val)
		{

			data_block dv(size(val));
			convert(val, dv);
			return dv;
		}
		static void convert(const std::vector<data_view> &val, data_block &store)
		{
			store.resize(size(val));
			auto sz = static_cast<unsigned short>(val.size());
			store[0] = sz >> 8;
			store[1] = (sz & 0xFF);
			unsigned int index = 2;
			for (auto &str : val)
			{
				auto ssz = static_cast<unsigned short>(str.size());
				store[index] = ssz >> 8;
				store[index + 1] = (ssz & 0xFF); //bit operations
				memcpy(&store[index + 2], str.data(), ssz);
				index += ssz + 2;
			}
		}
		static std::vector<data_view> interpret(const data_view &block)
		{
			std::vector<data_view> dvv;
			interpret(block, dvv);
			return dvv;
		}

		static void interpret(const data_view &block, std::vector<data_view> &val)
		{
			unsigned int vsize = static_cast<unsigned char>(block[0]) * 256 + static_cast<unsigned char>(block[1]);
			val.resize(vsize);
			unsigned int index = 2;
			for (unsigned int ii = 0; ii < vsize; ii++)
			{
				unsigned int nsize = static_cast<unsigned char>(block[index]) * 256 + static_cast<unsigned char>(block[index + 1]);
				index += 2;
				val[ii] = std::make_shared<data_block>(block.data() + index, nsize);
				index += nsize;
			}
		}

		static std::string type()
		{
			return "block_vector";
		}
		static size_t size(const std::vector<data_view > &val)
		{
			size_t sz = 2 + 2 * val.size();
			for (const auto &dv : val)
			{
				sz += dv.size();
			}
			return sz;
		}
	};


	/** converter for a vector of data_block objects, this types is interchangable with 
	with a vector of data_view objects
	*/
	template<>
	class ValueConverter<std::vector<data_block>>
	{
	public:
		using baseType = std::vector<data_block >;
		static data_block convert(const std::vector<data_block> &val)
		{

			data_block dv(size(val));
			convert(val, dv);
			return dv;
		}
		static void convert(const std::vector<data_block> &val, data_block &store)
		{
			store.resize(size(val));
			auto sz = static_cast<unsigned short>(val.size());
			store[0] = sz >> 8;
			store[1] = (sz & 0xFF);
			unsigned int index = 2;
			for (auto &str : val)
			{
				auto ssz = static_cast<unsigned short>(str.size());
				store[index] = ssz >> 8;
				store[index + 1] = (ssz & 0xFF); //bit operations
				memcpy(&store[index + 2], str.data(), ssz);
				index += ssz + 2;
			}
		}
		static std::vector<data_block> interpret(const data_view &block)
		{
			std::vector<data_block> dvv;
			interpret(block, dvv);
			return dvv;
		}

		static void interpret(const data_view &block, std::vector<data_block> &val)
		{
			unsigned int vsize = static_cast<unsigned char>(block[0]) * 256 + static_cast<unsigned char>(block[1]);
			val.resize(vsize);
			unsigned int index = 2;
			for (unsigned int ii = 0; ii < vsize; ii++)
			{
				unsigned int nsize = static_cast<unsigned char>(block[index]) * 256 + static_cast<unsigned char>(block[index + 1]);
				index += 2;
				val[ii].assign(block.data() + index, nsize);
				index += nsize;
			}
		}

		static std::string type()
		{
			return "block_vector";
		}
		static size_t size(const std::vector<data_block> &val)
		{
			size_t sz = 2 + 2 * val.size();
			for (const auto &dv : val)
			{
				sz += dv.size();
			}
			return sz;
		}
	};
}
#endif
