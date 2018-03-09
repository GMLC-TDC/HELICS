/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "Core.hpp"
#include "helics-time.hpp"
#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

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
    std::unordered_multimap<std::string, Core::handle_id_t> subscriptions;  //!< multimap of subscriptions
    std::unordered_multimap<std::string, Core::handle_id_t> filters;  //!< multimap for all the filters
  public:
    /** default constructor*/
    HandleManager () = default;
    /** add a handle to manage*/
    BasicHandleInfo *addHandle (Core::federate_id_t fed_id,
                                BasicHandleType what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo *addHandle (Core::federate_id_t fed_id,
                                BasicHandleType what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);
    BasicHandleInfo *getHandleInfo (Core::handle_id_t id_) const;
    BasicHandleInfo *getEndpoint (const std::string &name) const;
    BasicHandleInfo *getFilter (const std::string &name) const;
    BasicHandleInfo *getSubscription (const std::string &name) const;
    BasicHandleInfo *getPublication (const std::string &name) const;

    int32_t getLocalFedID (Core::handle_id_t id_) const;
private:
    void addType(BasicHandleInfo *handle, int32_t index);
    std::string generateName(BasicHandleType what);
};

}  // namespace helics

