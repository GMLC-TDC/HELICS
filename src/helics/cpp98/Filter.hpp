/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_FILTER_HPP_
#define HELICS_CPP98_FILTER_HPP_
#pragma once

#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <string>

namespace helicscpp {
/** object managing a filter in the C++98 interface*/
class Filter {
  public:
    /** construct from C level HelicsFilter object*/
    explicit Filter(HelicsFilter hfilt) HELICS_NOTHROW: filt(hfilt) {}
    /** default constructor*/
    Filter() HELICS_NOTHROW: filt(HELICS_NULL_POINTER) {}
    /** copy constructor*/
    Filter(const Filter& filter): filt(filter.filt) {}
    /** copy assignment*/
    Filter& operator=(const Filter& filter)
    {
        filt = filter.filt;
        return *this;
    }

    /** cast operator to get the underlying object*/
    operator HelicsFilter() const { return filt; }
    /** get the underlying HelicsFilter object*/
    HelicsFilter baseObject() const { return filt; }
    /** check if the filter is valid */
    bool isValid() const { return (helicsFilterIsValid(filt) == HELICS_TRUE); }
    /** get the name for the filter*/
    const char* getName() const { return helicsFilterGetName(filt); }
    /** set a property on a filter
    @param property the name of the property of the filter to change
    @param val the numerical value of the property
    */
    void set(const std::string& property, double val)
    {
        helicsFilterSet(filt, property.c_str(), val, hThrowOnError());
    }
    /** set a string property on a filter
   @param property the name of the property of the filter to change
   @param val the numerical value of the property
   */
    void setString(const std::string& property, const std::string& val)
    {
        helicsFilterSetString(filt, property.c_str(), val.c_str(), hThrowOnError());
    }

    /** add a destination target to a cloning filter
    @details all messages going to a destination are copied to the delivery address(es)*/
    void addDestinationTarget(const std::string& dest)
    {
        helicsFilterAddDestinationTarget(filt, dest.c_str(), hThrowOnError());
    }

    /** add a source target to a cloning filter
    @details all messages coming from a source are copied to the delivery address(es)*/
    void addSourceTarget(const std::string& source)
    {
        helicsFilterAddSourceTarget(filt, source.c_str(), hThrowOnError());
    }

    /** remove a destination target from a cloning filter*/
    void removeTarget(const std::string& dest)
    {
        helicsFilterRemoveTarget(filt, dest.c_str(), hThrowOnError());
    }
    /** get the interface information field of the filter*/
    const char* getInfo() const { return helicsFilterGetInfo(filt); }
    /** set the interface information field of the filter*/
    void setInfo(const std::string& info)
    {
        helicsFilterSetInfo(filt, info.c_str(), HELICS_IGNORE_ERROR);
    }
    /** get the value of a tag for the filter*/
    const char* getTag(const std::string& tagname) const
    {
        return helicsFilterGetTag(filt, tagname.c_str());
    }
    /** set the value of a tag for the filter*/
    void setTag(const std::string& tagname, const std::string& tagvalue)
    {
        helicsFilterSetTag(filt, tagname.c_str(), tagvalue.c_str(), HELICS_IGNORE_ERROR);
    }
    void setOption(int32_t option, int32_t value = 1)
    {
        helicsFilterSetOption(filt, option, value, HELICS_IGNORE_ERROR);
    }
    int32_t getOption(int32_t option) { return helicsFilterGetOption(filt, option); }

    void setCallback(HelicsMessage (*filtCall)(HelicsMessage message, void* userData),
                     void* userData)
    {
        helicsFilterSetCustomCallback(filt, filtCall, userData, HELICS_IGNORE_ERROR);
    }

  protected:
    HelicsFilter filt;  //!< the reference to the underlying publication
};

/** cloning filter extends some operations on filters
 */
class CloningFilter: public Filter {
  public:
    /** construct from underlying filter object*/
    explicit CloningFilter(HelicsFilter hfilt) HELICS_NOTHROW: Filter(hfilt) {}
    /** default constructor*/
    CloningFilter() HELICS_NOTHROW{};
    /** copy constructor*/
    CloningFilter(const CloningFilter& filter): Filter(filter) {}
    /** copy assignment*/
    CloningFilter& operator=(const CloningFilter& filter)
    {
        Filter::operator=(filter);
        return *this;
    }

    /** add a delivery endpoint to a cloning filter
    @details all cloned messages are sent to the delivery address(es)
    */
    void addDeliveryEndpoint(const std::string& deliveryEndpoint)
    {
        helicsFilterAddDeliveryEndpoint(filt, deliveryEndpoint.c_str(), hThrowOnError());
    }

    /** remove a delivery destination from a cloning filter*/
    void removeDeliveryEndpoint(const std::string& deliveryEndpoint)
    {
        helicsFilterRemoveDeliveryEndpoint(filt, deliveryEndpoint.c_str(), hThrowOnError());
    }
};

}  // namespace helicscpp
#endif
