/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
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
class HandlePointerManager
{
  private:
    std::vector<std::unique_ptr<BasicHandleInfo>> handles;  //!< local handle information
    std::unordered_map<std::string, interface_handle> publications;  //!< map of all local publications
    std::unordered_map<std::string, interface_handle> endpoints;  //!< map of all local endpoints
    std::unordered_multimap<std::string, interface_handle> subscriptions;  //!< multimap of subscriptions
    std::unordered_multimap<std::string, interface_handle> filters;  //!< multimap for all the filters
    std::unordered_multimap<std::string, Core::interface_handle> filters;  //!< multimap for all the filters
    std::unordered_multimap<std::string, Core::interface_handle> controlOutputs;  //!< multimap for all the control outputs
  public:
    /** default constructor*/
    HandlePointerManager () = default;
    /** add a handle to manage*/
    BasicHandleInfo *addHandle (global_federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo *addHandle (global_federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);
    BasicHandleInfo *getHandleInfo (interface_handle id_) const;
    BasicHandleInfo *getEndpoint (const std::string &name) const;
    BasicHandleInfo *getFilter (const std::string &name) const;
    BasicHandleInfo *getSubscription (const std::string &name) const;
    BasicHandleInfo *getPublication (const std::string &name) const;

    federate_id_t getLocalFedID (interface_handle id_) const;
    auto begin () { return handles.begin (); }
    auto end () { return handles.end (); }
    auto begin () const { return handles.begin (); }
    auto end () const { return handles.end (); }

  private:
    void addType (BasicHandleInfo *handle, int32_t index);
    std::string generateName (handle_type_t what);
};

}  // namespace helics
