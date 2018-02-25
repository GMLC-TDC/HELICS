/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once
#include "Core.hpp"
#include "helics-time.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <atomic>
#include <memory>

#include "BasicHandleInfo.hpp"
namespace helics
{
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class HandleManager
{
private:
    std::vector<std::unique_ptr<BasicHandleInfo>> handles;  //!< local handle information
    std::unordered_map<std::string, Core::handle_id_t> publications;  //!< map of all local publications
    std::unordered_map<std::string, Core::handle_id_t> endpoints;  //!< map of all local endpoints
    std::unordered_multimap<std::string, Core::handle_id_t> subscriptions; //!< multimap of subscriptions
    std::unordered_multimap<std::string, Core::handle_id_t> filters;  //!< multimap for all the filters
public:
    /** default constructor*/
    HandleManager() = default;
    /** add a handle to manage*/
    BasicHandleInfo *addHandle(Core::handle_id_t id,
        Core::federate_id_t fed_id,
        BasicHandleType what,
        const std::string &key,
        const std::string &type,
        const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo *addHandle(Core::handle_id_t id,
        Core::federate_id_t fed_id,
        BasicHandleType what,
        const std::string &key,
        const std::string &target,
        const std::string &type_in,
        const std::string &type_out);
};

}