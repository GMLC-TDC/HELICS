/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _HELICS_CPP98_EXCEPTIONS_
#define _HELICS_CPP98_EXCEPTIONS_
#pragma once

#include "../shared_api_library/helics.h"
#include "config.hpp"

#include <stdexcept>

namespace helicscpp {
/** defining an exception class for helics errors*/
class HelicsException: public std::runtime_error {
  public:
    /** constructor for the exception
    @param error_code an integer code of the error
    @param error_string a string describing the error
    */
    explicit HelicsException(int error_code, const char* error_string):
        std::runtime_error(error_string), eCode(error_code)
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
    /** this is an implicit conversion operation for use with helics_error*/
    operator helics_error*() { return &eObj; }

  private:
    helics_error eObj;  //!< holder for a helics error object which is used in the C interface
};
}  // namespace helicscpp

#endif /*_HELICS_CPP98_EXCEPTIONS_*/
