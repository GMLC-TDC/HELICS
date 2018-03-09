/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "Core.hpp"

namespace helics
{
/** define the type of the handle*/
enum BasicHandleType : char
{
    HANDLE_UNKNOWN,
    HANDLE_PUB,  //!< handle to publish interface
    HANDLE_SUB,  //!< handle to a subscribe interface
    HANDLE_END,  //!< handle to an endpoint
    HANDLE_SOURCE_FILTER,  //!< handle to a source filter
    HANDLE_DEST_FILTER,  //!< handle to a destination filter
    HANDLE_CLONE_FILTER,  //!< handle for a cloning filter
};

/** class defining and capturing basic information about a handle*/
class BasicHandleInfo
{
  public:
    /** default constructor*/
    BasicHandleInfo () noexcept : type_in (type), type_out (units){};
    /** construct from the data*/
    BasicHandleInfo (Core::handle_id_t id_,
                     Core::federate_id_t fed_id_,
                     BasicHandleType what_,
                     const std::string &key_,
                     const std::string &type_,
                     const std::string &units_)
        : id (id_), fed_id (fed_id_), what (what_), key (key_), type (type_), units (units_), type_in (type),
          type_out (units)

    {
    }
    /** construct from the data for filters*/
    BasicHandleInfo (Core::handle_id_t id_,
                     Core::federate_id_t fed_id_,
                     BasicHandleType what_,
                     const std::string &key_,
                     const std::string &target_,
                     const std::string &type_in_,
                     const std::string &type_out_)
        : id (id_), fed_id (fed_id_), what (what_), key (key_), type (type_in_), units (type_out_),
          target (target_), type_in (type), type_out (units)

    {
    }

    const Core::handle_id_t id = invalid_handle;  //!< the identification number for the handle
    const Core::federate_id_t fed_id = invalid_fed_id;  //!< the global federate id for the creator of the handle
    Core::federate_id_t local_fed_id = invalid_fed_id;  //!< the local federate id of the handle
    const BasicHandleType what = HANDLE_UNKNOWN;  //!< the type of the handle
    bool flag = false;  //!< indicator flag
    bool processed = false;  //!< indicator if the handle has been processed (subscription or endpoint found)
    bool mapped = false;
    bool hasSourceFilter = false;  //!< indicator that an endpoint handle has a source filter
    bool hasDestFilter = false;  //!< indicator that an endpoint has a destination filter
    bool used = false;  //!< indicator that the publication or filter is used
    // 5 byte hole here
    const std::string key;  //!< the name of the handle
    const std::string type;  //!< the type of data used by the handle
    const std::string units;  //!< the units associated with the handle
    const std::string target;  //!< the target of the handle mapped onto units since they will not be used together
    const std::string &type_in;  //!< the input type of a filter
    const std::string &type_out;  //!< the output type of a filter
};
}  // namespace helics

