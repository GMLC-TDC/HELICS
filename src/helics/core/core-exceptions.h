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
/** base exception class for helics*/
class HelicsException : public std::exception
{
public:
	HelicsException()=default;
	
};

/** exception class indicating that a function has failed for some reason*/
class functionExecutionFailure : public HelicsException
{
public:
	functionExecutionFailure() = default;

};

/** exception for an invalid identification Handle*/
class invalidIdentifier : public HelicsException
{
public:
	invalidIdentifier()=default;

};

/** exception when one or more of the parameters in the function call were invalid*/
class invalidParameter : public HelicsException
{
public:
	invalidParameter()=default;

};

/** exception thrown when a function call was made at an innapropriate time*/
class invalidFunctionCall : public HelicsException
{
public:
	invalidFunctionCall()=default;

};

/** exception indicating that the registration of an object has failed*/
class registrationFailure : public HelicsException
{
public:
	registrationFailure()=default;

};

/** severe exception indicating helics has terminated*/
class helicsTerminated : public HelicsException
{
public:
	helicsTerminated()=default;

};
}

#endif