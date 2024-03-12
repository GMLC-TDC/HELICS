/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_CPP98_EXCEPTIONS_HPP_
#define HELICS_CPP98_EXCEPTIONS_HPP_
#pragma once

#include "config.hpp"
#include "helics/helics.h"

#include <stdexcept>

namespace helicscpp {
/** defining an exception class for helics errors*/
class HelicsException: public std::runtime_error {
  public:
    /** constructor for the exception
    @param error_code an integer code of the error
    @param errorString a string describing the error
    */
    explicit HelicsException(int error_code, const char* errorString):
        std::runtime_error(errorString), eCode(error_code)
    {
    }
    /** get the error code */
    int errorCode() const { return eCode; }

  private:
    int eCode;  //!< containing the error code value
};

/** helper class that will throw an error if the helics error object has a actual error in it
@details it will throw an in the destructor*/
class hThrowOnError {
  public:
    /** constructor which creates and initializing an error object*/
    hThrowOnError(): eObj(helicsErrorInitialize()) {}
    /** throwing destructor
    @details if the error code object contains a non-zero error code the destructor will emit an
    exception */
    ~hThrowOnError() HELICS_THROWS_EXCEPTION
    {
        if (eObj.error_code != 0) {
            throw HelicsException(eObj.error_code, eObj.message);
        }
    }
    /** this is an implicit conversion operation for use with HelicsError*/
    operator HelicsError*() { return &eObj; }

  private:
    HelicsError eObj;  //!< holder for a helics error object which is used in the C interface
};
}  // namespace helicscpp

#endif /*HELICS_CPP98_EXCEPTIONS_HPP_*/
