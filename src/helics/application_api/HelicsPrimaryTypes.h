/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_PRIMARY_TYPES_H_
#define HELICS_PRIMARY_TYPES_H_
#pragma once

#include <boost/variant.hpp>
#include <string>
#include <complex>
#include <vector>

namespace helics
{
	using defV = boost::variant<std::string, double, int64_t, std::complex<double>, std::vector<double>>;

	enum typeLocation
	{
		stringLoc = 0,
		doubleLoc = 1,
		intLoc = 2,
		complexLoc = 3,
		vectorLoc = 4,
	};
	/** enumeration of locality namespaces*/
	enum class publication_locality
	{
		local,
		global,
	};

	constexpr publication_locality GLOBAL = publication_locality::global;
	constexpr publication_locality LOCAL = publication_locality::local;

} //namespace helics

#endif //HELICS_PRIMARY_TYPES_H_

