/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#pragma once

#include <string>
#include <vector>

namespace utilities
{
/** encode a binary sequence to a string*/
std::string base64_encode(unsigned char const *bytes_to_encode, int32_t in_len);

/** decode a string to a vector of unsigned chars*/
std::vector<unsigned char> base64_decode(std::string const& encoded_string, size_t offset = 0);

/** decode a string to a string*/
std::string base64_decode_to_string(std::string const& encoded_string, size_t offset=0);


/** decode a string to the specified memory location*/
size_t base64_decode(std::string const& encoded_string, void *data, size_t max_size);

/** decode a string to a typed vector*/
template<typename vType>
std::vector<vType> base64_decode_type(std::string const& encoded_string)
{
	auto dec = base64_decode(encoded_string);
	std::vector<vType> ret(dec.size() / sizeof(vType));
	memcpy(ret.data(), dec.data(), ret.size() * sizeof(vType));
	return ret;
}

}//namespace utilities

