/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_PUBLICATION_HPP_
#define HELICS_CPP98_PUBLICATION_HPP_
#pragma once

#include "DataBuffer.hpp"
#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <string>
#include <vector>

namespace helicscpp {
/** C++98 interface for a publication object*/
class Publication {
  public:
    /** construct from a HelicsPublication*/
    explicit Publication(HelicsPublication hpub) HELICS_NOTHROW: pub(hpub) {}
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
    /** cast operator to the underlying HelicsPublication object*/
    operator HelicsPublication() const { return pub; }
    /** return the underlying HelicsPublication object*/
    HelicsPublication baseObject() const { return pub; }

    /** check if the publication is valid */
    bool isValid() const { return (helicsPublicationIsValid(pub) == HELICS_TRUE); }

    /** Methods to publish values **/

    /** publish raw data from a pointer and length*/
    void publish(const char* data, int len)
    {
        // returns helics_status
        helicsPublicationPublishBytes(pub, data, len, HELICS_IGNORE_ERROR);
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

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
    // std::complex is explicitly allowed to alias like this in the standard
#endif
    /** publish a vector of doubles*/
    void publish(const std::vector<std::complex<double> >& data)
    {
        helicsPublicationPublishComplexVector(pub,
                                              reinterpret_cast<const double*>(data.data()),
                                              static_cast<int>(data.size()),
                                              HELICS_IGNORE_ERROR);
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

    /** publish a vector of doubles with adjacent elements making up the real and imaginary parts of
     * a complex number*/
    void publishComplex(const double* data, int length)
    {
        helicsPublicationPublishComplexVector(pub, data, length, HELICS_IGNORE_ERROR);
    }
    /** publish a named point with a string and double*/
    void publish(const std::string& name, double val)
    {
        helicsPublicationPublishNamedPoint(pub, name.c_str(), val, HELICS_IGNORE_ERROR);
    }
    /** publish a boolean value*/
    void publish(bool val)
    {
        helicsPublicationPublishBoolean(pub, val ? HELICS_TRUE : HELICS_FALSE, HELICS_IGNORE_ERROR);
    }
    /** publish a data buffer value*/
    void publish(DataBuffer& buffer)
    {
        helicsPublicationPublishDataBuffer(pub, buffer.getHelicsDataBuffer(), hThrowOnError());
    }
    /** get the name of the publication*/
    const char* getName() const { return helicsPublicationGetName(pub); }
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
    /** get the value of a tag for the publication*/
    const char* getTag(const std::string& tagname) const
    {
        return helicsPublicationGetTag(pub, tagname.c_str());
    }
    /** set the value of a tag for the publication*/
    void setTag(const std::string& tagname, const std::string& tagvalue)
    {
        helicsPublicationSetTag(pub, tagname.c_str(), tagvalue.c_str(), HELICS_IGNORE_ERROR);
    }
    void setOption(int32_t option, int32_t value = 1)
    {
        helicsPublicationSetOption(pub, option, value, HELICS_IGNORE_ERROR);
    }
    int32_t getOption(int32_t option) { return helicsPublicationGetOption(pub, option); }

  private:
    HelicsPublication pub;  //!< the reference to the underlying publication
};

}  // namespace helicscpp
#endif
