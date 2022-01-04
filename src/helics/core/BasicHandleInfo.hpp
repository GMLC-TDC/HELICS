/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_CoreTypes.hpp"
#include "flagOperations.hpp"

#include <string>
#include <utility>
#include <vector>

namespace helics {

/** define extra flag definitions*/
enum handle_flag_definitions {
    mapped_flag = extra_flag1,
    has_source_filter_flag =
        extra_flag2,  //!< indicator that an endpoint or message has a source filter
    has_dest_filter_flag =
        extra_flag3,  //!< indicator that an endpoint or message has a destination filter
    has_non_cloning_dest_filter_flag = extra_flag4  //!< indicator that the endpoint or filter has a
                                                    //!< destination filter that alters the message
};

/** class defining and capturing basic information about a handle*/
class BasicHandleInfo {
  public:
    /** default constructor*/
    BasicHandleInfo() noexcept: type_in(type), type_out(units) {}
    /** construct from the data*/
    BasicHandleInfo(GlobalFederateId federate_id,
                    InterfaceHandle handle_id,
                    InterfaceType type_of_handle,
                    const std::string& key_name,
                    const std::string& type_name,
                    const std::string& unit_name):
        handle{federate_id, handle_id},
        handleType(type_of_handle), key(key_name), type(type_name), units(unit_name), type_in(type),
        type_out(units)

    {
        /* NOTE:: all current cases already have std::string they are passing into the function and
        most are short,so we are taking by reference to avoid a copy then move  and just have a
        copy*/
    }

    const GlobalHandle handle{};  //!< the global federate id for the creator of the handle
    LocalFederateId local_fed_id{};  //!< the local federate id of the handle
    const InterfaceType handleType{InterfaceType::UNKNOWN};  //!< the type of the handle
    bool used{false};  //!< indicator that the handle is being used to link with another federate
    uint16_t flags{
        0};  //!< flags corresponding to the flags used in ActionMessages +some extra ones

    const std::string key;  //!< the name of the handle
    const std::string type;  //!< the type of data used by the handle
    const std::string units;  //!< the units associated with the handle
    const std::string& type_in;  //!< the input type of a filter
    const std::string& type_out;  //!< the output type of a filter
    /** get the interface handle information */
    InterfaceHandle getInterfaceHandle() const { return handle.handle; }
    /** extract a global federate id */
    GlobalFederateId getFederateId() const { return handle.fed_id; }
    /** set a tag (key-value pair)*/
    void setTag(const std::string& tag, const std::string& value);
    /** search for a tag by name*/
    const std::string& getTag(const std::string& tag) const;
    /** get a tag (key-value pair) by index*/
    const std::pair<std::string, std::string>& getTagByIndex(size_t index) const
    {
        return tags[index];
    }
    /** get the number of tags associated with an interface*/
    auto tagCount() const { return tags.size(); }

  private:
    std::vector<std::pair<std::string, std::string>> tags;  //!< storage for user defined tags
};
}  // namespace helics
