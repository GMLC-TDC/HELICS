/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_PUBLICATION_HPP_
#define HELICS_CPP98_PUBLICATION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "helicsExceptions.hpp"

namespace helicscpp
{
class Publication
{
public:
    explicit Publication(helics_publication hpub) :pub(hpub)
    {
    }
    Publication() {};

    Publication(const Publication &publication) :pub(publication.pub)
    {
    }

    Publication &operator=(const Publication &publication)
    {
        pub = publication.pub;
        return *this;
    }
    //there is no need for move operators since copying the underlying object is fine, it is a non-owning pointer
    operator helics_publication() const { return pub; }

    helics_publication baseObject() const { return pub; }
/** Methods to publish values **/
void publish( const char *data, int len)
{
    // returns helics_status
    helicsPublicationPublishRaw(pub, data, len, NULL);
}

void publish(const char *str)
{
    // returns helics_status
    helicsPublicationPublishString (pub, str, NULL);
}

void publish(const std::string &str)
{
    // returns helics_status
    helicsPublicationPublishString(pub, str.c_str(),NULL);
}

void publish( int64_t val)
{
    // returns helics_status
    helicsPublicationPublishInteger(pub, val,NULL);
}

void publish( double val)
{
    // returns helics_status
    helicsPublicationPublishDouble(pub, val,NULL);
}

void publish( std::complex<double> cmplx)
{
    helicsPublicationPublishComplex(pub, cmplx.real(), cmplx.imag(),NULL);
}

void publish(const std::vector<double> &data)
{
    // returns helics_status
    helicsPublicationPublishVector(pub, data.data(), static_cast<int>(data.size() * sizeof(double)),NULL);
}

void publish(const std::string &name, double val)
{
    // returns helics_status
    helicsPublicationPublishNamedPoint(pub, name.c_str(), val,NULL);
}

void publish(bool val)
{
    // returns helics_status
    helicsPublicationPublishBoolean(pub, val?helics_true:helics_false,NULL);
}

std::string getKey() const
{
    char str[255];
    helicsPublicationGetKey(pub, &str[0], sizeof(str),NULL);
    std::string result(str);
    return result;
}


std::string getUnits() const
{
    char str[255];
    helicsPublicationGetUnits (pub, &str[0], sizeof (str), NULL);
    std::string result(str);
    return result;
}


std::string getType() const
{
    char str[255];
    helicsPublicationGetType (pub, &str[0], sizeof (str), NULL);
    std::string result(str);
    return result;
}

private:
    helics_publication pub;  //!< the reference to the underlying publication
};

}
#endif
