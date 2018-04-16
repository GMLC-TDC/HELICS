/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_FILTER_HPP_
#define HELICS_CPP98_FILTER_HPP_
#pragma once

#include "../shared_api_library/MessageFilters.h"
#include <string>

namespace helics
{
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

    std::string getTarget () const
    {
        char str[255];
        helicsFilterGetTarget (filt, &str[0], sizeof (str));
        std::string result (str);
        return result;
    }

    std::string getName () const
    {
        char str[255];
        helicsFilterGetName (filt, &str[0], sizeof (str));
        std::string result (str);
        return result;
    }

    void set (const std::string &property, double val) { helicsFilterSet (filt, property.c_str (), val); }
    void setString (const std::string &property, const std::string &val)
    {
        helicsFilterSetString (filt, property.c_str (), val.c_str ());
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

    /** add a destination target to a cloning filter
    @details all messages going to a destination are copied to the delivery address(es)*/
    void addDestinationTarget (const std::string &dest) { helicsFilterAddDestinationTarget (filt, dest.c_str ()); }

    /** add a source target to a cloning filter
    @details all messages coming from a source are copied to the delivery address(es)*/
    void addSourceTarget (const std::string &source) { helicsFilterAddSourceTarget (filt, source.c_str ()); }

    /** add a delivery endpoint to a cloning filter
    @details all cloned messages are sent to the delivery address(es)
    */
    void addDeliveryEndpoint (const std::string &deliveryEndpoint)
    {
        helicsFilterAddDeliveryEndpoint (filt, deliveryEndpoint.c_str ());
    }

    /** remove a destination target from a cloning filter*/
    void removeDestinationTarget (const std::string &dest)
    {
        helicsFilterRemoveDestinationTarget (filt, dest.c_str ());
    }

    /** remove a source target from a cloning filter*/
    void removeSourceTarget (const std::string &source) { helicsFilterRemoveSourceTarget (filt, source.c_str ()); }

    /** remove a delivery destination from a cloning filter*/
    void removeDeliveryEndpoint (const std::string &deliveryEndpoint)
    {
        helicsFilterRemoveDeliveryEndpoint (filt, deliveryEndpoint.c_str ());
    }
};

}  // namespace helics
#endif
