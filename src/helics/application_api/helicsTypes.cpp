/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "helicsTypes.hpp"

namespace helics
{

static const std::string doubleString("double");
static const std::string intString("int64");
static const std::string stringString("string");
static const std::string complexString("complex");
static const std::string doubleVecString("double_vector");
static const std::string nullString("");


const std::string &typeNameStringRef(helicsType_t type)
{
	switch (type)
	{
	case helicsType_t::helicsDouble:
		return doubleString;
	case helicsType_t::helicsInt:
		return intString;
	case helicsType_t::helicsString:
		return stringString;
	case helicsType_t::helicsComplex:
		return complexString;
	case helicsType_t::helicsVector:
		return doubleVecString;
	default:
		return nullString;
	}
}
}