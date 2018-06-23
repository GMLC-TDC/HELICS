/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Core.hpp"
#include "ActionMessageDefintions.hpp"

namespace helics
{
/** define the type of the handle*/
enum class handle_type_t : char
{
    unknown,
    publication,  //!< handle to publish interface
    subscription,  //!< handle to a subscribe interface
    control_input, //!< handle for a control input
    control_output, //!< handle for a control output
    endpoint,  //!< handle to an endpoint
    source_filter,  //!< handle to a source filter
    destination_filter,  //!< handle to a destination filter
    cloning_filter,  //!< handle for a cloning filter
};

/** define extra flag definitions*/
enum handle_flag_definitions
{
    mapped_flag = extra_flag1,
    has_source_filter_flag = extra_flag2,
    has_dest_filter_flag = extra_flag3,
    has_non_cloning_dest_filter_flag=extra_flag4
};

/** class defining and capturing basic information about a handle*/
class BasicHandleInfo
{
  public:
    /** default constructor*/
    BasicHandleInfo () noexcept : type_in (type), type_out (units){};
    /** construct from the data*/
    BasicHandleInfo (Core::handle_id_t handle_id,
                     Core::federate_id_t federate_id,
                     handle_type_t type_of_handle,
                     const std::string &key_name,
                     const std::string &type_name,
                     const std::string &unit_name)
        : handle (handle_id), fed_id (federate_id), handle_type (type_of_handle), key (key_name), type (type_name), units (unit_name), type_in (type),
          type_out (units)

    {
    }
    /** construct from the data for filters*/
    BasicHandleInfo (Core::handle_id_t handle_id,
                     Core::federate_id_t federate_id,
                     handle_type_t type_of_handle,
                     const std::string &key_name,
                     const std::string &target_name,
                     const std::string &type_in_name,
                     const std::string &type_out_name)
        : handle(handle_id), fed_id (federate_id), handle_type (type_of_handle), key (key_name), type (type_in_name), units (type_out_name),
          target (target_name), type_in (type), type_out (units)

    {
    }
   
    const Core::handle_id_t handle = invalid_handle;  //!< the identification number for the handle
    const Core::federate_id_t fed_id = invalid_fed_id;  //!< the global federate id for the creator of the handle
    Core::federate_id_t local_fed_id = invalid_fed_id;  //!< the local federate id of the handle
    const handle_type_t handle_type = handle_type_t::unknown;  //!< the type of the handle
    bool used = false;  //!< indicator that the publication or filter is used
    uint16_t flags = 0; //!< flags corresponding to the flags used in ActionMessages +some extra ones

    const std::string key;  //!< the name of the handle
    const std::string type;  //!< the type of data used by the handle
    const std::string units;  //!< the units associated with the handle
    const std::string target;  //!< the target of the handle mapped onto units since they will not be used together
    const std::string &type_in;  //!< the input type of a filter
    const std::string &type_out;  //!< the output type of a filter
};
}  // namespace helics
