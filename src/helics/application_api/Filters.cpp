/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Filters.hpp"
namespace helics
{


	std::unique_ptr<DestinationFilter> make_destination_filter(defined_filter_types type,
		MessageFilterFederate *mFed,
		const std::string &target,
		const std::string &name)
	{
		return nullptr;
}
	std::unique_ptr<SourceFilter> make_Source_filter(defined_filter_types type,
		MessageFilterFederate *mFed,
		const std::string &target,
		const std::string &name)
	{
		return nullptr;
}


	delayFilterOperation::delayFilterOperation(Time delayTime) :delay(delayTime)
	{

	}
	void delayFilterOperation::set(const std::string &property, double val)
	{

	}
	void delayFilterOperation::setString(const std::string &property, const std::string &val)
	{

	}

	class randomDelayGenerator
	{
	public:
		Time delay;
	};

	randomDelayFilterOperation::randomDelayFilterOperation()
	{

	}
	randomDelayFilterOperation::~randomDelayFilterOperation()
	{

	}
	void randomDelayFilterOperation::set(const std::string &property, double val)
	{

	}
	void randomDelayFilterOperation::setString(const std::string &property, const std::string &val)
	{

	}
} //namespace helics