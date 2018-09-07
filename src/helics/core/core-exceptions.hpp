/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
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
    HelicsException () = default;
    explicit HelicsException (const std::string &message) : message_ (message) {}
    virtual const char *what () const noexcept override { return message_.c_str (); }
};

/** exception class indicating that a function has failed for some reason*/
class FunctionExecutionFailure : public HelicsException
{
  public:
    explicit FunctionExecutionFailure (const std::string &message = "HELICS execution failure")
        : HelicsException (message){};
};

/** exception for an invalid identification Handle*/
class InvalidIdentifier : public HelicsException
{
  public:
    explicit InvalidIdentifier (const std::string &message = "invalid identifier") : HelicsException (message){};
};

/** exception when one or more of the parameters in the function call were invalid*/
class InvalidParameter : public HelicsException
{
  public:
    explicit InvalidParameter (const std::string &message = "invalid parameter") : HelicsException (message){};
};

/** exception thrown when a function call was made at an inappropriate time*/
class InvalidFunctionCall : public HelicsException
{
  public:
    explicit InvalidFunctionCall (const std::string &message = "invalid function call")
        : HelicsException (message){};
};

/** exception indicating that the registration of an object has failed*/
class ConnectionFailure : public HelicsException
{
  public:
    explicit ConnectionFailure (const std::string &message = "failed to connect") : HelicsException (message){};
};

/** exception indicating that the registration of an object has failed*/
class RegistrationFailure : public HelicsException
{
  public:
    explicit RegistrationFailure (const std::string &message = "registration failure")
        : HelicsException (message){};
};

/** severe exception indicating HELICS has failed and terminated unexpectedly*/
class HelicsSystemFailure : public HelicsException
{
  public:
    explicit HelicsSystemFailure (const std::string &message = "HELICS system failure")
        : HelicsException (message){};
};
}
