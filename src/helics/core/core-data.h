/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _CORE_DATA_TYPES_H_
#define _CORE_DATA_TYPES_H_
#pragma once

#include "helics-time.h"
#include "helics/config.h"

#include <cstdint>

/**
* HELICS Core API
*/
namespace helics
{
	/**
	* Data to be communicated.
	*
	* Core operates on opaque byte buffers.
	*/
	typedef struct data_t
	{
		char *data;
		uint64_t len;
	} data_t;

	/**
	*  Message.
	*/
	typedef struct message_t
	{
		const char *origsrc;
		const char *src;
		const char *dst;
		const char *data;
		uint64_t len;
		Time time;
	} message_t;


	/**
	* FilterOperator abstract class
	*
	*/
	class FilterOperator
	{
	public:
		FilterOperator() {};
		virtual ~FilterOperator() = default;
		virtual message_t operator() (message_t *) = 0;
	};
}
#endif
