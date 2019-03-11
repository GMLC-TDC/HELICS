/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_PUBLICATION_HPP_
#define HELICS_CPP98_PUBLICATION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "helicsExceptions.hpp"

namespace helicscpp
{
/** C++98 interface for a publication object*/
class Publication
{
  public:
    explicit Publication (helics_publication hpub) : pub (hpub) {}
    Publication (){};

    Publication (const Publication &publication) : pub (publication.pub) {}

    Publication &operator= (const Publication &publication)
    {
        pub = publication.pub;
        return *this;
    }
    // there is no need for move operators since copying the underlying object is fine, it is a non-owning pointer
    operator helics_publication () const { return pub; }

    helics_publication baseObject () const { return pub; }
    /** Methods to publish values **/
    void publish (const char *data, int len)
    {
        // returns helics_status
        helicsPublicationPublishRaw (pub, data, len, NULL);
    }

    void publish (const char *str)
    {
        // returns helics_status
        helicsPublicationPublishString (pub, str, NULL);
    }

    void publish (const std::string &str)
    {
        // returns helics_status
        helicsPublicationPublishString (pub, str.c_str (), NULL);
    }

    void publish (int64_t val)
    {
        // returns helics_status
        helicsPublicationPublishInteger (pub, val, NULL);
    }

    void publish (double val)
    {
        // returns helics_status
        helicsPublicationPublishDouble (pub, val, NULL);
    }

    void publish (std::complex<double> cmplx)
    {
        helicsPublicationPublishComplex (pub, cmplx.real (), cmplx.imag (), NULL);
    }

    void publish (const std::vector<double> &data)
    {
        // returns helics_status
        helicsPublicationPublishVector (pub, data.data (), static_cast<int> (data.size () * sizeof (double)),
                                        NULL);
    }

    void publish (const std::string &name, double val)
    {
        // returns helics_status
        helicsPublicationPublishNamedPoint (pub, name.c_str (), val, NULL);
    }

    void publish (bool val)
    {
        // returns helics_status
        helicsPublicationPublishBoolean (pub, val ? helics_true : helics_false, NULL);
    }

    const char *getKey () const { return helicsPublicationGetKey (pub); }

    const char *getUnits () const { return helicsPublicationGetUnits (pub); }

    const char *getType () const { return helicsPublicationGetType (pub); }

  private:
    helics_publication pub;  //!< the reference to the underlying publication
};

}  // namespace helicscpp
#endif
