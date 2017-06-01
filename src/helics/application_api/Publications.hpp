/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_PUBLICATION_H_
#define _HELICS_PUBLICATION_H_
#pragma once

#include "ValueFederate.h"

namespace helics
{
/** enumeration of locality namespaces*/
enum class publication_locality
{
    local,
    global,
};

constexpr publication_locality GLOBAL = publication_locality::global;
constexpr publication_locality LOCAL = publication_locality::local;
/** class to handle a publication */
template <class X>
class Publication
{
  private:
    ValueFederate *fed = nullptr;  //!< the federate construct to interact with
    std::string m_name;  //!< the name of the publication
    std::string m_units;  //!< the defined units of the publication
    publication_id_t id;  //!< the internal id of the publication
  public:
    Publication () noexcept {};
    /**constructor to build a publication object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] units the units associated with a Federate
    */
    Publication (ValueFederate *valueFed, std::string name, std::string units = "")
        : fed (valueFed), m_name (std::move (name)), m_units (std::move (units))
    {
        id = fed->registerPublication<X> (m_name, m_units);
    }
    /**constructor to build a publication object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] units the units associated with a Federate
    */
    Publication (publication_locality locality, ValueFederate *valueFed, std::string name, std::string units = "")
        : fed (valueFed), m_name (std::move (name)), m_units (std::move (units))
    {
        if (locality == GLOBAL)
        {
            id = fed->registerGlobalPublication<X> (m_name, m_units);
        }
        else
        {
            id = fed->registerPublication<X> (m_name, m_units);
        }
    }
    /** send a value for publication
    @param[in] val the value to publish*/
    virtual void publish (const X &val) const { fed->publish (id, val); }
    /** secondary publish function to allow unit conversion before publication
    @param[in] val the value to publish
    @param[in] units  the units association with the publication
    */
    virtual void publish (const X &val, const std::string & /*units*/) const
    {
        // TODO:: figure out units
        publish (val);
    }
};

/** class to handle a publication
but the value is only published in the change is greater than a certain level*/
template <class X>
class PublicationOnChange : public Publication<X>
{
  private:
    X publishDelta;  //!< the delta on which to publish a value
    mutable X prev;  //!< the previous value
  public:
    PublicationOnChange () noexcept {};
    /**constructor to build a publishOnChange object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] minChange  the minimum change required to actually publish the value
    @param[in] units the units associated with a Federate
    */
    PublicationOnChange (ValueFederate *valueFed,
                         const std::string &name,
                         const X &minChange,
                         const std::string &units = "")
        : Publication<X> (valueFed, name, units), publishDelta (minChange)
    {
        prev = X ();
    }
    /** send a value for publication
    @details the value is only published if it exceeds the specified level
    @param[in] val the value to publish*/
    virtual void publish (const X &val) const override
    {
        if (std::abs (val - prev) >= publishDelta)
        {
            prev = val;
            Publication<X>::publish (val);
        }
    }
};

/** class to handle a publication
but the value is only published in the change is greater than a certain level*/
template <>
class PublicationOnChange<std::string> : public Publication<std::string>
{
  private:
    mutable std::string prev;  //!< the previous value
  public:
    PublicationOnChange () noexcept {};
    /**constructor to build a publishOnChange object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] minChange  the minimum change required to actually publish the value
    @param[in] units the units associated with a Federate
    */
    PublicationOnChange (ValueFederate *valueFed, const std::string &name, const std::string &units = "")
        : Publication<std::string> (valueFed, name, units)
    {
    }
    /** send a value for publication
    @details the value is only published if it exceeds the specified level
    @param[in] val the value to publish*/
    virtual void publish (const std::string &val) const override
    {
        if (val != prev)
        {
            prev = val;
            Publication<std::string>::publish (val);
        }
    }
};
}
#endif
