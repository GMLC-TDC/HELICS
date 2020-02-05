/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <stdexcept>
#include <string>

/** @file
@details definitions of exceptions used in HELICS
*/

namespace helics {
/** base exception class for helics*/
class HelicsException: public std::exception {
  private:
    std::string errorMessage = "HELICS EXCEPTION";

  public:
    HelicsException() = default;
    explicit HelicsException(std::string message) noexcept: errorMessage(std::move(message)) {}
    virtual const char* what() const noexcept override { return errorMessage.c_str(); }
};

/** exception class indicating that a function has failed for some reason*/
class FunctionExecutionFailure: public HelicsException {
  public:
    explicit FunctionExecutionFailure(std::string message = "HELICS execution failure") noexcept:
        HelicsException(std::move(message)){};
};

/** exception for an invalid identification Handle*/
class InvalidIdentifier: public HelicsException {
  public:
    explicit InvalidIdentifier(std::string message = "invalid identifier") noexcept:
        HelicsException(std::move(message)){};
};

/** exception when one or more of the parameters in the function call were invalid*/
class InvalidParameter: public HelicsException {
  public:
    explicit InvalidParameter(std::string message = "invalid parameter") noexcept:
        HelicsException(std::move(message)){};
};

/** exception when a requested conversion is unable to be performed*/
class InvalidConversion: public HelicsException {
  public:
    explicit InvalidConversion(
        std::string message = "unable to perform the requested conversion") noexcept:
        HelicsException(std::move(message)){};
};

/** exception thrown when a function call was made at an inappropriate time*/
class InvalidFunctionCall: public HelicsException {
  public:
    explicit InvalidFunctionCall(std::string message = "invalid function call") noexcept:
        HelicsException(std::move(message)){};
};

/** exception indicating that the connections of an object or network have failed*/
class ConnectionFailure: public HelicsException {
  public:
    explicit ConnectionFailure(std::string message = "failed to connect") noexcept:
        HelicsException(std::move(message)){};
};

/** exception indicating that the registration of an object has failed*/
class RegistrationFailure: public HelicsException {
  public:
    explicit RegistrationFailure(std::string message = "registration failure") noexcept:
        HelicsException(std::move(message)){};
};

/** severe exception indicating HELICS has failed and terminated unexpectedly*/
class HelicsSystemFailure: public HelicsException {
  public:
    explicit HelicsSystemFailure(std::string message = "HELICS system failure") noexcept:
        HelicsException(std::move(message)){};
};
} // namespace helics
