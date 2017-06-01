/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef PREC_HELPER_H_
#define PREC_HELPER_H_

#include <string>
#include <complex>

enum class valueTypes_t
{
	stringValue,
	doubleValue,
	int64Value,
	complexValue,
	vectorValue,
	unknownValue,
};

valueTypes_t getType(const std::string &typeString);

std::string typeString(valueTypes_t type);

char typeCharacter(valueTypes_t type);

std::string helicsComplexString(double real, double imag);

std::string helicsComplexString(std::complex<double> val);
std::complex<double> helicsGetComplex(const std::string &val);



#endif
