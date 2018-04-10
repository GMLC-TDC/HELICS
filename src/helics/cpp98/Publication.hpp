/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_PUBLICATION_HPP_
#define HELICS_CPP98_PUBLICATION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"

namespace helics
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

    helics_publication baseObject() const { return pub; }
/** Methods to publish values **/
void publish( const char *data, int len)
{
    // returns helics_status
    helicsPublicationPublishRaw(pub, data, len);
}

void publish( std::string str)
{
    // returns helics_status
    helicsPublicationPublishString(pub, str.c_str());
}

void publish( int64_t val)
{
    // returns helics_status
    helicsPublicationPublishInteger(pub, val);
}

void publish( double val)
{
    // returns helics_status
    helicsPublicationPublishDouble(pub, val);
}

void publish( std::complex<double> cmplx)
{
    // returns helics_status
    helicsPublicationPublishComplex(pub, cmplx.real(), cmplx.imag());
}

void publish(std::vector<double> data)
{
    // c++98 doesn't guarantee vector data will be contiguous
    // might make sense to have a pre-allocated array (that can grow) set aside for reuse
    double *arr = (double*)malloc(data.size() * sizeof(double));
    for (unsigned int i = 0; i < data.size(); i++)
    {
        arr[i] = data[i];
    }
    // returns helics_status
    helicsPublicationPublishVector(pub, arr, static_cast<int>(data.size() * sizeof(double)));
    free(arr);
}



std::string getKey() const
{
    char str[255];
    helicsPublicationGetKey(pub, &str[0], sizeof(str));
    std::string result(str);
    return result;
}


std::string getUnits() const
{
    char str[255];
    helicsPublicationGetUnits(pub, &str[0], sizeof(str));
    std::string result(str);
    return result;
}


std::string getType() const
{
    char str[255];
    helicsPublicationGetType(pub, &str[0], sizeof(str));
    std::string result(str);
    return result;
}

private:
    helics_publication pub;  //!< the reference to the underlying publication
};

}
#endif
