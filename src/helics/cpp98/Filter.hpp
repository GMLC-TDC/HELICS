/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_FILTER_HPP_
#define HELICS_CPP98_FILTER_HPP_
#pragma once

#include "../shared_api_library/MessageFilters.h"
#include "helicsExceptions.hpp"
#include <string>

namespace helicscpp
{
/** object managing a filter in the C++98 interface*/
class Filter
{
  public:
    explicit Filter (helics_filter hfilt) : filt (hfilt) {}
    Filter (){};

    Filter (const Filter &filter) : filt (filter.filt) {}

    Filter &operator= (const Filter &filter)
    {
        filt = filter.filt;
        return *this;
    }

    virtual ~Filter (){};
    operator helics_filter () const { return filt; }

    helics_filter baseObject () const { return filt; }

    const char *getName () const { return helicsFilterGetName (filt); }

    void set (const std::string &property, double val)
    {
        helicsFilterSet (filt, property.c_str (), val, hThrowOnError ());
    }
    void setString (const std::string &property, const std::string &val)
    {
        helicsFilterSetString (filt, property.c_str (), val.c_str (), hThrowOnError ());
    }

    /** add a destination target to a cloning filter
    @details all messages going to a destination are copied to the delivery address(es)*/
    void addDestinationTarget (const std::string &dest)
    {
        helicsFilterAddDestinationTarget (filt, dest.c_str (), hThrowOnError ());
    }

    /** add a source target to a cloning filter
    @details all messages coming from a source are copied to the delivery address(es)*/
    void addSourceTarget (const std::string &source)
    {
        helicsFilterAddSourceTarget (filt, source.c_str (), hThrowOnError ());
    }

    /** remove a destination target from a cloning filter*/
    void removeTarget (const std::string &dest)
    {
        helicsFilterRemoveTarget (filt, dest.c_str (), hThrowOnError ());
    }

  protected:
    helics_filter filt;  //!< the reference to the underlying publication
};

class CloningFilter : public Filter
{
  public:
    explicit CloningFilter (helics_filter hfilt) : Filter (hfilt) {}
    CloningFilter (){};

    CloningFilter (const CloningFilter &filter) : Filter (filter) {}

    CloningFilter &operator= (const CloningFilter &filter)
    {
        Filter::operator= (filter);
        return *this;
    }

    /** add a delivery endpoint to a cloning filter
    @details all cloned messages are sent to the delivery address(es)
    */
    void addDeliveryEndpoint (const std::string &deliveryEndpoint)
    {
        helicsFilterAddDeliveryEndpoint (filt, deliveryEndpoint.c_str (), hThrowOnError ());
    }

    /** remove a delivery destination from a cloning filter*/
    void removeDeliveryEndpoint (const std::string &deliveryEndpoint)
    {
        helicsFilterRemoveDeliveryEndpoint (filt, deliveryEndpoint.c_str (), hThrowOnError ());
    }
};

}  // namespace helicscpp
#endif
