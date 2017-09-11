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
class functionExecutionFailure : public HelicsException
{
public:
	functionExecutionFailure(const std::string &message="Helics execution failure") :HelicsException(message) {};
};

/** exception for an invalid identification Handle*/
class invalidIdentifier : public HelicsException
{
public:
	invalidIdentifier(const std::string &message = "invalid identifier") :HelicsException(message) {};

};

/** exception when one or more of the parameters in the function call were invalid*/
class invalidParameter : public HelicsException
{
public:
	invalidParameter(const std::string &message = "invalid parameter") :HelicsException(message) {};

};

/** exception thrown when a function call was made at an innapropriate time*/
class invalidFunctionCall : public HelicsException
{
public:
	invalidFunctionCall(const std::string &message = "invalid function call") :HelicsException(message) {};

};

/** exception indicating that the registration of an object has failed*/
class connectionFailure : public HelicsException
{
public:
	connectionFailure(const std::string &message = "failed to connect") :HelicsException(message) {};

};

/** exception indicating that the registration of an object has failed*/
class registrationFailure : public HelicsException
{
public:
	registrationFailure(const std::string &message = "registration failure") :HelicsException(message) {};

};

/** severe exception indicating helics has terminated*/
class helicsTerminated : public HelicsException
{
public:
	helicsTerminated(const std::string &message = "Helics termination") :HelicsException(message) {};

};
}

#endif