/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_PUBLICATION_H_
#define _HELICS_PUBLICATION_H_
#pragma once

#include "ValueFederate.h"
#include "helicsTypes.hpp"
#include "HelicsPrimaryTypes.h"
#include "boost/lexical_cast.hpp"



namespace helics
{


class Publication
{
private:
	ValueFederate *fed = nullptr;  //!< the federate construct to interact with
	publication_id_t id;  //!< the internal id of the publication
	double delta = -1.0;  //!< the minimum change to publish
	helicsType_t type; //!< the type of publication
	bool changeDetectionEnabled = false; //!< the change detection is enabled
	std::string m_name;  //!< the name of the publication
	std::string m_units;  //!< the defined units of the publication
	mutable defV prevValue;  //!< the previous value of the publication
public:
	Publication() noexcept {};
	/**constructor to build a publication object
	@param[in] valueFed  the ValueFederate to use
	@param[in] name the name of the subscription
	@param[in] units the units associated with a Federate
	*/
	template<class X, typename std::enable_if<helicsType<X>() != helicsType_t::helicsInvalid, bool>::type>
	Publication(ValueFederate *valueFed, std::string name, std::string units = "")
		: fed(valueFed), m_name(std::move(name)), m_units(std::move(units)), type(helicsType<X>)
	{
		id = fed->registerPublication(m_name, typeNameStringRef(type), m_units);
	}
	/**constructor to build a publication object
	@param[in] valueFed  the ValueFederate to use
	@param[in] name the name of the subscription
	@param[in] units the units associated with a Federate
	*/
	template<class X, typename std::enable_if<helicsType<X>() != helicsType_t::helicsInvalid, bool>::type>
	Publication(publication_locality locality, ValueFederate *valueFed, std::string name, std::string units = "")
		: fed(valueFed), m_name(std::move(name)), m_units(std::move(units)),type(helicsType<X>)
	{
		if (locality == GLOBAL)
		{
			id = fed->registerGlobalPublication(m_name, typeNameStringRef(type), m_units);
		}
		else
		{
			id = fed->registerPublication(m_name, typeNameStringRef(type), m_units);
		}
	}
	/** send a value for publication
	@param[in] val the value to publish*/
	void publish(double val) const;
	void publish(int64_t val) const;
	void publish(const char *val) const;
	void publish(const std::string &val) const;
	void publish(const std::vector<double> &val) const;
	void publish(std::complex<double> val) const;
	/** secondary publish function to allow unit conversion before publication
	@param[in] val the value to publish
	@param[in] units  the units association with the publication
	*/
	template <class X>
	void publish(const X &val, const std::string & /*units*/) const
	{
		// TODO:: figure out units
		publish(val);
	}

private:
	bool changeDetected(const std::string &val) const;
	bool changeDetected(const std::vector<double> &val) const;
	bool changeDetected(const std::complex<double> &val) const;
	bool changeDetected(double val) const;
	bool changeDetected(int64_t val) const;
};

/** class to handle a publication */
template <class X>
class PublicationT
{
  private:
    ValueFederate *fed = nullptr;  //!< the federate construct to interact with
    std::string m_name;  //!< the name of the publication
    std::string m_units;  //!< the defined units of the publication
    publication_id_t id;  //!< the internal id of the publication
  public:
    PublicationT () noexcept {};
    /**constructor to build a publication object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] units the units associated with a Federate
    */
    PublicationT (ValueFederate *valueFed, std::string name, std::string units = "")
        : fed (valueFed), m_name (std::move (name)), m_units (std::move (units))
    {
        id = fed->registerPublication<X> (m_name, m_units);
    }
    /**constructor to build a publication object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] units the units associated with a Federate
    */
    PublicationT (publication_locality locality, ValueFederate *valueFed, std::string name, std::string units = "")
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
class PublicationOnChange : public PublicationT<X>
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
        : PublicationT<X> (valueFed, name, units), publishDelta (minChange)
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
            PublicationT<X>::publish (val);
        }
    }
};

/** class to handle a publication
but the value is only published in the change is greater than a certain level*/
template <>
class PublicationOnChange<std::string> : public PublicationT<std::string>
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
        : PublicationT<std::string> (valueFed, name, units)
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
            PublicationT<std::string>::publish (val);
        }
    }
};
}
#endif
