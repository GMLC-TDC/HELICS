/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef _HELICS_CPP98_EXCEPTIONS_
#define _HELICS_CPP98_EXCEPTIONS_
#pragma once

#include "../shared_api_library/helics.h"
#include "config.hpp"
#include <stdexcept>

namespace helicscpp
{
/** defining an exception class for state transition errors*/
class HelicsException : public std::runtime_error
{
  public:
    explicit HelicsException (int error_code, const char *s) : std::runtime_error (s), eCode (error_code) {}
    int errorCode () const { return eCode; }

  private:
    int eCode;
};

class hThrowOnError
{
  public:
    hThrowOnError () : eObj (helicsErrorInitialize ()) {}
    /** throwing destructor
	@details if the error code object contains a non-zero error code the destructor will emit an exception */
    ~hThrowOnError () THROWS_EXCEPTION
    {
        if (eObj.error_code != 0)
        {
            throw HelicsException (eObj.error_code, eObj.message);
        }
    }
    /** this is an implicit conversion operation for use with helics_error*/
    operator helics_error * () { return &eObj; }

  private:
    helics_error eObj;
};
}  // namespace helicscpp

#endif /*_HELICS_CPP98_EXCEPTIONS_*/
