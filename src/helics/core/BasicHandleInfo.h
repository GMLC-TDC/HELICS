/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef BASIC_HANDLE_INFO_H_
#define BASIC_HANDLE_INFO_H_
#pragma once

#include "core.h"

namespace helics
{
/** define the type of the handle*/
enum BasicHandleType:char
{
	HANDLE_UNKNOWN,
    HANDLE_PUB,  //!< handle to publish interface
    HANDLE_SUB,	//!< handle to a subscribe interface
    HANDLE_END,	//!< handle to an endpoint
    HANDLE_FILTER,	//!< handle to a filter
};

/** class definining and capturing basic information about a handle*/
class BasicHandleInfo
{
  public:
	  /** default constructor*/
	  BasicHandleInfo()=default;
	  /** construct from the data*/
    BasicHandleInfo (Core::Handle id_,
                     Core::federate_id_t fed_id_,
                     BasicHandleType what_,
                     const std::string &key_,
                     const std::string &type_,
                     const std::string &units_,
                     bool flag_ = false)
        : id (id_), fed_id (fed_id_), what (what_), flag (flag_), key (key_), type (type_), units (units_)

    {
		if (what == HANDLE_FILTER)
		{
			target = units;
			destFilter = flag;
		}
    }

    Core::Handle id=invalid_Handle;  //!< the identification number for the handle
    Core::federate_id_t fed_id=invalid_fed_id; //!< the global federate id for the creator of the handle
	Core::federate_id_t local_fed_id=invalid_fed_id;	//!< the local federate id of the handle
    BasicHandleType what=HANDLE_UNKNOWN;	//!< the type of the handle
    bool flag=false;	//!< indicator flag
	bool destFilter = false;	//!< indicator if the handle is a destination filter
    std::string key;	//!< the name of the handle
    std::string type;	//!< the type of data used by the handle
    std::string units;	//!< the units associated with the handle
	std::string target;	//!< the target of the handle
};
}
#endif
