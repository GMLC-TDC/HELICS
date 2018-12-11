/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Core.hpp"
#include "flagOperations.hpp"

namespace helics
{
/** define the type of the handle*/
enum class handle_type : char
{
    unknown = 'u',
    publication = 'p',  //!< handle to output interface
    input = 'i',  //!< handle to a input interface
    endpoint = 'e',  //!< handle to an endpoint
    filter = 'f',  //!< handle to a filter
};

/** define extra flag definitions*/
enum handle_flag_definitions
{
    mapped_flag = extra_flag1,
    has_source_filter_flag = extra_flag2,
    has_dest_filter_flag = extra_flag3,
    has_non_cloning_dest_filter_flag = extra_flag4
};

/** class defining and capturing basic information about a handle*/
class BasicHandleInfo
{
  public:
    /** default constructor*/
    BasicHandleInfo () noexcept : type_in (type), type_out (units){};
    /** construct from the data*/
    BasicHandleInfo (global_federate_id federate_id_t,
                     interface_handle handle_id,
                     handle_type type_of_handle,
                     const std::string &key_name,
                     const std::string &type_name,
                     const std::string &unit_name)
        : handle{federate_id_t, handle_id}, handleType (type_of_handle), key (key_name), type (type_name),
          units (unit_name), type_in (type), type_out (units)

    {
        /* NOTE:: all current cases already have std::string they are passing into the function and most are
        short,so we are taking by reference to avoid a copy/move  and just have a copy*/
    }

    const global_handle handle;  //!< the global federate id for the creator of the handle
    federate_id_t local_fed_id;  //!< the local federate id of the handle
    const handle_type handleType = handle_type::unknown;  //!< the type of the handle
    bool used = false;  //!< indicator that the handle is being used to link with another federate
    uint16_t flags = 0;  //!< flags corresponding to the flags used in ActionMessages +some extra ones

    const std::string key;  //!< the name of the handle
    const std::string type;  //!< the type of data used by the handle
    const std::string units;  //!< the units associated with the handle
    std::string interface_info;
    const std::string &type_in;  //!< the input type of a filter
    const std::string &type_out;  //!< the output type of a filter

    interface_handle getInterfaceHandle () const { return handle.handle; }
    global_federate_id getFederateId () const { return handle.fed_id; }

    void setInfoField (std::string &info) { interface_info = info; }
};
}  // namespace helics
