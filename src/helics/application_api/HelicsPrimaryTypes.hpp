/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_PRIMARY_TYPES_H_
#define HELICS_PRIMARY_TYPES_H_
#pragma once

#include <boost/variant.hpp>
#include <string>
#include <complex>
#include <vector>
#include <cstdint>
#include "helicsTypes.hpp"
#include "ValueConverter.hpp"
#include "boost/lexical_cast.hpp"

/** @file 
@brief naming a set of types that are interchangeable and recognizable inside the HELICS application API and core
*/
namespace helics
{
	/** define a variant with the different types*/
	using defV = boost::variant< double, int64_t, std::string, std::complex<double>, std::vector<double>,std::vector<std::complex<double>>>;

	/**enumeration of the order inside the variant so the Which function returns match the enumeration*/
	enum type_location
	{
		
		doubleLoc = 0,
		intLoc = 1,
		stringLoc = 2,
		complexLoc = 3,
		vectorLoc = 4,
		complexVectorLoc=5,
	};

	bool changeDetected(const defV &prevValue, const std::string &val, double deltaV);
	bool changeDetected(const defV &prevValue, const std::vector<double> &val, double deltaV);
	bool changeDetected(const defV &prevValue, const std::vector<std::complex<double>> &val, double deltaV);
	bool changeDetected(const defV &prevValue, const double *vals, size_t size, double deltaV);
	bool changeDetected(const defV &prevValue, const std::complex<double> &val, double deltaV);
	bool changeDetected(const defV &prevValue, double val, double deltaV);
	bool changeDetected(const defV &prevValue, int64_t val, double deltaV);

	void valueExtract(const defV &dv, std::string &val);

	void valueExtract(const defV &dv, std::complex<double> &val);

	void valueExtract(const defV &dv, std::vector<double> &val);

	void valueExtract(const defV &dv, std::vector<std::complex<double>> &val);

	void valueExtract(const data_view &dv, helics_type_t baseType, std::string &val);

	void valueExtract(const data_view &dv, helics_type_t baseType, std::vector<double> &val);

	void valueExtract(const data_view &dv, helics_type_t baseType, std::complex<double> &val);

	void valueExtract(const data_view &dv, helics_type_t baseType, std::vector<std::complex<double>> &val);

	/** for numeric types*/
	template <class X>
	std::enable_if_t<std::is_arithmetic<X>::value> valueExtract(const defV &dv, X &val)
	{
		switch (dv.which())
		{
		case doubleLoc:  // double
			val = static_cast<X> (boost::get<double>(dv));
			break;
		case intLoc:  // int64_t
			val = static_cast<X> (boost::get<int64_t>(dv));
			break;
		case stringLoc:  // string
		default:
			val = boost::lexical_cast<X> (boost::get<std::string>(dv));
			break;
		case complexLoc:  // complex
			val = static_cast<X> (std::abs(boost::get<std::complex<double>>(dv)));
			break;
		case vectorLoc:  // vector
		{
			auto &vec = boost::get<std::vector<double>>(dv);
			if (!vec.empty())
			{
				val = static_cast<X> (vec.front());
			}
			else
			{
				val = std::numeric_limits<X>::min();
			}
			break;
		}
		case complexVectorLoc:  // complex vector
		{
			auto &vec = boost::get<std::vector<std::complex<double>>>(dv);
			if (!vec.empty())
			{
				val = static_cast<X> (std::abs(vec.front()));
			}
			else
			{
				val = std::numeric_limits<X>::min();
			}
			break;
		}
		}
	}

	/** assume it is some numeric type (int or double)*/
	template <class X>
	std::enable_if_t<std::is_arithmetic<X>::value> valueExtract(const data_view &dv, helics_type_t baseType, X &val)
	{
		switch (baseType)
		{
        case helics_type_t::helicsAny:
        {
            try
            {
                val = static_cast<X> (std::stod(dv.string()));
            }
            catch (const std::invalid_argument &ble)
            { //well lets try a direct conversion
                auto V = ValueConverter<double>::interpret(dv);
                val = static_cast<X> (V);
            }
            break;
        }
		case helics_type_t::helicsString:
		{
            val = static_cast<X> (boost::lexical_cast<double> (dv.string()));
            break;
		}
		case helics_type_t::helicsDouble:
		{
			auto V = ValueConverter<double>::interpret(dv);
			val = static_cast<X> (V);
			break;
		}
		case helics_type_t::helicsInt:
		{
			auto V = ValueConverter<int64_t>::interpret(dv);
			val = static_cast<X> (V);
			break;
		}

		case helics_type_t::helicsVector:
		{
			auto V = ValueConverter<std::vector<double>>::interpret(dv);
            val = (!V.empty()) ? static_cast<X> (V[0]) : 0.0;
			break;
		}
		case helics_type_t::helicsComplex:
		{
			auto V = ValueConverter<std::complex<double>>::interpret(dv);
			val = static_cast<X> (std::abs(V));
			break;
		}
		case helics_type_t::helicsComplexVector:
		{
			auto V = ValueConverter<std::vector<std::complex<double>>>::interpret(dv);
            val = (!V.empty()) ? static_cast<X> (std::abs(V.front())) : 0.0;
			break;
		}
		case helics_type_t::helicsInvalid:
			throw (std::invalid_argument("unrecognized helics type"));
		}
	}
} //namespace helics

#endif //HELICS_PRIMARY_TYPES_H_

