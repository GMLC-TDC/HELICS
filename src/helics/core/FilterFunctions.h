/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef FILTER_FUNCTIONS_H_
#define FILTER_FUNCTIONS_H_
#pragma once

#include "core.h"

#include <vector>
namespace helics
{
class FilterFunctions
{
public:
	std::vector<std::pair<Core::federate_id_t, Core::Handle>> sourceOperators;
	std::pair<Core::federate_id_t, Core::Handle> finalSourceFilter;
	std::pair<Core::federate_id_t, Core::Handle> destOperator;
	bool hasSourceOperators=false;
	bool hasSourceFilter=false;
	bool hasDestOperator=false;
};
}

#endif