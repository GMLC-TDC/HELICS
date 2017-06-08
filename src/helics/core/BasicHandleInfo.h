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
enum BasicHandleType:char
{
	HANDLE_UNKNOWN,
    HANDLE_PUB,
    HANDLE_SUB,
    HANDLE_END,
    HANDLE_FILTER,
};

class BasicHandleInfo
{
  public:
	  BasicHandleInfo() noexcept {};
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

    Core::Handle id=invalid_Handle;
    Core::federate_id_t fed_id=invalid_fed_id;
	Core::federate_id_t local_fed_id=invalid_fed_id;
    BasicHandleType what=HANDLE_UNKNOWN;
    bool flag=false;
	bool destFilter = false;
    std::string key;
    std::string type;
    std::string units;
	std::string target;
};
}
#endif
