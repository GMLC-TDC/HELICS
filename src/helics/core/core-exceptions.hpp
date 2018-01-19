/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_EXCEPTIONS_H_
#define HELICS_EXCEPTIONS_H_

#pragma once
#include <stdexcept>
#include <string>

/** @file
@details definitions of exceptions used in HELICS
*/

namespace helics
{
/** base exception class for helics*/
class HelicsException : public std::exception
{
private:
	std::string message_ = "HELICS EXCEPTION";
public:
	HelicsException()=default;
	HelicsException(const std::string &message) :message_(message)
	{}
	virtual const char *what() const noexcept override
	{
		return message_.c_str();
	}
	
};

/** exception class indicating that a function has failed for some reason*/
class FunctionExecutionFailure : public HelicsException
{
public:
	FunctionExecutionFailure(const std::string &message="HELICS execution failure") :HelicsException(message) {};
};

/** exception for an invalid identification Handle*/
class InvalidIdentifier : public HelicsException
{
public:
	InvalidIdentifier(const std::string &message = "invalid identifier") :HelicsException(message) {};

};

/** exception when one or more of the parameters in the function call were invalid*/
class InvalidParameter : public HelicsException
{
public:
	InvalidParameter(const std::string &message = "invalid parameter") :HelicsException(message) {};

};

/** exception thrown when a function call was made at an inappropriate time*/
class InvalidFunctionCall : public HelicsException
{
public:
	InvalidFunctionCall(const std::string &message = "invalid function call") :HelicsException(message) {};

};

/** exception indicating that the registration of an object has failed*/
class ConnectionFailure : public HelicsException
{
public:
	ConnectionFailure(const std::string &message = "failed to connect") :HelicsException(message) {};

};

/** exception indicating that the registration of an object has failed*/
class RegistrationFailure : public HelicsException
{
public:
	RegistrationFailure(const std::string &message = "registration failure") :HelicsException(message) {};

};

/** severe exception indicating HELICS has terminated*/
class HelicsTerminated : public HelicsException
{
public:
	HelicsTerminated(const std::string &message = "HELICS termination") :HelicsException(message) {};

};
}

#endif