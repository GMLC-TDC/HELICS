/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_HANDLE_
#define _HELICS_ZEROMQ_HANDLE_

#include <map>
#include <string>
#include <vector>

#include "helics/config.h"
#include "helics/core/core.h"
#include "helics/core/helics-time.h"

namespace helics
{

enum ZeroMQHandleType {
	HANDLE_PUB,
	HANDLE_SUB,
	HANDLE_END,
	HANDLE_SRC_FILTER,
	HANDLE_END_FILTER,
};

class ZeroMQHandle
{
  public:
    ZeroMQHandle (Core::federate_id_t fed_id_,
                ZeroMQHandleType what_,
                const char *key_,
                const char *type_,
                const char *units_,
                bool required_ = false,
                bool is_endpoint_ = false)
        : id (0),
          fed_id (fed_id_),
          what (what_),
          key (key_),
          type (type_),
          units (units_),
          required (required_),
          is_endpoint (is_endpoint_),
          handles()
    {
    }

    ~ZeroMQHandle () {}

	Core::Handle id;
	Core::federate_id_t fed_id;
    ZeroMQHandleType what;
    std::string key;
    std::string type;
    std::string units;
    bool required;
    std::string data;
    bool is_endpoint;
    std::vector<Core::Handle> handles;
    bool has_update=false;
	FilterOperator *filterOp = nullptr;
	std::string filterTarget;

    typedef std::map<std::string, ZeroMQHandle *>::iterator iter;
};

}  // namespace helics

#endif /* _HELICS_ZEROMQ_HANDLE_ */
