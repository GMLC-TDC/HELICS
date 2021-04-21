/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_PUBLICATION_HPP_
#define HELICS_CPP98_PUBLICATION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "helicsExceptions.hpp"

#include <string>
#include <vector>

namespace helicscpp {
/** C++98 interface for a publication object*/
class Publication {
  public:
    /** construct from a helics_publication*/
    explicit Publication(helics_publication hpub) HELICS_NOTHROW: pub(hpub) {}
    /** default constructor*/
    Publication() HELICS_NOTHROW: pub(HELICS_NULL_POINTER) {}
    /** copy constructor*/
    Publication(const Publication& publication): pub(publication.pub) {}
    /** copy assignment*/
    Publication& operator=(const Publication& publication)
    {
        pub = publication.pub;
        return *this;
    }
    // there is no need for move operators since copying the underlying object is fine, it is a
    // non-owning pointer
    /** cast operator to the underlying helics_publication object*/
    operator helics_publication() const { return pub; }
    /** return the underlying helics_publication object*/
    helics_publication baseObject() const { return pub; }

    /** check if the publication is valid */
    bool isValid() const { return (helicsPublicationIsValid(pub) == helics_true); }

    /** Methods to publish values **/

    /** publish raw data from a pointer and length*/
    void publish(const char* data, int len)
    {
        // returns helics_status
        helicsPublicationPublishRaw(pub, data, len, HELICS_IGNORE_ERROR);
    }
    /** publish a string from a char * */
    void publish(const char* str) { helicsPublicationPublishString(pub, str, HELICS_IGNORE_ERROR); }
    /** publish a string value*/
    void publish(const std::string& str)
    {
        helicsPublicationPublishString(pub, str.c_str(), HELICS_IGNORE_ERROR);
    }

    /** publish an int64_t value*/
    void publish(int64_t val)
    {
        // returns helics_status
        helicsPublicationPublishInteger(pub, val, HELICS_IGNORE_ERROR);
    }
    /** publish a double*/
    void publish(double val) { helicsPublicationPublishDouble(pub, val, HELICS_IGNORE_ERROR); }

    /** publish a complex number*/
    void publish(std::complex<double> cmplx)
    {
        helicsPublicationPublishComplex(pub, cmplx.real(), cmplx.imag(), HELICS_IGNORE_ERROR);
    }
    /** publish a vector of doubles*/
    void publish(const std::vector<double>& data)
    {
        helicsPublicationPublishVector(pub,
                                       data.data(),
                                       static_cast<int>(data.size()),
                                       HELICS_IGNORE_ERROR);
    }
    /** publish a vector of doubles*/
    void publish(const double* data, int length)
    {
        helicsPublicationPublishVector(pub, data, length, HELICS_IGNORE_ERROR);
    }
    /** publish a named point with a string and double*/
    void publish(const std::string& name, double val)
    {
        helicsPublicationPublishNamedPoint(pub, name.c_str(), val, HELICS_IGNORE_ERROR);
    }
    /** publish a boolean value*/
    void publish(bool val)
    {
        helicsPublicationPublishBoolean(pub, val ? helics_true : helics_false, HELICS_IGNORE_ERROR);
    }
    /** get the key for the publication*/
    const char* getKey() const { return helicsPublicationGetKey(pub); }
    /** get the units of the publication*/
    const char* getUnits() const { return helicsPublicationGetUnits(pub); }
    /** get the type for the publication*/
    const char* getType() const { return helicsPublicationGetType(pub); }
    /** get the interface information field of the publication*/
    const char* getInfo() const { return helicsPublicationGetInfo(pub); }

    /** set the interface information field of the publication*/
    void setInfo(const std::string& info)
    {
        helicsPublicationSetInfo(pub, info.c_str(), HELICS_IGNORE_ERROR);
    }

  private:
    helics_publication pub;  //!< the reference to the underlying publication
};

}  // namespace helicscpp
#endif
