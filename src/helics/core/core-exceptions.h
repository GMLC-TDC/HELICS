/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_EXCEPTIONS_H_
#define HELICS_EXCEPTIONS_H_

#pragma once
#include <stdexcept>
namespace helics
{
class HelicsException : public std::exception
{
public:
	HelicsException() {};
	
};

class invalidIdentifier : public HelicsException
{
public:
	invalidIdentifier() {};

};

class invalidParameter : public HelicsException
{
public:
	invalidParameter() {};

};

class invalidFunctionCall : public HelicsException
{
public:
	invalidFunctionCall() {};

};

class registrationFailure : public HelicsException
{
public:
	registrationFailure() {};

};

class helicsTerminated : public HelicsException
{
public:
	helicsTerminated() {};

};
}

#endif